#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <vector>
#include <cstdio>

struct Texture {
    int w = 0, h = 0;
    std::vector<unsigned char> data; // RGB

    bool load(const std::string& path) {
        int n = 0;
        unsigned char* raw = stbi_load(path.c_str(), &w, &h, &n, 3);
        if (!raw) {
            std::fprintf(stderr, "[texture] Falha ao carregar: %s\n", path.c_str());
            return false;
        }
        data.assign(raw, raw + w*h*3);
        stbi_image_free(raw);
        return true;
    }

    // Amostragem bilinear — UVs em [0,1]
    Vec3 sample(float u, float v) const {
        if (data.empty()) return Vec3(0.7f);
        u = u - std::floor(u);
        v = 1.0f - (v - std::floor(v)); // flip V

        float fx = u * (w - 1), fy = v * (h - 1);
        int x0 = (int)fx, y0 = (int)fy;
        int x1 = std::min(x0+1, w-1), y1 = std::min(y0+1, h-1);
        float tx = fx - x0, ty = fy - y0;

        auto get = [&](int x, int y) {
            size_t i = (size_t)(y*w + x) * 3;
            return Vec3(data[i]/255.0f, data[i+1]/255.0f, data[i+2]/255.0f);
        };
        Vec3 a = lerp(get(x0,y0), get(x1,y0), tx);
        Vec3 b = lerp(get(x0,y1), get(x1,y1), tx);
        return lerp(a, b, ty);
    }
};

struct TextureManager {
    std::vector<Texture> textures;
    int load(const std::string& p) {
        Texture t;
        if (!t.load(p)) return -1;
        textures.push_back(std::move(t));
        return (int)textures.size() - 1;
    }
    const Texture* get(int id) const {
        if (id < 0 || id >= (int)textures.size()) return nullptr;
        return &textures[id];
    }
};