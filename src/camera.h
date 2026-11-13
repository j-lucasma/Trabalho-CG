#pragma once
#include "ray.h"
#include "vec3.h"

enum class ProjectionMode { Perspective, Orthographic, Oblique };

// Câmera com eye / at / up / d / janela [xmin,xmax,ymin,ymax] — requisitos 2.1, 2.2, 3.1–3.3
struct Camera {
    Vec3 eye{0, 1.5f, 4.0f};
    Vec3 at{0, 1.0f, 0.0f};
    Vec3 up{0, 1.0f, 0.0f};
    float d = 1.0f;
    float xmin = -0.5f, xmax = 0.5f, ymin = -0.5f, ymax = 0.5f;

    ProjectionMode mode = ProjectionMode::Perspective;

    // Parâmetros da oblíqua (Cavalier/Cabinet)
    float obliqAngle = 45.0f * (float)M_PI / 180.0f;
    float obliqK = 0.5f;

    int W = 800, H = 800; // >= 500x500 — requisito 6

    // Zoom: mexe na janela de visão
    void zoomIn(float factor = 0.8f) {
        xmin *= factor; xmax *= factor;
        ymin *= factor; ymax *= factor;
    }
    void zoomOut(float factor = 1.25f) {
        xmin *= factor; xmax *= factor;
        ymin *= factor; ymax *= factor;
    }

    // Base ortonormal (u, v, w) da câmera
    void basis(Vec3& u, Vec3& v, Vec3& w) const {
        w = normalize(eye - at);
        u = normalize(cross(up, w));
        v = cross(w, u);
    }

    // Raio por pixel — cobre 3.1 (perspectiva), 3.2 (ortográfica), 3.3 (oblíqua)
    Ray primaryRay(int i, int j) const {
        Vec3 u, v, w; basis(u, v, w);

        float uc = xmin + (xmax - xmin) * (i + 0.5f) / W;
        float vc = ymin + (ymax - ymin) * (j + 0.5f) / H;

        if (mode == ProjectionMode::Perspective) {
            Vec3 dir = normalize(-d * w + uc * u + vc * v);
            return Ray(eye, dir);
        } else if (mode == ProjectionMode::Orthographic) {
            Vec3 orig = eye + uc * u + vc * v;
            return Ray(orig, -w);
        } else { // Oblique — desloca o ponto de partida na janela
            float dx = obliqK * std::cos(obliqAngle);
            float dy = obliqK * std::sin(obliqAngle);
            Vec3 orig = eye + (uc + dx) * u + (vc + dy) * v;
            return Ray(orig, -w);
        }
    }
};