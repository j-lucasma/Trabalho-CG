#pragma once
#include "vec3.h"

enum class LightType { Point, Spot, Directional, Ambient };

struct Light {
    LightType type = LightType::Point;
    Vec3 position{0,0,0};       // para Point/Spot
    Vec3 direction{0,-1,0};     // para Spot/Directional
    Vec3 color{1,1,1};
    float intensity = 1.0f;

    // Atenuação Point/Spot: 1 / (a + b*d + c*d²)
    float attA = 1.0f, attB = 0.0f, attC = 0.0f;

    // Spot: cossenos de cutoff (interno e externo)
    float cosInner = 0.95f, cosOuter = 0.85f;

    static Light point(const Vec3& p, const Vec3& c, float i = 1.0f) {
        Light l; l.type = LightType::Point; l.position = p; l.color = c;
        l.intensity = i; l.attA = 1.0f; l.attB = 0.009f; l.attC = 0.0001f;
        return l;
    }
    static Light spot(const Vec3& p, const Vec3& dir, const Vec3& c, float i = 1.0f) {
        Light l; l.type = LightType::Spot; l.position = p;
        l.direction = normalize(dir); l.color = c; l.intensity = i;
        l.attA = 1.0f; l.attB = 0.005f; l.attC = 0.0001f;
        l.cosInner = 0.92f; l.cosOuter = 0.78f;
        return l;
    }
    static Light directional(const Vec3& dir, const Vec3& c, float i = 1.0f) {
        Light l; l.type = LightType::Directional; l.direction = normalize(dir);
        l.color = c; l.intensity = i; return l;
    }
    static Light ambient(const Vec3& c) {
        Light l; l.type = LightType::Ambient; l.color = c; l.intensity = 1.0f;
        return l;
    }
};