#ifndef FAKEMATH_HPP
#define FAKEMATH_HPP

#include <stddef.h>

// A continuous point/vector in 3D space. Used internally for geometry and
// arc-length interpolation, where sub-cell precision is needed.
struct Vec3 {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	Vec3 operator+(const Vec3 &rhs) const;
	Vec3 operator-(const Vec3 &rhs) const;
	Vec3 operator*(double s) const;   // scale

	double length() const;            // magnitude
	Vec3 normalized() const;          // unit vector (zero vector stays zero)
};

// An integer grid coordinate ("pixel"/voxel). This is the externally-visible
// coordinate system: routes are defined in it and the simulation reports
// positions and velocities in it.
struct Vec3i {
	int x = 0;
	int y = 0;
	int z = 0;

	Vec3i operator+(const Vec3i &rhs) const;
	Vec3i operator-(const Vec3i &rhs) const;
};

// Promote an integer cell to a continuous point (exact).
Vec3 to_vec3(const Vec3i &v);
// Snap a continuous point to the nearest integer grid cell.
Vec3i round_to_grid(const Vec3 &v);

class Base {
public:
	Base(int param1, int param2);
	Base(int param1);

	void show_params();
	int get_param(const char *param_name);

private:
	int parameter1;
	int parameter2;
};

#endif // FAKEMATH_HPP