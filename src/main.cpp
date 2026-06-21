#include <stdio.h>
#include <vector>
#include <math.h>
#include "raylib.h"
#include "include/fake-math.hpp"
#include "include/fake-geo.hpp"
#include "include/sim.hpp"
#include "include/visualizer.hpp"


int main() {
    // --- Configure the pixel space ---------------------------------------
    // Change these to resize the world; everything below frames itself to it.
    PixelSpace space{PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};

    // --- Define the route (integer cells) --------------------------------
    // TODO these need to be in some kind of config file
    Route route;
    route.add_waypoint(Vec3i{0, 0, 0});
    route.add_waypoint(Vec3i{100, 0, 0});
    route.add_waypoint(Vec3i{20, 120, 0});
    route.add_waypoint(Vec3i{0, 10, 20});
    route.add_waypoint(Vec3i{10, 20, 10});

    // --- Voxel field: shapes built with geo:: ----------------------------
    // geo:: returns the coordinates that make up a shape; the stamp loop is
    // where per-cell values / gradients get assigned. Here the box uses a
    // vertical gradient (value == y) and the sphere a flat value.
    VoxelField field(space);
    unsigned char temp = VoxelField::pack_value(1, 120);
    for (const Vec3i &c : geo::box(Vec3i{0, 0, 0}, 30, 30, 30)) {
        field.set(c, 20);
    }
    for (const Vec3i &c : geo::sphere(Vec3i{120, 60, 40}, 18)) {
        field.set(c, 20);
    }
    // Individual cells can still be updated by coordinate too:
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

    // Snapshot of the field's active cells for rendering, plus its value range
    // for the colour ramp. The field is static during playback, so build once.
    std::vector<std::pair<Vec3i, unsigned char>> field_cells = field.entries();
    short vmin = 0, vmax = 1;
    if (!field_cells.empty()) {
        vmin = vmax = field_cells[0].second;
        for (const auto &e : field_cells) {
            if (e.second < vmin) vmin = e.second;
            if (e.second > vmax) vmax = e.second;
        }
    }

    sim_loop(space, sim_dt, frames, field_cells, vmin, vmax);
    
    return 0;
}
