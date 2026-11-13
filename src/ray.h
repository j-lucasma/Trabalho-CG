#pragma once
#include "vec3.h"

struct Ray {
    Vec3 origin, dir;
    Ray() {}
    Ray(const Vec3& o, const Vec3& d) : origin(o), dir(normalize(d)) {}
    Vec3 at(float t) const { return origin + dir * t; }
};