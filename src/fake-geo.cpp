#include <stdlib.h>   // abs
#include "include/fake-geo.hpp"

namespace geo {

std::vector<Vec3i> box(const Vec3i &origin, int width, int height, int depth) {
    std::vector<Vec3i> cells;
    if (width <= 0 || height <= 0 || depth <= 0) {
        return cells;
    }
    cells.reserve((size_t)width * height * depth);
    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                cells.push_back(Vec3i{origin.x + x, origin.y + y, origin.z + z});
            }
        }
    }
    return cells;
}

std::vector<Vec3i> ellipsoid(const Vec3i &center, int rx, int ry, int rz) {
    std::vector<Vec3i> cells;
    if (rx <= 0 || ry <= 0 || rz <= 0) {
        return cells;
    }
    // Walk the bounding box and keep cells inside the ellipsoid. Comparing
    // squared, normalised distance avoids any sqrt.
    for (int dz = -rz; dz <= rz; ++dz) {
        for (int dy = -ry; dy <= ry; ++dy) {
            for (int dx = -rx; dx <= rx; ++dx) {
                double nx = (double)dx / rx;
                double ny = (double)dy / ry;
                double nz = (double)dz / rz;
                if (nx * nx + ny * ny + nz * nz <= 1.0) {
                    cells.push_back(Vec3i{center.x + dx, center.y + dy, center.z + dz});
                }
            }
        }
    }
    return cells;
}

std::vector<Vec3i> sphere(const Vec3i &center, int radius) {
    return ellipsoid(center, radius, radius, radius);
}

std::vector<Vec3i> line(const Vec3i &a, const Vec3i &b) {
    std::vector<Vec3i> cells;

    int dx = abs(b.x - a.x);
    int dy = abs(b.y - a.y);
    int dz = abs(b.z - a.z);
    int sx = (b.x > a.x) ? 1 : -1;
    int sy = (b.y > a.y) ? 1 : -1;
    int sz = (b.z > a.z) ? 1 : -1;

    Vec3i p = a;
    cells.push_back(p);

    // Drive the loop along the dominant axis; the other two axes accumulate
    // error and step when it overflows (3D extension of Bresenham's line).
    if (dx >= dy && dx >= dz) {
        int ey = 2 * dy - dx;
        int ez = 2 * dz - dx;
        while (p.x != b.x) {
            p.x += sx;
            if (ey >= 0) { p.y += sy; ey -= 2 * dx; }
            if (ez >= 0) { p.z += sz; ez -= 2 * dx; }
            ey += 2 * dy;
            ez += 2 * dz;
            cells.push_back(p);
        }
    } else if (dy >= dx && dy >= dz) {
        int ex = 2 * dx - dy;
        int ez = 2 * dz - dy;
        while (p.y != b.y) {
            p.y += sy;
            if (ex >= 0) { p.x += sx; ex -= 2 * dy; }
            if (ez >= 0) { p.z += sz; ez -= 2 * dy; }
            ex += 2 * dx;
            ez += 2 * dz;
            cells.push_back(p);
        }
    } else {
        int ex = 2 * dx - dz;
        int ey = 2 * dy - dz;
        while (p.z != b.z) {
            p.z += sz;
            if (ex >= 0) { p.x += sx; ex -= 2 * dz; }
            if (ey >= 0) { p.y += sy; ey -= 2 * dz; }
            ex += 2 * dx;
            ey += 2 * dy;
            cells.push_back(p);
        }
    }
    return cells;
}

} // namespace geo
