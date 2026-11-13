#pragma once
#include "vec3.h"

// 4+ materiais distintos são instâncias desta struct — requisito 1.3.2
struct Material {
    Vec3 ka{0.1f, 0.1f, 0.1f};       // ambiente
    Vec3 kd{0.7f, 0.7f, 0.7f};       // difuso
    Vec3 ks{0.3f, 0.3f, 0.3f};       // especular
    float shininess = 32.0f;
    float reflectivity = 0.0f;       // 0..1 (para efeito "espelho")
    int   textureId = -1;            // -1 = sem textura

    // Materiais prontos (>= 4 distintos) — requisito 1.3.2
    static Material plasticRed() {
        Material m; m.ka={0.1f,0.02f,0.02f}; m.kd={0.85f,0.15f,0.15f};
        m.ks={0.6f,0.6f,0.6f}; m.shininess=64.0f; return m;
    }
    static Material metal() {
        Material m; m.ka={0.15f,0.15f,0.18f}; m.kd={0.55f,0.55f,0.6f};
        m.ks={0.95f,0.95f,0.95f}; m.shininess=128.0f; m.reflectivity=0.45f;
        return m;
    }
    static Material woodTextured(int tex) {
        Material m; m.ka={0.12f,0.08f,0.05f}; m.kd={0.6f,0.4f,0.25f};
        m.ks={0.15f,0.15f,0.15f}; m.shininess=16.0f; m.textureId=tex; return m;
    }
    static Material glass() {
        Material m; m.ka={0.05f,0.08f,0.1f}; m.kd={0.2f,0.4f,0.5f};
        m.ks={1.0f,1.0f,1.0f}; m.shininess=256.0f; m.reflectivity=0.7f;
        return m;
    }
    static Material emerald() {
        Material m; m.ka={0.02f,0.1f,0.05f}; m.kd={0.1f,0.8f,0.4f};
        m.ks={0.9f,0.9f,0.9f}; m.shininess=96.0f; return m;
    }
};