#pragma once
#include <cmath>
#include <algorithm>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    explicit Vec3(float s) : x(s), y(s), z(s) {}

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    Vec3 operator*(const Vec3& o) const { return {x*o.x, y*o.y, z*o.z}; }
    Vec3 operator/(float s) const { return {x/s, y/s, z/s}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3& operator+=(const Vec3& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    Vec3& operator*=(float s) { x*=s; y*=s; z*=s; return *this; }

    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const { float l = length(); return l > 1e-8f ? (*this)/l : Vec3(0); }
};

inline float dot(const Vec3& a, const Vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}
inline Vec3 normalize(const Vec3& v) { return v.normalized(); }
inline Vec3 reflect(const Vec3& I, const Vec3& N) { return I - N * (2.0f * dot(I, N)); }
inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) { return a*(1-t) + b*t; }
inline Vec3 clampColor(const Vec3& c) {
    return { std::min(1.0f, std::max(0.0f, c.x)),
             std::min(1.0f, std::max(0.0f, c.y)),
             std::min(1.0f, std::max(0.0f, c.z)) };
}
// Permite float * Vec3 (necessário em camera.h e renderer.h)
inline Vec3 operator*(float s, const Vec3& v) { return v * s; }
