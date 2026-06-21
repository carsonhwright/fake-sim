// First test in the fake-sim suite. No framework -- each check prints PASS/FAIL
// and the program returns non-zero if any check fails (CI-friendly).
//
// Built and run by `make test` (one binary per tests/*.cpp, linked against the
// sim/geo/math core).

#include <cstdio>
#include "include/sim.hpp"
#include "include/fake-geo.hpp"

static int failures = 0;

static void check(bool cond, const char *what) {
    printf("[%s] %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) ++failures;
}

int main() {
    // --- geo::box -------------------------------------------------------
    auto b = geo::box(Vec3i{2, 3, 4}, 3, 4, 5);
    check(b.size() == (size_t)(3 * 4 * 5), "box has width*height*depth cells");
    bool has_origin = false, has_far = false;
    for (const auto &c : b) {
        if (c.x == 2 && c.y == 3 && c.z == 4) has_origin = true;
        if (c.x == 4 && c.y == 6 && c.z == 8) has_far = true;   // origin+size-1
    }
    check(has_origin && has_far, "box spans origin .. origin+size-1");

    // --- geo::sphere ----------------------------------------------------
    auto s = geo::sphere(Vec3i{0, 0, 0}, 5);
    bool all_within = true, center_in = false;
    for (const auto &c : s) {
        if (c.x == 0 && c.y == 0 && c.z == 0) center_in = true;
        if (c.x * c.x + c.y * c.y + c.z * c.z > 5 * 5 + 0) all_within = false;
    }
    check(center_in, "sphere includes its centre");
    check(all_within, "sphere cells lie within the radius");

    // --- geo::line ------------------------------------------------------
    auto l = geo::line(Vec3i{0, 0, 0}, Vec3i{5, 2, 0});
    check(l.size() == 6, "line spans the dominant axis (6 cells)");
    check(l.front().x == 0 && l.back().x == 5, "line includes both endpoints");

    // --- VoxelField: sparse get/set ------------------------------------
    PixelSpace space{16, 16, 16};
    VoxelField field(space);
    check(field.active_cells() == 0, "field starts empty");
    field.set(Vec3i{1, 2, 3}, 42);
    check(field.get(Vec3i{1, 2, 3}) == 42, "set value reads back");
    check(field.get(Vec3i{9, 9, 9}) == field.default_value, "unset cell is default");
    check(field.get(Vec3i{-1, 0, 0}) == field.default_value, "out-of-bounds read is safe");
    field.set(Vec3i{1, 2, 3}, field.default_value);
    check(field.active_cells() == 0, "writing default erases (stays sparse)");

    // --- Particle follows a route and samples the field ----------------
    Route route;
    route.add_waypoint(Vec3i{0, 0, 0});
    route.add_waypoint(Vec3i{10, 0, 0});
    field.set(Vec3i{10, 0, 0}, 7);

    auto frames = record(route, constant_speed(5), 0.1, field);
    check(!frames.empty(), "record produces frames");
    check(frames.front().position.x == 0, "particle starts at route origin");
    check(frames.back().position.x == 10, "particle reaches the route end");
    check(frames.back().value == 7, "particle samples the field at the end cell");

    printf("\n%s (%d failure%s)\n", failures ? "TESTS FAILED" : "ALL TESTS PASSED",
           failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
