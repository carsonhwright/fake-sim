#ifndef FAKEGEO_HPP
#define FAKEGEO_HPP

#include <vector>
#include "include/fake-math.hpp"

// Shape generators for the voxel world. Each function takes a few parameters
// describing a geometric object and returns the integer grid coordinates that
// make up that shape -- the set of cells you would write into a VoxelField.
//
// These are pure geometry: they don't touch a VoxelField and don't clamp to
// any space, so coordinates may fall outside a given world. Stamping them in is
// the caller's job (VoxelField::set already ignores out-of-bounds cells), which
// also leaves room to assign per-cell values / gradients at stamp time.
namespace geo {

// Filled axis-aligned box. `origin` is the minimum corner; width/height/depth
// are extents in cells (so the box spans origin .. origin+size-1 on each axis).
std::vector<Vec3i> box(const Vec3i &origin, int width, int height, int depth);

// Filled ellipsoid centred at `center` with per-axis radii (in cells). A cell
// is included when (dx/rx)^2 + (dy/ry)^2 + (dz/rz)^2 <= 1.
std::vector<Vec3i> ellipsoid(const Vec3i &center, int rx, int ry, int rz);

// Filled sphere: an ellipsoid with equal radii.
std::vector<Vec3i> sphere(const Vec3i &center, int radius);

// Voxelised straight line between two endpoints (3D Bresenham). Includes both
// endpoints; every cell is 6-connected to the next.
std::vector<Vec3i> line(const Vec3i &a, const Vec3i &b);

} // namespace geo

#endif // FAKEGEO_HPP
