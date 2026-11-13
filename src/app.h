#pragma once
#include "scene.h"
#include "camera.h"
#include "renderer.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <cstdint>
#include <chrono>
#include <cstdio>

// Estado global compartilhado entre GUI e renderização.
struct AppState {
    Scene scene;
    Camera camera;

    // Framebuffer do ray tracer
    std::vector<uint8_t> pixels;
    GLuint textureId = 0;

    // Controle de renderização
    bool  needsRedraw   = true;
    bool  autoRedraw    = true;   // re-renderiza ao mexer em algo
    int   aaSamples     = 1;      // 1 = rapido
    int   selectedId    = -1;     // objeto selecionado via pick
    bool  showHighlight = false;
    bool  quickPreview  = false;   // render rapido enquanto arrasta

    // Status
    double lastRenderMs = 0.0;
    double lastPickMs   = 0.0;
    int    pickX = 0, pickY = 0;

    // Preview reduzido para interação fluida (renderiza em 400x400 e
    // exibe ampliado enquanto o usuário arrasta; ao soltar, renderiza full).
    int previewRes = 400;

    // Céu/Background
    Vec3 skyTop    {0.55f, 0.7f, 0.95f};
    Vec3 skyBottom {0.95f, 0.95f, 0.98f};

    AppState() { pixels.assign(camera.W * camera.H * 3, 255); }

    void uploadTexture() {
        if (textureId == 0) {
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, camera.W, camera.H, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    }

    void doRender(int aa) {
        auto t0 = std::chrono::high_resolution_clock::now();

        int renderW = camera.W, renderH = camera.H;
        if (quickPreview) {
            renderW = camera.W / 2;
            renderH = camera.H / 2;
            aa = 1;
        }

        Renderer R(scene, camera);
        R.aaSamples = aa;

        std::vector<uint8_t> newPixels = R.render(renderW, renderH);

        if (showHighlight && selectedId >= 0) {
            std::vector<int> idBuffer(renderW * renderH, -1);
            for (int j = 0; j < renderH; ++j)
                for (int i = 0; i < renderW; ++i)
                    idBuffer[j*renderW + i] = R.pick(i, j);
            const uint8_t hi[3] = {255, 230, 40};
            for (int j = 1; j < renderH-1; ++j)
                for (int i = 1; i < renderW-1; ++i) {
                    int id = idBuffer[j*renderW + i];
                    if (id == selectedId) {
                        bool edge = false;
                        for (int dj = -1; dj <= 1 && !edge; ++dj)
                          for (int di = -1; di <= 1 && !edge; ++di)
                            if (idBuffer[(j+dj)*renderW + (i+di)] != selectedId) edge = true;
                        if (edge) {
                            size_t idx = (size_t)(j*renderW + i) * 3;
                            newPixels[idx+0] = hi[0];
                            newPixels[idx+1] = hi[1];
                            newPixels[idx+2] = hi[2];
                        }
                    }
                }
        }

        if (quickPreview) {
            std::vector<uint8_t> up(camera.W * camera.H * 3);
            for (int j = 0; j < camera.H; ++j)
                for (int i = 0; i < camera.W; ++i) {
                    int sj = std::min(renderH-1, j * renderH / camera.H);
                    int si = std::min(renderW-1, i * renderW / camera.W);
                    size_t src = (size_t)(sj*renderW + si) * 3;
                    size_t dst = (size_t)(j*camera.W + i) * 3;
                    up[dst+0] = newPixels[src+0];
                    up[dst+1] = newPixels[src+1];
                    up[dst+2] = newPixels[src+2];
                }
            newPixels = std::move(up);
        }

        pixels = std::move(newPixels);
        uploadTexture();

        auto t1 = std::chrono::high_resolution_clock::now();
        lastRenderMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        needsRedraw = false;
    }
};