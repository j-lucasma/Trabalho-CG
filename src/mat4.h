#pragma once
#include "vec3.h"
#include <cstring>

// Matriz 4x4 column-major (como OpenGL) — usamos para transformações.
struct Mat4 {
    float m[16]; // m[col*4 + row]

    Mat4() { identity(); }

    void identity() {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Mat4 translation(const Vec3& t) {
        Mat4 M;
        M.m[12] = t.x; M.m[13] = t.y; M.m[14] = t.z;
        return M;
    }

    static Mat4 scale(const Vec3& s) {
        Mat4 M;
        M.m[0] = s.x; M.m[5] = s.y; M.m[10] = s.z;
        return M;
    }

    static Mat4 rotateX(float a) {
        Mat4 M; float c = std::cos(a), s = std::sin(a);
        M.m[5] = c; M.m[6] = s; M.m[9] = -s; M.m[10] = c;
        return M;
    }
    static Mat4 rotateY(float a) {
        Mat4 M; float c = std::cos(a), s = std::sin(a);
        M.m[0] = c; M.m[2] = -s; M.m[8] = s; M.m[10] = c;
        return M;
    }
    static Mat4 rotateZ(float a) {
        Mat4 M; float c = std::cos(a), s = std::sin(a);
        M.m[0] = c; M.m[1] = s; M.m[4] = -s; M.m[5] = c;
        return M;
    }

    // Cisalhamento (shear) — item 1.4.4 (+0.5)
    static Mat4 shear(float kxy, float kxz, float kyx, float kyz, float kzx, float kzy) {
        Mat4 M;
        M.m[4] = kxy;  // x += kxy * y  (coluna y, linha x)
        M.m[8] = kxz;  // x += kxz * z
        M.m[1] = kyx;  // y += kyx * x
        M.m[9] = kyz;
        M.m[2] = kzx;
        M.m[6] = kzy;
        return M;
    }

    // Espelho em relação a plano arbitrário n·p + d = 0 — item 1.4.5 (+0.5)
    static Mat4 mirrorPlane(const Vec3& n_, float d) {
        Vec3 n = normalize(n_);
        Mat4 M;
        M.m[0] = 1 - 2*n.x*n.x; M.m[4] = -2*n.x*n.y;    M.m[8]  = -2*n.x*n.z;
        M.m[1] = -2*n.y*n.x;    M.m[5] = 1 - 2*n.y*n.y; M.m[9]  = -2*n.y*n.z;
        M.m[2] = -2*n.z*n.x;    M.m[6] = -2*n.z*n.y;    M.m[10] = 1 - 2*n.z*n.z;
        M.m[12] = -2*n.x*d; M.m[13] = -2*n.y*d; M.m[14] = -2*n.z*d;
        return M;
    }

    Mat4 operator*(const Mat4& o) const {
        Mat4 R;
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r) {
                float s = 0;
                for (int k = 0; k < 4; ++k) s += m[k*4+r] * o.m[c*4+k];
                R.m[c*4+r] = s;
            }
        return R;
    }

    Vec3 transformPoint(const Vec3& p) const {
        float x = m[0]*p.x + m[4]*p.y + m[8]*p.z  + m[12];
        float y = m[1]*p.x + m[5]*p.y + m[9]*p.z  + m[13];
        float z = m[2]*p.x + m[6]*p.y + m[10]*p.z + m[14];
        float w = m[3]*p.x + m[7]*p.y + m[11]*p.z + m[15];
        if (std::abs(w) > 1e-8f) { x/=w; y/=w; z/=w; }
        return {x, y, z};
    }

    Vec3 transformDir(const Vec3& v) const {
        return { m[0]*v.x + m[4]*v.y + m[8]*v.z,
                 m[1]*v.x + m[5]*v.y + m[9]*v.z,
                 m[2]*v.x + m[6]*v.y + m[10]*v.z };
    }

    // Inversa geral 4x4 (Gauss-Jordan)
    Mat4 inverse() const {
        double a[4][8];
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) a[r][c] = m[c*4+r];
            for (int c = 0; c < 4; ++c) a[r][c+4] = (r == c) ? 1.0 : 0.0;
        }
        for (int i = 0; i < 4; ++i) {
            int piv = i;
            for (int r = i+1; r < 4; ++r)
                if (std::abs(a[r][i]) > std::abs(a[piv][i])) piv = r;
            if (std::abs(a[piv][i]) < 1e-12) return Mat4();
            if (piv != i) for (int c = 0; c < 8; ++c) std::swap(a[i][c], a[piv][c]);
            double div = a[i][i];
            for (int c = 0; c < 8; ++c) a[i][c] /= div;
            for (int r = 0; r < 4; ++r) {
                if (r == i) continue;
                double f = a[r][i];
                for (int c = 0; c < 8; ++c) a[r][c] -= f * a[i][c];
            }
        }
        Mat4 R;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) R.m[c*4+r] = (float)a[r][c+4];
        return R;
    }

    // Transposta da inversa — para normais
    Mat4 normalMatrix() const {
        Mat4 inv = inverse();
        Mat4 T;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) T.m[r*4+c] = inv.m[c*4+r];
        return T;
    }
};