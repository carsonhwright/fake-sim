#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <unordered_map>
#include "include/fake-math.hpp"

Vec3 Vec3::operator+(const Vec3 &rhs) const {
    return Vec3{x + rhs.x, y + rhs.y, z + rhs.z};
}

Vec3 Vec3::operator-(const Vec3 &rhs) const {
    return Vec3{x - rhs.x, y - rhs.y, z - rhs.z};
}

Vec3 Vec3::operator*(double s) const {
    return Vec3{x * s, y * s, z * s};
}

double Vec3::length() const {
    return sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    double len = length();
    if (len == 0.0) {
        return Vec3{0.0, 0.0, 0.0};
    }
    return Vec3{x / len, y / len, z / len};
}

Vec3i Vec3i::operator+(const Vec3i &rhs) const {
    return Vec3i{x + rhs.x, y + rhs.y, z + rhs.z};
}

Vec3i Vec3i::operator-(const Vec3i &rhs) const {
    return Vec3i{x - rhs.x, y - rhs.y, z - rhs.z};
}

Vec3 to_vec3(const Vec3i &v) {
    return Vec3{(double)v.x, (double)v.y, (double)v.z};
}

Vec3i round_to_grid(const Vec3 &v) {
    return Vec3i{(int)lround(v.x), (int)lround(v.y), (int)lround(v.z)};
}

enum class ParamSelect {param1, param2, unknown};

static ParamSelect to_param_select(const char *param_name) {
    static const std::unordered_map<std::string, ParamSelect> param_map = {
        {"param1", ParamSelect::param1},
        {"param2", ParamSelect::param2},
    };

    auto it = param_map.find(param_name);
    if (it != param_map.end()) {
        return it->second;
    }
    return ParamSelect::unknown;
}

Base::Base(int param1, int param2) {
    parameter1 = param1;
    parameter2 = param2;
}

Base::Base(int param1) : Base(0, param1) {}

void Base::show_params() {
    printf("param1: %d\nparam2: %d\n", this->parameter1, this->parameter2);
    fflush(stdout);
}

int Base::get_param(const char *param_name) {
    switch(to_param_select(param_name)) {
        case ParamSelect::param1 :
            return parameter1;
        case ParamSelect::param2 :
            return parameter2;
        default:
            return -1;
    }

    return 0;
}