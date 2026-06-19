#include <stdio.h>
#include <vector>
#include "raylib.h"
#include "include/fake-math.hpp"
#include "include/sim.hpp"

// Map an integer grid cell into raylib's coordinate space (Y up). One grid
// cell == one world unit; the camera is scaled to the pixel space so this
// stays readable at any size.
static Vector3 to_rl(const Vec3i &v) {
    return Vector3{(float)v.x, (float)v.y, (float)v.z};
}

// Map a field value to a colour ramp (blue -> red) with a very low alpha so the
// field reads as a translucent haze you can see the particle move through.
static Color value_color(short v, short vmin, short vmax) {
    float t = (vmax > vmin) ? (float)(v - vmin) / (float)(vmax - vmin) : 0.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    unsigned char r = (unsigned char)(t * 255.0f);
    unsigned char b = (unsigned char)((1.0f - t) * 255.0f);
    return Color{r, 40, b, 40};   // alpha 40/255 -> very translucent
}

int main() {
    // --- Configure the pixel space ---------------------------------------
    // Change these to resize the world; everything below frames itself to it.
    PixelSpace space{PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};

    // --- Define the route (integer cells) --------------------------------
    Route route;
    route.add_waypoint(Vec3i{0, 0, 0});
    route.add_waypoint(Vec3i{100, 0, 0});
    route.add_waypoint(Vec3i{20, 120, 0});
    route.add_waypoint(Vec3i{0, 10, 20});
    route.add_waypoint(Vec3i{10, 20, 10});

    // --- Voxel field: one short per cell ---------------------------------
    // Sparse storage: only cells we set carry data. A future routine will fill
    // shapes/gradients; for now we set a modest slab so there's something to
    // sample and visualise. value == y gives a vertical gradient.
    VoxelField field(space);
    for (int z = 0; z <= 30; ++z)
        for (int y = 0; y <= 40; ++y)
            for (int x = 0; x <= 60; ++x)
                field.set(x, y, z, (short)y);
    // Individual cells can be updated by coordinate too:
    field.set(Vec3i{100, 0, 0}, 9);
    printf("field active cells: %zu (sparse)\n", field.active_cells());

    // Integer speed (cells per second). Swap for any SpeedModel to vary it,
    // e.g. [](double d, double t){ return 6.0 + 2.0 * t; }  // accelerating
    const double sim_dt = 0.05;
    std::vector<Frame> frames = record(route, constant_speed(8), sim_dt, field);

    if (frames.empty()) {
        printf("nothing to play back\n");
        return 0;
    }
    printf("recorded %zu frames, route length %.2f, space %dx%dx%d\n",
           frames.size(), route.length(), space.width, space.height, space.depth);

    // --- Viewer -----------------------------------------------------------
    const int screenW = 1000, screenH = 700;
    InitWindow(screenW, screenH, "fake-sim: pixel particle playback");
    SetTargetFPS(60);

    // Frame the camera around the centre of the pixel space, backed off by a
    // distance proportional to the largest dimension.
    Vector3 center = {space.width * 0.5f, space.height * 0.5f, space.depth * 0.5f};
    int maxdim = space.width;
    if (space.height > maxdim) maxdim = space.height;
    if (space.depth  > maxdim) maxdim = space.depth;
    float back = maxdim * 1.6f;

    Camera3D cam = {};
    cam.position = Vector3{center.x + back, center.y + back * 0.8f, center.z + back};
    cam.target   = center;
    cam.up       = Vector3{0.0f, 1.0f, 0.0f};
    cam.fovy     = 45.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    // Snapshot of the field's active cells for rendering, plus its value range
    // for the colour ramp. The field is static during playback, so build once.
    std::vector<std::pair<Vec3i, short>> field_cells = field.entries();
    short vmin = 0, vmax = 1;
    if (!field_cells.empty()) {
        vmin = vmax = field_cells[0].second;
        for (const auto &e : field_cells) {
            if (e.second < vmin) vmin = e.second;
            if (e.second > vmax) vmax = e.second;
        }
    }

    // Playback state: a (possibly fractional) index into `frames`.
    double cursor = 0.0;
    double play_speed = 1.0;   // multiples of real time
    bool playing = true;
    const double frames_per_sec = 1.0 / sim_dt;

    while (!WindowShouldClose()) {
        // --- Controls ---
        if (IsKeyPressed(KEY_SPACE)) playing = !playing;
        if (IsKeyPressed(KEY_R))     { cursor = 0.0; playing = true; }
        if (IsKeyPressed(KEY_UP))    play_speed *= 2.0;
        if (IsKeyPressed(KEY_DOWN))  play_speed *= 0.5;
        if (IsKeyDown(KEY_LEFT))  { cursor -= frames_per_sec * GetFrameTime(); playing = false; }
        if (IsKeyDown(KEY_RIGHT)) { cursor += frames_per_sec * GetFrameTime(); playing = false; }

        if (playing) {
            cursor += frames_per_sec * play_speed * GetFrameTime();
            if (cursor >= (double)(frames.size() - 1)) {
                cursor = (double)(frames.size() - 1);
                playing = false;   // stop at the end; press R to replay
            }
        }
        if (cursor < 0.0) cursor = 0.0;

        size_t idx = (size_t)cursor;
        if (idx > frames.size() - 1) idx = frames.size() - 1;
        const Frame &f = frames[idx];

        // Camera is intentionally static (no UpdateCamera call).

        // --- Draw ---
        BeginDrawing();
        ClearBackground(Color{20, 22, 28, 255});

        BeginMode3D(cam);

        // Pixel-space bounds: a wireframe box from origin to (w,h,d).
        DrawCubeWires(center, (float)space.width, (float)space.height,
                      (float)space.depth, DARKGRAY);
        DrawGrid(int(maxdim/8), 11);

        // Route as a trail of small voxels (one per recorded cell).
        for (const Frame &tf : frames) {
            DrawCube(to_rl(tf.position), maxdim/128, maxdim/128, maxdim/128, SKYBLUE);
        }

        // The particle: a unit voxel that snaps cell-to-cell.
        DrawCube(to_rl(f.position), maxdim/32, maxdim/32, maxdim/32, RED);
        DrawCubeWires(to_rl(f.position), maxdim/32, maxdim/32, maxdim/32, MAROON);

        // Voxel field as translucent coloured cells. Drawn last so the alpha
        // blends over the opaque geometry behind it.
        for (const auto &e : field_cells) {
            DrawCube(to_rl(e.first), 1.0f, 1.0f, 1.0f, value_color(e.second, vmin, vmax));
        }

        EndMode3D();

        // --- HUD ---
        DrawText(TextFormat("t = %5.2f s   dist = %d cells", f.time, f.distance),
                 12, 12, 20, RAYWHITE);
        DrawText(TextFormat("pos = (%d, %d, %d)   vel = (%d, %d, %d) cells/s   value = %d",
                            f.position.x, f.position.y, f.position.z,
                            f.velocity.x, f.velocity.y, f.velocity.z, f.value),
                 12, 36, 20, LIGHTGRAY);
        DrawText(TextFormat("frame %zu / %zu   speed x%.2g   %s",
                            idx, frames.size() - 1, play_speed,
                            playing ? "PLAYING" : "PAUSED"),
                 12, 60, 20, LIGHTGRAY);
        DrawText("SPACE play/pause  R restart  UP/DOWN speed  LEFT/RIGHT scrub",
                 12, screenH - 28, 18, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
