#pragma once
#include "imgui.h"
#include "app.h"
#include "quaternion.h"
#include <cstring>
#include <string>
#include <cstdlib>

// ---------- Helpers ----------
static bool DragVec3(const char* label, Vec3& v, float speed = 0.01f) {
    float arr[3] = { v.x, v.y, v.z };
    bool ch = ImGui::DragFloat3(label, arr, speed);
    if (ch) { v.x = arr[0]; v.y = arr[1]; v.z = arr[2]; }
    return ch;
}
static bool DragColor(const char* label, Vec3& c) {
    float arr[3] = { c.x, c.y, c.z };
    bool ch = ImGui::ColorEdit3(label, arr);
    if (ch) { c.x = arr[0]; c.y = arr[1]; c.z = arr[2]; }
    return ch;
}

// ---------- Presets de câmera ----------
static void applyPreset1Ponto(AppState& app) {
    app.camera.eye = Vec3(2.0f, 2.0f, 6.0f);
    app.camera.at  = Vec3(2.0f, 2.0f, 2.0f);
    app.camera.up  = Vec3(0, 1, 0);
    app.camera.d = 1.5f;
    app.camera.xmin=-0.6f; app.camera.xmax=0.6f;
    app.camera.ymin=-0.6f; app.camera.ymax=0.6f;
    app.camera.mode = ProjectionMode::Perspective;
    app.needsRedraw = true;
}
static void applyPreset2Pontos(AppState& app) {
    app.camera.eye = Vec3(5.5f, 2.0f, 5.5f);
    app.camera.at  = Vec3(2.0f, 2.0f, 2.0f);
    app.camera.up  = Vec3(0, 1, 0);
    app.camera.d = 1.2f;
    app.camera.xmin=-0.6f; app.camera.xmax=0.6f;
    app.camera.ymin=-0.6f; app.camera.ymax=0.6f;
    app.camera.mode = ProjectionMode::Perspective;
    app.needsRedraw = true;
}
static void applyPreset3Pontos(AppState& app) {
    app.camera.eye = Vec3(5.5f, 3.2f, 5.5f);
    app.camera.at  = Vec3(2.0f, 1.0f, 2.0f);
    app.camera.up  = Vec3(0, 1, 0);
    app.camera.d = 1.2f;
    app.camera.xmin=-0.6f; app.camera.xmax=0.6f;
    app.camera.ymin=-0.6f; app.camera.ymax=0.6f;
    app.camera.mode = ProjectionMode::Perspective;
    app.needsRedraw = true;
}
static void applyPresetPadrao(AppState& app) {
    app.camera.eye = Vec3(1.2f, 1.6f, 4.2f);
    app.camera.at  = Vec3(2.0f, 1.2f, 2.0f);
    app.camera.up  = Vec3(0, 1, 0);
    app.camera.d = 1.0f;
    app.camera.xmin=-0.5f; app.camera.xmax=0.5f;
    app.camera.ymin=-0.5f; app.camera.ymax=0.5f;
    app.camera.mode = ProjectionMode::Perspective;
    app.needsRedraw = true;
}

// ---------- Painel principal (chamado a cada frame) ----------
inline void drawGUI(AppState& app) {
    // -------- Painel 1: Câmera --------
    ImGui::Begin("Camera (req 2.1, 2.2, 3.1)");

    if (ImGui::CollapsingHeader("Parametros de visao", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (DragVec3("Eye",   app.camera.eye, 0.05f)) app.needsRedraw = true;
        if (ImGui::IsItemActive()) app.quickPreview = true;
        if (DragVec3("At",    app.camera.at,  0.05f)) app.needsRedraw = true;
        if (ImGui::IsItemActive()) app.quickPreview = true;
        if (DragVec3("Up",    app.camera.up,  0.05f)) app.needsRedraw = true;
        if (ImGui::IsItemActive()) app.quickPreview = true;
    }
    if (ImGui::CollapsingHeader("Parametros adicionais", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::DragFloat("d (dist. focal)", &app.camera.d, 0.01f, 0.05f, 10.0f))
            app.needsRedraw = true;

        float w = app.camera.xmax - app.camera.xmin;
        float h = app.camera.ymax - app.camera.ymin;
        if (ImGui::DragFloat("largura janela", &w, 0.005f, 0.05f, 5.0f)) {
            float cx = 0.5f*(app.camera.xmin + app.camera.xmax);
            app.camera.xmin = cx - w*0.5f;
            app.camera.xmax = cx + w*0.5f;
            app.needsRedraw = true;
        }
        if (ImGui::DragFloat("altura janela", &h, 0.005f, 0.05f, 5.0f)) {
            float cy = 0.5f*(app.camera.ymin + app.camera.ymax);
            app.camera.ymin = cy - h*0.5f;
            app.camera.ymax = cy + h*0.5f;
            app.needsRedraw = true;
        }
        ImGui::Text("janela = [%.2f,%.2f] x [%.2f,%.2f]",
            app.camera.xmin, app.camera.xmax, app.camera.ymin, app.camera.ymax);
    }

    if (ImGui::CollapsingHeader("Zoom (req 3.1.1)", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Zoom In"))  { app.camera.zoomIn(0.8f);  app.needsRedraw = true; }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) { app.camera.zoomOut(1.25f); app.needsRedraw = true; }
    }

    if (ImGui::CollapsingHeader("Modo de projecao (req 3.1/3.2/3.3)",
                                 ImGuiTreeNodeFlags_DefaultOpen)) {
        int mode = (int)app.camera.mode;
        if (ImGui::RadioButton("Perspectiva",  &mode, 0)) { app.camera.mode = (ProjectionMode)mode; app.needsRedraw = true; }
        if (ImGui::RadioButton("Ortografica",  &mode, 1)) { app.camera.mode = (ProjectionMode)mode; app.needsRedraw = true; }
        if (ImGui::RadioButton("Obliqua",      &mode, 2)) { app.camera.mode = (ProjectionMode)mode; app.needsRedraw = true; }

        if (app.camera.mode == ProjectionMode::Oblique) {
            float ang = app.camera.obliqAngle * 180.0f / (float)M_PI;
            if (ImGui::SliderFloat("angulo obliqua (graus)", &ang, 0.0f, 90.0f)) {
                app.camera.obliqAngle = ang * (float)M_PI / 180.0f;
                app.needsRedraw = true;
            }
            if (ImGui::SliderFloat("fator k (obliqua)", &app.camera.obliqK, 0.0f, 1.5f))
                app.needsRedraw = true;
        }
    }

    if (ImGui::CollapsingHeader("Presets de pontos de fuga (req 3.1.2)", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("1 ponto de fuga"))  applyPreset1Ponto(app);
        ImGui::SameLine();
        if (ImGui::Button("2 pontos de fuga")) applyPreset2Pontos(app);
        ImGui::SameLine();
        if (ImGui::Button("3 pontos de fuga")) applyPreset3Pontos(app);
        if (ImGui::Button("Camera padrao"))    applyPresetPadrao(app);
    }

    ImGui::End();

    // -------- Painel 2: Render --------
    ImGui::Begin("Render (req 6, 7)");
    ImGui::Text("Resolucao: %d x %d", app.camera.W, app.camera.H);
    int res = app.camera.W;
    if (ImGui::SliderInt("Resolucao", &res, 500, 1600, "%d px")) {
        app.camera.W = app.camera.H = res;
        app.pixels.assign(res*res*3, 255);
        app.needsRedraw = true;
    }
    if (ImGui::IsItemActive()) app.quickPreview = true;
    if (ImGui::SliderInt("AA (amostras/axis)", &app.aaSamples, 1, 4))
        app.needsRedraw = true;
    if (ImGui::IsItemActive()) app.quickPreview = true;

    ImGui::Checkbox("Auto redraw", &app.autoRedraw);
    ImGui::SameLine();
    if (ImGui::Button("Redesenhar agora")) app.needsRedraw = true;

    ImGui::Text("Ultimo render: %.1f ms", app.lastRenderMs);
    ImGui::End();

    // -------- Painel 3: Luzes --------
    ImGui::Begin("Luzes (req 1.5)");
    for (int i = 0; i < (int)app.scene.lights.size(); ++i) {
        auto& L = app.scene.lights[i];
        ImGui::PushID(i);
        const char* tname = "?";
        switch (L.type) {
            case LightType::Point:       tname = "Point";       break;
            case LightType::Spot:        tname = "Spot";        break;
            case LightType::Directional: tname = "Directional"; break;
            case LightType::Ambient:     tname = "Ambient";     break;
        }
        if (ImGui::TreeNode(tname)) {
            DragColor("cor", L.color);
            if (L.type != LightType::Ambient) {
                ImGui::DragFloat("intensidade", &L.intensity, 0.01f, 0.0f, 5.0f);
                if (L.type != LightType::Directional) {
                    DragVec3("posicao", L.position, 0.02f);
                    ImGui::DragFloat("att a", &L.attA, 0.01f, 0.0f, 2.0f);
                    ImGui::DragFloat("att b", &L.attB, 0.001f, 0.0f, 0.5f);
                    ImGui::DragFloat("att c", &L.attC, 0.0001f, 0.0f, 0.01f);
                }
                if (L.type == LightType::Spot || L.type == LightType::Directional) {
                    DragVec3("direcao", L.direction, 0.02f);
                }
                if (L.type == LightType::Spot) {
                    ImGui::SliderFloat("cosInner", &L.cosInner, 0.0f, 1.0f);
                    ImGui::SliderFloat("cosOuter", &L.cosOuter, 0.0f, 1.0f);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();

    // -------- Painel 4: Objetos + Pick + Materiais --------
    ImGui::Begin("Objetos (req 1.3, 1.4, 5.1)");
    ImGui::Text("Clique na imagem para selecionar (pick)");
    if (app.selectedId >= 0) {
        ImGui::TextColored(ImVec4(1,1,0,1), "Selecionado: primitiva %d", app.selectedId);
        ImGui::SameLine();
        if (ImGui::SmallButton("limpar")) app.selectedId = -1;
    } else {
        ImGui::TextDisabled("Nada selecionado");
    }
    ImGui::Checkbox("Destacar selecionado", &app.showHighlight);
    ImGui::Text("Ultimo pick: (%d,%d) em %.2f ms", app.pickX, app.pickY, app.lastPickMs);

    ImGui::Separator();
    for (int i = 0; i < (int)app.scene.prims.size(); ++i) {
        auto& P = app.scene.prims[i];
        ImGui::PushID(1000 + i);
        const char* pname = "?";
        switch (P.type) {
            case PrimType::Sphere:   pname = "Esfera";   break;
            case PrimType::Cylinder: pname = "Cilindro"; break;
            case PrimType::Cone:     pname = "Cone";     break;
            case PrimType::Mesh:     pname = "Malha";    break;
            case PrimType::Plane:    pname = "Plano";    break;
        }
        bool sel = (app.selectedId == i);
        char label[128];
        std::snprintf(label, sizeof(label), "[%d] %s", i, pname);
        if (ImGui::Selectable(label, sel))
            app.selectedId = i;
        ImGui::PopID();
    }

    if (app.selectedId >= 0 && app.selectedId < (int)app.scene.prims.size()) {
        auto& P = app.scene.prims[app.selectedId];
        ImGui::Separator();
        
        // Controles de Posicao Absoluta (Gizmo de translação manual)
        static Vec3 pos(2.0f, 0.0f, 2.0f); 
        if (ImGui::CollapsingHeader("Posicao Absoluta (Mover Objeto)", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (DragVec3("Posicao XYZ", pos, 0.05f)) {
                // Reconstroi a matriz aplicando a nova posicao antes de tudo
                // Assumimos que a posicao atual e o 'P.center' para primitivas soltas
                P.center = pos;
                if (P.type == PrimType::Sphere || P.type == PrimType::Cylinder || P.type == PrimType::Cone) {
                    P.transform = Mat4::translation(pos);
                    P.center = Vec3(0,0,0);
                } else {
                    // Para malhas, a translação é o m[12], m[13], m[14]
                    P.transform.m[12] = pos.x;
                    P.transform.m[13] = pos.y;
                    P.transform.m[14] = pos.z;
                }
                P.bake();
                app.needsRedraw = true;
            }
        }
        
        ImGui::Separator();
        ImGui::Text("Material da primitiva:");
        for (int m = 0; m < (int)app.scene.materials.size(); ++m) {
            const auto& M = app.scene.materials[m];
            ImGui::PushID(2000 + m);
            char mlbl[128];
            std::snprintf(mlbl, sizeof(mlbl), "mat %d  kd=(%.2f,%.2f,%.2f) refl=%.2f tex=%d",
                m, M.kd.x, M.kd.y, M.kd.z, M.reflectivity, M.textureId);
            if (ImGui::RadioButton(mlbl, P.materialId == m)) {
                P.materialId = m;
                app.needsRedraw = true;
            }
            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::Text("Transformacoes (req 1.4)");
        // Botões para empilhar transformações sobre a primitiva selecionada
        static float tx=0.1f, ty=0.0f, tz=0.0f;
        static float ang=15.0f;
        static Vec3  axis {0.0f, 1.0f, 0.0f};
        static float sx=1.05f, sy=1.05f, sz=1.05f;
        static float shearK = 0.1f;
        static Vec3  mirN {0.0f, 1.0f, 1.0f};
        static float mirD = -1.0f;

        if (ImGui::TreeNode("Transladar")) {
            ImGui::DragFloat("x##t", &tx, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("y##t", &ty, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("z##t", &tz, 0.01f, -1.0f, 1.0f);
            if (ImGui::Button("aplicar translacao")) {
                P.transform = Mat4::translation(Vec3(tx,ty,tz)) * P.transform;
                P.bake(); app.needsRedraw = true;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Rotacionar")) {
            ImGui::DragFloat("ang (graus)", &ang, 0.5f, -180.0f, 180.0f);
            DragVec3("eixo", axis, 0.01f);
            if (ImGui::Button("aplicar rotacao arbitraria (quaternion)")) {
                P.transform = rotateArbitrary(axis, ang * (float)M_PI/180.0f) * P.transform;
                P.bake(); app.needsRedraw = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("rot X")) { P.transform = Mat4::rotateX(ang*(float)M_PI/180.0f)*P.transform; P.bake(); app.needsRedraw=true; }
            ImGui::SameLine();
            if (ImGui::Button("rot Y")) { P.transform = Mat4::rotateY(ang*(float)M_PI/180.0f)*P.transform; P.bake(); app.needsRedraw=true; }
            ImGui::SameLine();
            if (ImGui::Button("rot Z")) { P.transform = Mat4::rotateZ(ang*(float)M_PI/180.0f)*P.transform; P.bake(); app.needsRedraw=true; }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Escalar")) {
            ImGui::DragFloat("sx", &sx, 0.01f, 0.05f, 3.0f);
            ImGui::DragFloat("sy", &sy, 0.01f, 0.05f, 3.0f);
            ImGui::DragFloat("sz", &sz, 0.01f, 0.05f, 3.0f);
            if (ImGui::Button("aplicar escala")) {
                P.transform = Mat4::scale(Vec3(sx,sy,sz)) * P.transform;
                P.bake(); app.needsRedraw = true;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Cisalhamento (+0.5)")) {
            ImGui::DragFloat("k (x+=k*y)", &shearK, 0.01f, -1.0f, 1.0f);
            if (ImGui::Button("aplicar cisalhamento")) {
                P.transform = Mat4::shear(shearK, 0, 0, 0, 0, 0) * P.transform;
                P.bake(); app.needsRedraw = true;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Espelho em plano (+0.5)")) {
            DragVec3("normal do plano", mirN, 0.01f);
            ImGui::DragFloat("d (n.p + d = 0)", &mirD, 0.05f, -10.0f, 10.0f);
            if (ImGui::Button("aplicar espelho")) {
                P.transform = Mat4::mirrorPlane(mirN, mirD) * P.transform;
                P.bake(); app.needsRedraw = true;
            }
            ImGui::TreePop();
        }
        if (ImGui::Button("resetar transformacao (identidade)")) {
            P.transform = Mat4();
            P.bake(); app.needsRedraw = true;
        }
    }
    ImGui::End();

    // -------- Painel 5: Sobre / Debug --------
    ImGui::Begin("Sobre / Debug");
    ImGui::Text("Computacao Grafica — Trabalho Final");
    ImGui::Separator();
    ImGui::Text("Primitivas: %zu", app.scene.prims.size());
    ImGui::Text("Materiais:  %zu", app.scene.materials.size());
    ImGui::Text("Luzes:      %zu", app.scene.lights.size());
    ImGui::Text("Texturas:   %zu", app.scene.textures.textures.size());
    ImGui::Separator();
    ImGui::Text("Controles:");
    ImGui::BulletText("Clique na imagem para pick");
    ImGui::BulletText("Arraste com botao direito para orbitar a camera");
    ImGui::BulletText("Scroll do mouse: zoom in/out");
    ImGui::End();
}