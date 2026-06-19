#include <math.h>
#include "include/sim.hpp"

long PixelSpace::cell_count() const {
    return (long)width * (long)height * (long)depth;
}

// Coordinates are non-negative and bounds-checked before storage, so we pack
// (x, y, z) into one 64-bit key with 21 bits each (up to 2,097,151 per axis).
static const uint64_t COORD_BITS = 21;
static const uint64_t COORD_MASK = (1ull << COORD_BITS) - 1;

VoxelField::VoxelField(const PixelSpace &space, short default_val)
    : default_value(default_val), space_(space) {}

uint64_t VoxelField::key(int x, int y, int z) const {
    return (uint64_t)x
         | ((uint64_t)y << COORD_BITS)
         | ((uint64_t)z << (2 * COORD_BITS));
}

bool VoxelField::in_bounds(int x, int y, int z) const {
    return x >= 0 && x < space_.width
        && y >= 0 && y < space_.height
        && z >= 0 && z < space_.depth;
}

bool VoxelField::in_bounds(const Vec3i &c) const {
    return in_bounds(c.x, c.y, c.z);
}

short VoxelField::get(int x, int y, int z) const {
    if (!in_bounds(x, y, z)) {
        return default_value;
    }
    auto it = data_.find(key(x, y, z));
    return (it == data_.end()) ? default_value : it->second;
}

short VoxelField::get(const Vec3i &c) const {
    return get(c.x, c.y, c.z);
}

void VoxelField::set(int x, int y, int z, short value) {
    if (!in_bounds(x, y, z)) {
        return;
    }
    if (value == default_value) {
        data_.erase(key(x, y, z));   // keep storage sparse
    } else {
        data_[key(x, y, z)] = value;
    }
}

void VoxelField::set(const Vec3i &c, short value) {
    set(c.x, c.y, c.z, value);
}

std::vector<std::pair<Vec3i, short>> VoxelField::entries() const {
    std::vector<std::pair<Vec3i, short>> out;
    out.reserve(data_.size());
    for (const auto &kv : data_) {
        Vec3i c{(int)(kv.first & COORD_MASK),
                (int)((kv.first >> COORD_BITS) & COORD_MASK),
                (int)((kv.first >> (2 * COORD_BITS)) & COORD_MASK)};
        out.push_back({c, kv.second});
    }
    return out;
}

void Route::add_waypoint(const Vec3i &p) {
    Vec3 cp = to_vec3(p);
    if (waypoints_.empty()) {
        cumulative_.push_back(0.0);
    } else {
        double seg = (cp - waypoints_.back()).length();
        cumulative_.push_back(cumulative_.back() + seg);
    }
    waypoints_.push_back(cp);
}

size_t Route::num_waypoints() const {
    return waypoints_.size();
}

double Route::length() const {
    return cumulative_.empty() ? 0.0 : cumulative_.back();
}

Vec3 Route::position_at(double distance) const {
    if (waypoints_.empty()) {
        return Vec3{};
    }
    // Clamp to the route's extent.
    if (distance <= 0.0) {
        return waypoints_.front();
    }
    if (distance >= length()) {
        return waypoints_.back();
    }

    // Find the segment [i-1, i] containing `distance`. Linear scan is fine for
    // modest routes; swap for std::upper_bound if routes get large.
    size_t i = 1;
    while (i < cumulative_.size() && cumulative_[i] < distance) {
        ++i;
    }

    double seg_start = cumulative_[i - 1];
    double seg_len = cumulative_[i] - seg_start;
    double t = (seg_len > 0.0) ? (distance - seg_start) / seg_len : 0.0;

    const Vec3 &a = waypoints_[i - 1];
    const Vec3 &b = waypoints_[i];
    return a + (b - a) * t;
}

Vec3 Route::direction_at(double distance) const {
    if (waypoints_.size() < 2) {
        return Vec3{};
    }
    if (distance < 0.0) distance = 0.0;
    if (distance > length()) distance = length();

    size_t i = 1;
    while (i < cumulative_.size() && cumulative_[i] < distance) {
        ++i;
    }
    return (waypoints_[i] - waypoints_[i - 1]).normalized();
}

SpeedModel constant_speed(int cells_per_second) {
    double v = (double)cells_per_second;
    return [v](double, double) { return v; };
}

Particle::Particle(const Route &route, SpeedModel speed)
    : route_(route), speed_(std::move(speed)) {
    position_ = route_.position_at(0.0);
}

void Particle::step(double dt) {
    if (finished()) {
        return;
    }
    double v = speed_(distance_, time_);
    distance_ += v * dt;
    time_ += dt;

    double total = route_.length();
    if (distance_ > total) {
        distance_ = total;            // clamp so we land exactly on the end
    }
    position_ = route_.position_at(distance_);
}

bool Particle::finished() const {
    return distance_ >= route_.length();
}

Vec3i Particle::position() const {
    return round_to_grid(position_);
}

Vec3i Particle::velocity() const {
    double v = speed_(distance_, time_);
    Vec3 dir = route_.direction_at(distance_);
    return round_to_grid(dir * v);
}

double Particle::distance() const {
    return distance_;
}

double Particle::time() const {
    return time_;
}

std::vector<Frame> record(const Route &route, SpeedModel speed, double dt,
                          const VoxelField &field) {
    std::vector<Frame> frames;
    Particle p(route, std::move(speed));
    for (;;) {
        Vec3i cell = p.position();
        frames.push_back(Frame{p.time(), (int)lround(p.distance()),
                               cell, p.velocity(), field.get(cell)});
        if (p.finished()) {
            break;
        }
        p.step(dt);
    }
    return frames;
}
