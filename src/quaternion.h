#pragma once
#include "vec3.h"
#include "mat4.h"
#include <cmath>

struct Quat {
    float w, x, y, z;
    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    // q = (cos(θ/2), sen(θ/2) * eixo) — requisito 1.4.2
    static Quat fromAxisAngle(const Vec3& axis, float angle) {
        Vec3 a = normalize(axis);
        float h = angle * 0.5f;
        float s = std::sin(h);
        return Quat(std::cos(h), a.x*s, a.y*s, a.z*s);
    }

    Quat operator*(const Quat& o) const {
        return Quat(
            w*o.w - x*o.x - y*o.y - z*o.z,
            w*o.x + x*o.w + y*o.z - z*o.y,
            w*o.y - x*o.z + y*o.w + z*o.x,
            w*o.z + x*o.y - y*o.x + z*o.w);
    }

    Quat conjugate() const { return Quat(w, -x, -y, -z); }

    Mat4 toMat4() const {
        Mat4 M;
        float xx = x*x, yy = y*y, zz = z*z;
        float xy = x*y, xz = x*z, yz = y*z;
        float wx = w*x, wy = w*y, wz = w*z;
        M.m[0] = 1 - 2*(yy+zz); M.m[4] = 2*(xy-wz);     M.m[8]  = 2*(xz+wy);
        M.m[1] = 2*(xy+wz);     M.m[5] = 1 - 2*(xx+zz); M.m[9]  = 2*(yz-wx);
        M.m[2] = 2*(xz-wy);     M.m[6] = 2*(yz+wx);     M.m[10] = 1 - 2*(xx+yy);
        return M;
    }
};

inline Mat4 rotateArbitrary(const Vec3& axis, float angle) {
    return Quat::fromAxisAngle(axis, angle).toMat4();
}