#pragma once
#include "scene.h"
#include "camera.h"
#include <vector>
#include <cstdint>
#include <cmath>
#include <random>
#include <functional>

struct Renderer {
    const Scene& scene;
    const Camera& cam;
    int maxDepth = 3;
    int aaSamples = 4; // 2x2 — pode aumentar para 4x4 (bônus beleza)

    Renderer(const Scene& s, const Camera& c) : scene(s), cam(c) {}

    // ---- Interseção primária (com pick e devolução de id) ----
    bool traceClosest(const Ray& r, HitRecord& out, int& hitPrimId) const {
        bool any = false;
        float best = FLT_MAX;
        for (int i = 0; i < (int)scene.prims.size(); ++i) {
            HitRecord rec;
            if (scene.prims[i].intersect(r, rec) && rec.t < best) {
                best = rec.t;
                out = rec;
                hitPrimId = i;
                any = true;
            }
        }
        return any;
    }

    // Sombra: dispara raio da luz para o ponto — requisito 4
    bool inShadow(const Vec3& P, const Vec3& N, const Vec3& L, float maxDist) const {
        Vec3 dir = L - P;
        float dist = dir.length();
        if (dist < 1e-4f) return false;
        Ray sr(P + N * 1e-3f, dir / dist);
        for (auto& p : scene.prims) {
            HitRecord rec;
            if (p.intersect(sr, rec) && rec.t < dist - 1e-3f) return true;
        }
        return false;
    }

    // Iluminação local — ambient + point + spot + directional
    Vec3 shade(const HitRecord& rec, const Vec3& viewDir) const {
        const Material& mat = scene.materials[rec.materialId];

        Vec3 baseColor = mat.kd;
        if (mat.textureId >= 0) {
            const Texture* tex = scene.textures.get(mat.textureId);
            if (tex) baseColor = baseColor * tex->sample(rec.u, rec.v);
        }

        Vec3 color(0);
        Vec3 N = rec.normal;
        if (dot(N, viewDir) < 0) N = -N; // normal sempre pro observador

        for (const auto& l : scene.lights) {
            if (l.type == LightType::Ambient) {
                color += mat.ka * l.color;
                continue;
            }

            Vec3 Ldir;
            float atten = 1.0f;
            float cosFactor = 1.0f;
            float maxDist = FLT_MAX;

            if (l.type == LightType::Directional) {
                Ldir = -l.direction;
            } else {
                Vec3 toL = l.position - rec.point;
                float dist = toL.length();
                Ldir = toL / dist;
                maxDist = dist;
                atten = 1.0f / (l.attA + l.attB*dist + l.attC*dist*dist);

                if (l.type == LightType::Spot) {
                    float cosAngle = dot(-Ldir, l.direction);
                    if (cosAngle < l.cosOuter) continue;
                    float t = (cosAngle - l.cosOuter) / (l.cosInner - l.cosOuter);
                    cosFactor = std::min(1.0f, std::max(0.0f, t));
                }
            }

            if (inShadow(rec.point, N, l.type == LightType::Directional ? rec.point + Ldir * 1e6f : l.position, maxDist))
                continue;

            float ndotl = std::max(0.0f, dot(N, Ldir));
            if (ndotl <= 0) continue;

            Vec3 H = normalize(Ldir + viewDir);
            float spec = std::pow(std::max(0.0f, dot(N, H)), mat.shininess);

            Vec3 lightContrib = (baseColor * ndotl + mat.ks * spec) * l.color * (l.intensity * atten * cosFactor);
            color += lightContrib;
        }
        return color;
    }

    // Ray casting recursivo com reflexão (bônus beleza — metal/espelho)
    Vec3 trace(const Ray& r, int depth) const {
        HitRecord rec;
        int pid;
        if (!traceClosest(r, rec, pid)) {
            // Background: gradiente vertical (bônus beleza)
            float t = 0.5f * (r.dir.y + 1.0f);
            return lerp(Vec3(0.95f, 0.95f, 0.98f), Vec3(0.55f, 0.7f, 0.95f), t);
        }

        Vec3 color = shade(rec, -r.dir);

        // Reflexão
        const Material& mat = scene.materials[rec.materialId];
        if (mat.reflectivity > 0 && depth < maxDepth) {
            Vec3 refDir = reflect(r.dir, rec.normal);
            Ray refRay(rec.point + rec.normal * 1e-3f, refDir);
            Vec3 refColor = trace(refRay, depth + 1);
            color = lerp(color, refColor, mat.reflectivity);
        }
        return color;
    }

    // Renderiza a imagem inteira (>= 500x500) com AA — requisitos 6 e 7
    std::vector<uint8_t> render(int W, int H, const std::function<void(int)>& progress = {}) const {
        std::vector<uint8_t> pixels(W * H * 3);
        int aa = std::max(1, aaSamples);
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                Vec3 acc(0);
                for (int sy = 0; sy < aa; ++sy) {
                    for (int sx = 0; sx < aa; ++sx) {
                        // Sub-pixel jitter — bônus AA
                        float jx = (sx + 0.5f) / aa;
                        float jy = (sy + 0.5f) / aa;
                        Ray r;
                        // Reaproveita primaryRay com coordenadas flutuantes
                        Vec3 u, v, w;
                        cam.basis(u, v, w);
                        float uc = cam.xmin + (cam.xmax - cam.xmin) * (i + jx) / W;
                        float vc = cam.ymin + (cam.ymax - cam.ymin) * (j + jy) / H;
                        if (cam.mode == ProjectionMode::Perspective) {
                            Vec3 d = normalize(-cam.d * w + uc * u + vc * v);
                            r = Ray(cam.eye, d);
                        } else if (cam.mode == ProjectionMode::Orthographic) {
                            r = Ray(cam.eye + uc * u + vc * v, -w);
                        } else {
                            float dx = cam.obliqK * std::cos(cam.obliqAngle);
                            float dy = cam.obliqK * std::sin(cam.obliqAngle);
                            r = Ray(cam.eye + (uc+dx)*u + (vc+dy)*v, -w);
                        }
                        acc += trace(r, 0);
                    }
                }
                acc = acc / (float)(aa*aa);
                acc = clampColor(acc);
                // Gamma 2.2 — bônus beleza
                auto g = [](float c){ return std::pow(c, 1.0f/2.2f); };
                size_t idx = ((size_t)j * W + i) * 3;
                pixels[idx+0] = (uint8_t)(255.0f * g(acc.x));
                pixels[idx+1] = (uint8_t)(255.0f * g(acc.y));
                pixels[idx+2] = (uint8_t)(255.0f * g(acc.z));
            }
            if (progress) progress(j);
        }
        return pixels;
    }

    // Pick — requisito 5.1
    int pick(int pixelX, int pixelY) const {
        Vec3 u, v, w; cam.basis(u, v, w);
        float uc = cam.xmin + (cam.xmax - cam.xmin) * (pixelX + 0.5f) / cam.W;
        float vc = cam.ymin + (cam.ymax - cam.ymin) * (pixelY + 0.5f) / cam.H;
        Ray r;
        if (cam.mode == ProjectionMode::Perspective) {
            r = Ray(cam.eye, normalize(-cam.d * w + uc * u + vc * v));
        } else if (cam.mode == ProjectionMode::Orthographic) {
            r = Ray(cam.eye + uc * u + vc * v, -w);
        } else {
            float dx = cam.obliqK * std::cos(cam.obliqAngle);
            float dy = cam.obliqK * std::sin(cam.obliqAngle);
            r = Ray(cam.eye + (uc+dx)*u + (vc+dy)*v, -w);
        }
        HitRecord rec; int pid = -1;
        if (traceClosest(r, rec, pid)) return pid;
        return -1;
    }
};