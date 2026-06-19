#ifndef SIM_HPP
#define SIM_HPP

#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include "include/fake-math.hpp"

#define PIXEL_SIZE 200

// The bounds of the integer "pixel" space the simulation lives in, in grid
// cells. Change these (or construct a different PixelSpace) to resize the
// world; the viewer frames itself to whatever size you pick.
struct PixelSpace {
    int width  = PIXEL_SIZE;   // x extent (cells)
    int height = PIXEL_SIZE;   // y extent (cells)
    int depth  = PIXEL_SIZE;   // z extent (cells)

    long cell_count() const;   // width * height * depth
};

// A scalar field over the voxel grid: a cell (x, y, z) can hold a `short`
// value, addressable by its integer coordinate. Values are read and written by
// coordinate, and the particle samples the cell it occupies.
//
// Storage is *sparse*: only cells that have been explicitly set occupy memory
// (a hash map keyed by a packed coordinate). Any cell never set -- the vast
// majority in a large space -- reads back as `default_value`. This suits a
// world where only a few regions/shapes carry data, without paying for a dense
// width*height*depth buffer.
class VoxelField {
public:
    VoxelField() = default;
    explicit VoxelField(const PixelSpace &space, short default_val = 0);

    bool in_bounds(int x, int y, int z) const;
    bool in_bounds(const Vec3i &c) const;

    // Read a cell. Cells never set -- and out-of-bounds cells -- return
    // `default_value`, so sampling stays safe everywhere.
    short get(int x, int y, int z) const;
    short get(const Vec3i &c) const;

    // Write a cell. Out-of-bounds writes are ignored. Writing default_value
    // erases the entry, keeping storage sparse.
    void set(int x, int y, int z, short value);
    void set(const Vec3i &c, short value);

    const PixelSpace &space() const { return space_; }
    size_t active_cells() const { return data_.size(); }

    // Iterate the cells that actually hold a (non-default) value, e.g. for
    // visualization. Returns coordinate/value pairs.
    std::vector<std::pair<Vec3i, short>> entries() const;

    short default_value = 0;

private:
    uint64_t key(int x, int y, int z) const;

    PixelSpace space_;
    std::unordered_map<uint64_t, short> data_;
};

// A route is an ordered list of integer waypoints. The particle travels in
// straight segments from one waypoint to the next. Internally we keep the
// waypoints as continuous points and precompute the cumulative arc-length, so
// a position can be looked up by "distance travelled along the route" -- this
// decouples the path's *shape* from the particle's *speed*. Positions are
// snapped back to the integer grid when reported.
class Route {
public:
    void add_waypoint(const Vec3i &p);

    size_t num_waypoints() const;
    double length() const;                  // total arc length of the route

    // Continuous position at a given distance along the route. Distances past
    // the ends are clamped to the first/last waypoint.
    Vec3 position_at(double distance) const;
    // Unit tangent (direction of travel) at a given distance.
    Vec3 direction_at(double distance) const;

private:
    std::vector<Vec3> waypoints_;
    std::vector<double> cumulative_;        // cumulative_[i] = distance to waypoints_[i]
};

// Speed (in grid cells per second) as a function of how far the particle has
// travelled and the elapsed time. Constant speed ignores both arguments;
// variable speed (acceleration, slowing into corners, a scripted profile,
// etc.) is expressed by returning different values.
using SpeedModel = std::function<double(double distance, double time)>;

// Convenience: a model that always returns the same (integer) speed.
SpeedModel constant_speed(int cells_per_second);

// A particle moving along a route under a speed model.
class Particle {
public:
    Particle(const Route &route, SpeedModel speed);

    // Advance the simulation by dt seconds.
    void step(double dt);

    bool finished() const;                  // reached the end of the route
    Vec3i position() const;                  // current cell (snapped to grid)
    Vec3i velocity() const;                  // current velocity vector (cells/s)
    double distance() const;                // distance travelled along route
    double time() const;                    // elapsed simulated time

private:
    const Route &route_;
    SpeedModel speed_;
    double distance_ = 0.0;
    double time_ = 0.0;
    Vec3 position_;                          // continuous, snapped on access
};

// One captured sample of the particle's state, in integer units.
struct Frame {
    double time;
    int distance;        // distance travelled along route (cells)
    Vec3i position;      // grid cell
    Vec3i velocity;      // velocity vector (cells/s)
    short value;         // voxel field value at this cell (what the particle "sees")
};

// Run the simulation to completion at a fixed timestep, capturing one frame
// per step (plus a final frame at the end). At every step the particle samples
// `field` at the cell it occupies, recorded into Frame::value. The result is a
// self-contained trajectory that a viewer can replay without re-running physics.
std::vector<Frame> record(const Route &route, SpeedModel speed, double dt,
                          const VoxelField &field);

#endif // SIM_HPP
