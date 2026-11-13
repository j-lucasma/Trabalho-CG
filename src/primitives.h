#pragma once
#include "ray.h"
#include "vec3.h"
#include "mat4.h"
#include "material.h"
#include <vector>
#include <limits>
#include <cfloat>

struct HitRecord {
    float t = FLT_MAX;
    Vec3  point;
    Vec3  normal;
    Vec3  color;
    float u = 0, v = 0;
    int   materialId = 0;
    bool  hit = false;
};

enum class PrimType { Sphere, Cylinder, Cone, Plane, Mesh };

struct Triangle {
    Vec3 v0, v1, v2;
    Vec3 n0, n1, n2;
    float u0, v0u, u1, v1u, u2, v2u; // UVs por vértice
};

struct Primitive {
    PrimType type = PrimType::Sphere;
    int materialId = 0;
    Mat4 transform;        // T·R·S (ou qualquer combinação) — itens 1.4.x
    Mat4 invTransform;     // cache
    Mat4 normalMatrix;     // cache

    // Parâmetros locais (no espaço do objeto, ANTES da transform)
    Vec3  center{0,0,0};
    float radius = 1.0f;
    float height = 1.0f;    // cilindro/cone
    Vec3  planeNormal{0,1,0};
    float planeD = 0.0f;

    // Malha (triângulos em espaço LOCAL)
    std::vector<Triangle> triangles;

    void bake() {
        invTransform  = transform.inverse();
        normalMatrix  = transform.normalMatrix();
    }

    // ---- Interseções (em espaço local) ----
    bool intersectSphere(const Ray& r, float& t, Vec3& n) const {
        Vec3 oc = r.origin - center;
        float a = dot(r.dir, r.dir);
        float b = 2.0f * dot(oc, r.dir);
        float c = dot(oc, oc) - radius*radius;
        float disc = b*b - 4*a*c;
        if (disc < 0) return false;
        float sq = std::sqrt(disc);
        float t0 = (-b - sq) / (2*a);
        float t1 = (-b + sq) / (2*a);
        t = (t0 > 1e-4f) ? t0 : ((t1 > 1e-4f) ? t1 : -1);
        if (t < 0) return false;
        Vec3 p = r.at(t);
        n = normalize(p - center);
        return true;
    }

    // Cilindro vertical (eixo y), raio = 1, y em [0, height]
    bool intersectCylinder(const Ray& r, float& t, Vec3& n) const {
        float ox = r.origin.x - center.x;
        float oz = r.origin.z - center.z;
        float dx = r.dir.x, dz = r.dir.z;
        float a = dx*dx + dz*dz;
        if (a < 1e-6f) return false;
        float b = 2*(ox*dx + oz*dz);
        float c = ox*ox + oz*oz - radius*radius;
        float disc = b*b - 4*a*c;
        if (disc < 0) return false;
        float sq = std::sqrt(disc);
        float candidates[2] = {(-b-sq)/(2*a), (-b+sq)/(2*a)};

        float bestT = FLT_MAX;
        Vec3 bestN;
        for (float tt : candidates) {
            if (tt < 1e-4f) continue;
            float y = r.origin.y + tt*r.dir.y - center.y;
            if (y >= 0 && y <= height) {
                if (tt < bestT) {
                    bestT = tt;
                    Vec3 p = r.at(tt);
                    bestN = normalize(Vec3(p.x - center.x, 0, p.z - center.z));
                }
            }
        }
        // Tampos
        for (float yPlane : {0.0f, height}) {
            if (std::abs(r.dir.y) < 1e-6f) continue;
            float tt = (center.y + yPlane - r.origin.y) / r.dir.y;
            if (tt < 1e-4f) continue;
            Vec3 p = r.at(tt);
            float ddx = p.x - center.x, ddz = p.z - center.z;
            if (ddx*ddx + ddz*ddz <= radius*radius && tt < bestT) {
                bestT = tt;
                bestN = (yPlane == 0) ? Vec3(0,-1,0) : Vec3(0,1,0);
            }
        }
        if (bestT == FLT_MAX) return false;
        t = bestT; n = bestN;
        return true;
    }

    // Cone vertical (eixo y), base em y=0 com raio, ápice em y=height
    bool intersectCone(const Ray& r, float& t, Vec3& n) const {
        float k = radius / height; // inclinação lateral
        float ox = r.origin.x - center.x;
        float oy = r.origin.y - center.y;
        float oz = r.origin.z - center.z;
        float dx = r.dir.x, dy = r.dir.y, dz = r.dir.z;
        float a = dx*dx + dz*dz - k*k*dy*dy;
        float b = 2*(ox*dx + oz*dz - k*k*(oy - height)*dy);
        float c = ox*ox + oz*oz - k*k*(oy - height)*(oy - height);
        float bestT = FLT_MAX;
        Vec3 bestN;

        if (std::abs(a) > 1e-6f) {
            float disc = b*b - 4*a*c;
            if (disc >= 0) {
                float sq = std::sqrt(disc);
                for (float tt : {(-b-sq)/(2*a), (-b+sq)/(2*a)}) {
                    if (tt < 1e-4f) continue;
                    float y = oy + tt*dy;
                    if (y >= 0 && y <= height && tt < bestT) {
                        bestT = tt;
                        Vec3 p = r.at(tt);
                        Vec3 cp(p.x - center.x, p.y - center.y, p.z - center.z);
                        // normal = gradiente de x²+z² - k²(y-h)²
                        bestN = normalize(Vec3(cp.x, k*k*(cp.y - height), cp.z));
                    }
                }
            }
        }
        // Base
        if (std::abs(dy) > 1e-6f) {
            float tt = (center.y - r.origin.y) / dy;
            if (tt > 1e-4f && tt < bestT) {
                Vec3 p = r.at(tt);
                float ddx = p.x - center.x, ddz = p.z - center.z;
                if (ddx*ddx + ddz*ddz <= radius*radius) {
                    bestT = tt; bestN = Vec3(0,-1,0);
                }
            }
        }
        if (bestT == FLT_MAX) return false;
        t = bestT; n = bestN;
        return true;
    }

    bool intersectPlane(const Ray& r, float& t, Vec3& n) const {
        float denom = dot(planeNormal, r.dir);
        if (std::abs(denom) < 1e-6f) return false;
        t = (planeD - dot(planeNormal, r.origin)) / denom;
        if (t < 1e-4f) return false;
        n = normalize(planeNormal);
        return true;
    }

    bool intersectTriangle(const Ray& r, const Triangle& tri, float& t, Vec3& n,
                           float& u, float& v) const {
        Vec3 e1 = tri.v1 - tri.v0;
        Vec3 e2 = tri.v2 - tri.v0;
        Vec3 p  = cross(r.dir, e2);
        float det = dot(e1, p);
        if (std::abs(det) < 1e-8f) return false;
        float invDet = 1.0f / det;
        Vec3 T = r.origin - tri.v0;
        u = dot(T, p) * invDet;
        if (u < 0 || u > 1) return false;
        Vec3 Q = cross(T, e1);
        v = dot(r.dir, Q) * invDet;
        if (v < 0 || u + v > 1) return false;
        t = dot(e2, Q) * invDet;
        if (t < 1e-4f) return false;
        Vec3 bary = Vec3(1-u-v, u, v); // (n0,n1,n2)
        n = normalize(tri.n0*bary.x + tri.n1*bary.y + tri.n2*bary.z);
        return true;
    }

    bool intersectMesh(const Ray& r, float& t, Vec3& n, float& u, float& v) const {
        float bestT = FLT_MAX;
        Vec3 bestN; float bu=0, bv=0;
        for (auto& tri : triangles) {
            float tt, uu, vv; Vec3 nn;
            if (intersectTriangle(r, tri, tt, nn, uu, vv) && tt < bestT) {
                bestT = tt; bestN = nn; bu = uu; bv = vv;
            }
        }
        if (bestT == FLT_MAX) return false;
        t = bestT; n = bestN; u = bu; v = bv;
        return true;
    }

    // Interseção com transformação: leva o raio pro espaço do objeto
    bool intersect(const Ray& worldRay, HitRecord& rec) const {
        Ray r(worldRay.origin, worldRay.dir);
        Vec3 o = invTransform.transformPoint(worldRay.origin);
        Vec3 d = invTransform.transformDir(worldRay.dir);
        Ray local(o, d);

        float t, u = 0, v = 0; Vec3 n;
        bool ok = false;

        switch (type) {
            case PrimType::Sphere:   ok = intersectSphere(local, t, n); break;
            case PrimType::Cylinder: ok = intersectCylinder(local, t, n); break;
            case PrimType::Cone:     ok = intersectCone(local, t, n); break;
            case PrimType::Plane:    ok = intersectPlane(local, t, n); break;
            case PrimType::Mesh:     ok = intersectMesh(local, t, n, u, v); break;
        }
        if (!ok) return false;

        Vec3 localP = local.at(t);
        Vec3 worldP = transform.transformPoint(localP);
        Vec3 worldN = normalize(normalMatrix.transformDir(n));

        // t no espaço local == t no espaço do mundo porque o raio do mundo
        // não foi renormalizado (a direção local já traz o fator).
        // Reconstruímos o t do mundo a partir do ponto:
        float tWorld = dot(worldP - worldRay.origin, worldRay.dir);
        if (tWorld < 1e-4f) return false;

        rec.t = tWorld;
        rec.point = worldP;
        rec.normal = worldN;
        rec.u = u;
        rec.v = v;
        rec.hit = true;
        rec.materialId = materialId;

        // UVs por primitiva
        if (type == PrimType::Sphere) {
            Vec3 lp = normalize(localP - center);
            rec.u = 0.5f + std::atan2(lp.z, lp.x) / (2*M_PI);
            rec.v = 0.5f - std::asin(lp.y) / M_PI;
        } else if (type == PrimType::Cylinder) {
            Vec3 lp = localP - center;
            rec.u = 0.5f + std::atan2(lp.z, lp.x) / (2*M_PI);
            rec.v = lp.y / height;
        } else if (type == PrimType::Cone) {
            Vec3 lp = localP - center;
            rec.u = 0.5f + std::atan2(lp.z, lp.x) / (2*M_PI);
            rec.v = lp.y / height;
        }
        return true;
    }

    // AABB para o requisito "tudo no 1º octante"
    void computeAABB(Vec3& mn, Vec3& mx) const {
        mn = Vec3( FLT_MAX);
        mx = Vec3(-FLT_MAX);
        auto expand = [&](const Vec3& p) {
            mn.x = std::min(mn.x, p.x); mn.y = std::min(mn.y, p.y); mn.z = std::min(mn.z, p.z);
            mx.x = std::max(mx.x, p.x); mx.y = std::max(mx.y, p.y); mx.z = std::max(mx.z, p.z);
        };
        if (type == PrimType::Mesh) {
            for (auto& t : triangles) {
                expand(transform.transformPoint(t.v0));
                expand(transform.transformPoint(t.v1));
                expand(transform.transformPoint(t.v2));
            }
        } else if (type == PrimType::Sphere) {
            Vec3 c = transform.transformPoint(center);
            float rr = radius * std::max({ std::abs(transform.m[0]), std::abs(transform.m[5]), std::abs(transform.m[10]) });
            expand(c - Vec3(rr)); expand(c + Vec3(rr));
        } else {
            Vec3 corners[8] = {
                center + Vec3(-radius, 0, -radius), center + Vec3(radius, 0, radius),
                center + Vec3(-radius, height, -radius), center + Vec3(radius, height, radius),
                center + Vec3(-radius, 0, radius), center + Vec3(radius, 0, -radius),
                center + Vec3(-radius, height, radius), center + Vec3(radius, height, -radius)
            };
            for (auto& c : corners) expand(transform.transformPoint(c));
        }
    }
};