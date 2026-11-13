#include "app.h"
#include "gui.h"
#include "scene.h"
#include "renderer.h"
#include "quaternion.h"

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

// ---------- Cenário temático (Setup de Computador) ----------
static void montarCena(Scene& scene) {
    int woodTex = scene.textures.load("assets/wood.jpg");

    // Materiais
    int matWood    = scene.addMaterial(Material::woodTextured(woodTex));
    int matMetal   = scene.addMaterial(Material::metal());
    
    // Material do Piso (Cinza Escuro)
    Material floorMat;
    floorMat.ka = Vec3(0.05f, 0.05f, 0.05f);
    floorMat.kd = Vec3(0.2f, 0.2f, 0.2f);
    floorMat.ks = Vec3(0.1f, 0.1f, 0.1f);
    int matFloor = scene.addMaterial(floorMat);

    // Textura procedural de teclado
    Texture kbTex;
    kbTex.w = 512; kbTex.h = 256;
    kbTex.data.resize(kbTex.w * kbTex.h * 3, 0);
    for (int y = 0; y < kbTex.h; ++y) {
        for (int x = 0; x < kbTex.w; ++x) {
            int idx = (y * kbTex.w + x) * 3;
            unsigned char r = 40, g = 40, b = 45; // Fundo
            int tx = x % 32;
            int ty = y % 32;
            if (tx > 2 && tx < 30 && ty > 2 && ty < 30) {
                r = 180; g = 180; b = 190; // Tecla
            }
            kbTex.data[idx] = r;
            kbTex.data[idx+1] = g;
            kbTex.data[idx+2] = b;
        }
    }
    scene.textures.textures.push_back(kbTex);
    int kbTexId = (int)scene.textures.textures.size() - 1;

    Material kbMat;
    kbMat.ka = Vec3(0.1f, 0.1f, 0.1f);
    kbMat.kd = Vec3(0.8f, 0.8f, 0.8f);
    kbMat.ks = Vec3(0.3f, 0.3f, 0.3f);
    kbMat.textureId = kbTexId;
    int matKeyboard = scene.addMaterial(kbMat);

    // Material do Livro (Couro)
    Material leatherMat;
    leatherMat.ka = Vec3(0.05f, 0.02f, 0.01f);
    leatherMat.kd = Vec3(0.35f, 0.15f, 0.05f);
    leatherMat.ks = Vec3(0.1f, 0.1f, 0.1f);
    leatherMat.shininess = 8.0f;
    leatherMat.textureId = woodTex;
    int matLeather = scene.addMaterial(leatherMat);

    // Material Paginas (Papel)
    Material paperMat;
    paperMat.ka = Vec3(0.2f, 0.2f, 0.15f);
    paperMat.kd = Vec3(0.9f, 0.85f, 0.7f);
    int matPaper = scene.addMaterial(paperMat);

    // Material Monitor Desligado
    Material monitorMat;
    monitorMat.ka = Vec3(0.02f, 0.02f, 0.02f);
    monitorMat.kd = Vec3(0.05f, 0.05f, 0.05f);
    monitorMat.ks = Vec3(1.0f, 1.0f, 1.0f);
    monitorMat.shininess = 512.0f;
    monitorMat.reflectivity = 0.85f;
    int matMonitor = scene.addMaterial(monitorMat);

    // Material Fruta (Maçã)
    Material appleMat;
    appleMat.ka = Vec3(0.1f, 0.0f, 0.0f);
    appleMat.kd = Vec3(0.8f, 0.1f, 0.1f);
    appleMat.ks = Vec3(0.9f, 0.9f, 0.9f);
    appleMat.shininess = 128.0f;
    int matApple = scene.addMaterial(appleMat);

    // Material Plástico (Garrafa)
    Material plasticMat;
    plasticMat.ka = Vec3(0.2f, 0.3f, 0.4f);
    plasticMat.kd = Vec3(0.4f, 0.6f, 0.8f); // Azul claro translúcido
    plasticMat.ks = Vec3(0.6f, 0.6f, 0.6f); // Brilho suave
    plasticMat.shininess = 32.0f;
    plasticMat.reflectivity = 0.1f; // Pouca reflexão
    int matPlastic = scene.addMaterial(plasticMat);

    // 1. Mesa (Cilindro)
    {
        Primitive p;
        p.type = PrimType::Cylinder;
        p.center = Vec3(0.0f, 0.0f, 0.0f);
        p.radius = 2.5f;
        p.height = 0.1f;
        p.materialId = matWood;
        p.transform = Mat4::translation(Vec3(2.0f, 0.0f, 2.0f));
        scene.addPrimitive(p);
    }

    // 2. Piso (Plano)
    {
        Primitive p;
        p.type = PrimType::Plane;
        p.planeNormal = Vec3(0, 1, 0);
        p.planeD = 0.0f;
        p.materialId = matFloor;
        scene.addPrimitive(p);
    }

    // 3. Caderno (Malha)
    {
        Primitive pCover;
        pCover.type = PrimType::Mesh;
        pCover.materialId = matLeather;
        std::vector<Triangle> tris;
        if (scene.loadOBJ("assets/book.obj", tris, 1.2f)) {
            pCover.triangles = tris;
        }
        Mat4 T = Mat4::translation(Vec3(0.9f, 0.15f, 3.0f));
        Mat4 R = Mat4::rotateY(20.0f*(float)M_PI/180.0f);
        Mat4 S_livro = Mat4::scale(Vec3(0.7f, 1.0f, 0.7f)); // Diminuir o livro
        pCover.transform = T * R * S_livro;
        scene.addPrimitive(pCover);

        Primitive pPages;
        pPages.type = PrimType::Mesh;
        pPages.materialId = matPaper;
        for (auto tri : pCover.triangles) {
            tri.v0 = tri.v0 * 0.96f; tri.v1 = tri.v1 * 0.96f; tri.v2 = tri.v2 * 0.96f;
            pPages.triangles.push_back(tri);
        }
        pPages.transform = T * R * S_livro;
        scene.addPrimitive(pPages);
    }

    // 4. Maçã e Cabinho
    {
        Primitive p;
        p.type = PrimType::Sphere;
        p.center = Vec3(0, 0, 0);
        p.radius = 0.2f;
        p.materialId = matApple;
        p.transform = Mat4::translation(Vec3(2.8f, 0.3f, 1.5f));
        scene.addPrimitive(p);

        Primitive pc;
        pc.type = PrimType::Cone;
        pc.center = Vec3(0, 0, 0);
        pc.radius = 0.02f;
        pc.height = 0.08f;
        pc.materialId = matLeather;
        pc.transform = Mat4::translation(Vec3(2.8f, 0.48f, 1.5f));
        scene.addPrimitive(pc);
    }

    // 5. Luminária de Mesa (Passo 1: Apenas Base e Haste)
    {
        // Posição original
        Vec3 basePos = Vec3(1.2f, 0.0f, 2.0f);

        // 1. Base (Cilindro achatado)
        Primitive pBase;
        pBase.type = PrimType::Cylinder;
        pBase.center = Vec3(0, 0, 0);
        pBase.radius = 0.15f;
        pBase.height = 0.04f;
        pBase.materialId = matMetal;
        // O topo da base fica em y = 0.14 (0.12 centro + 0.02 metade da altura)
        pBase.transform = Mat4::translation(basePos + Vec3(0, 0.12f, 0));
        scene.addPrimitive(pBase);

        // 2. Haste (Cilindro fino)
        Primitive pHaste;
        pHaste.type = PrimType::Cylinder;
        pHaste.center = Vec3(0, 0, 0);
        pHaste.radius = 0.025f; // Levemente mais fina
        pHaste.height = 0.8f;
        pHaste.materialId = matMetal;
        
       
        pHaste.transform = Mat4::translation(basePos + Vec3(0, 0.15f, 0));
        scene.addPrimitive(pHaste);

        // 3. Cúpula (Cone Invertido)
        Primitive pCone;
        pCone.type = PrimType::Cone;
        pCone.center = Vec3(0, 0, 0);
        pCone.radius = 0.25f;
        pCone.height = 0.3f;
        pCone.materialId = matPaper;
        

        Mat4 R = Mat4::rotateX((float)M_PI);
        pCone.transform = Mat4::translation(basePos + Vec3(0, 0.9f, 0)) * R;
        scene.addPrimitive(pCone);
    }

    // 5.5 Teclado (Otimizado)
    {
        Primitive pBase;
        pBase.type = PrimType::Mesh;
        pBase.materialId = matKeyboard; 
        auto addQuad = [&](Primitive& p, Vec3 a, Vec3 b, Vec3 c, Vec3 d){
            Vec3 n = normalize(cross(b-a, c-a));
            p.triangles.push_back({a,b,c, n,n,n, 0,0, 1,0, 1,1});
            p.triangles.push_back({a,c,d, n,n,n, 0,0, 1,1, 0,1});
        };
        Vec3 o(-0.4f, 0.0f, -0.15f);
        Vec3 v000=o, v100=o+Vec3(0.8f,0,0), v010=o+Vec3(0,0.04f,0), v110=o+Vec3(0.8f,0.04f,0);
        Vec3 v001=o+Vec3(0,0,0.3f), v101=o+Vec3(0.8f,0,0.3f), v011=o+Vec3(0,0.04f,0.3f), v111=o+Vec3(0.8f,0.04f,0.3f);
        addQuad(pBase, v000, v010, v110, v100); addQuad(pBase, v001, v101, v111, v011);
        addQuad(pBase, v000, v001, v011, v010); addQuad(pBase, v100, v110, v111, v101);
        addQuad(pBase, v000, v100, v101, v001); addQuad(pBase, v010, v011, v111, v110);
        Mat4 Tb = Mat4::translation(Vec3(2.2f, 0.1f, 2.4f));
        Mat4 Rb = Mat4::rotateY(10.0f*(float)M_PI/180.0f);
        pBase.transform = Tb * Rb;
        scene.addPrimitive(pBase);
    }

    // 6. Monitor Desligado
    {
        Primitive p;
        p.type = PrimType::Mesh;
        p.materialId = matMonitor;
        auto addQuad = [&](Primitive& p, Vec3 a, Vec3 b, Vec3 c, Vec3 d){
            Vec3 n = normalize(cross(b-a, c-a));
            p.triangles.push_back({a,b,c, n,n,n, 0,0, 1,0, 1,1});
            p.triangles.push_back({a,c,d, n,n,n, 0,0, 1,1, 0,1});
        };
        addQuad(p, Vec3(-0.7f, 0.0f, 0.0f), Vec3( 0.7f, 0.0f, 0.0f),
                   Vec3( 0.7f, 0.8f, 0.0f), Vec3(-0.7f, 0.8f, 0.0f));
        Mat4 T = Mat4::translation(Vec3(2.0f, 0.5f, 1.0f));
        p.transform = T;
        scene.addPrimitive(p);

        Primitive base;
        base.type = PrimType::Cylinder;
        base.center = Vec3(0,0,0);
        base.radius = 0.2f;
        base.height = 0.05f;
        base.materialId = matMetal;
        base.transform = Mat4::translation(Vec3(2.0f, 0.1f, 1.0f));
        scene.addPrimitive(base);

        Primitive neck;
        neck.type = PrimType::Cylinder;
        neck.center = Vec3(0,0,0);
        neck.radius = 0.05f;
        neck.height = 0.45f;
        neck.materialId = matMetal;
        neck.transform = Mat4::translation(Vec3(2.0f, 0.15f, 1.0f));
        scene.addPrimitive(neck);
    }

    // 7. Gabinete
    {
        Primitive p;
        p.type = PrimType::Mesh;
        p.materialId = matMetal;
        auto addQuad = [&](Primitive& p, Vec3 a, Vec3 b, Vec3 c, Vec3 d){
            Vec3 n = normalize(cross(b-a, c-a));
            p.triangles.push_back({a,b,c, n,n,n, 0,0, 1,0, 1,1});
            p.triangles.push_back({a,c,d, n,n,n, 0,0, 1,1, 0,1});
        };
        Vec3 o(-0.2f, 0.0f, -0.4f);
        Vec3 v000=o, v100=o+Vec3(0.4f,0,0), v010=o+Vec3(0,0.8f,0), v110=o+Vec3(0.4f,0.8f,0);
        Vec3 v001=o+Vec3(0,0,0.8f), v101=o+Vec3(0.4f,0,0.8f), v011=o+Vec3(0,0.8f,0.8f), v111=o+Vec3(0.4f,0.8f,0.8f);
        addQuad(p, v000, v010, v110, v100); addQuad(p, v001, v101, v111, v011);
        addQuad(p, v000, v001, v011, v010); addQuad(p, v100, v110, v111, v101);
        addQuad(p, v000, v100, v101, v001); addQuad(p, v010, v011, v111, v110);
        Mat4 T = Mat4::translation(Vec3(3.2f, 0.1f, 1.0f));
        p.transform = T;
        scene.addPrimitive(p);
    }

    // 8. Mouse
    {
        Primitive p;
        p.type = PrimType::Sphere;
        p.center = Vec3(0, 0, 0);
        p.radius = 0.1f;
        p.materialId = matMonitor;
        Mat4 T = Mat4::translation(Vec3(2.9f, 0.1f, 2.4f));
        Mat4 S = Mat4::scale(Vec3(1.0f, 0.4f, 1.5f)); 
        p.transform = T * S;
        scene.addPrimitive(p);
    }

    // Luzes
    scene.lights.push_back(Light::ambient(Vec3(0.2f, 0.2f, 0.2f)));
    scene.lights.push_back(Light::point(Vec3(2.0f, 3.0f, 3.0f), Vec3(1.0f,1.0f,0.9f), 1.2f));
    scene.lights.push_back(Light::directional(Vec3(-0.5f,-1.0f,-0.5f), Vec3(0.8f,0.9f,1.0f), 0.4f));
}

static void assertPrimeiroOctante(const Scene& s) {
    for (size_t i = 0; i < s.prims.size(); ++i) {
        Vec3 mn, mx; s.prims[i].computeAABB(mn, mx);
        if (mn.x < 0 || mn.y < 0 || mn.z < 0) {
            std::fprintf(stderr, "AVISO: primitiva %zu com AABB fora do 1o octante "
                "(min=%.2f,%.2f,%.2f)\n", i, mn.x, mn.y, mn.z);
        }
    }
}

struct ImageInteract {
    bool   dragging = false;
    double lastX = 0, lastY = 0;
};

int main(int, char**) {
    if (!glfwInit()) { std::fprintf(stderr, "Falha GLFW\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1400, 900, "Trabalho CG - Ray Tracer", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "Falha criar janela\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    AppState app;
    montarCena(app.scene);
    assertPrimeiroOctante(app.scene);

    // Camera configurada corretamente (isometrica de frente)
    app.camera.eye = Vec3(2.0f, 2.0f, 5.5f);
    app.camera.at  = Vec3(2.0f, 1.0f, 2.0f);
    app.camera.up  = Vec3(0,1,0);
    app.camera.d   = 1.0f;
    app.camera.xmin=-0.6f; app.camera.xmax=0.6f;
    app.camera.ymin=-0.6f; app.camera.ymax=0.6f;
    app.camera.W = 800; app.camera.H = 800;
    app.camera.mode = ProjectionMode::Perspective;

    app.doRender(app.aaSamples);

    ImageInteract img;
    bool draggingObject = false;
    ImVec2 lastDragPos;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (app.autoRedraw && app.needsRedraw) {
            app.doRender(app.aaSamples);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(320, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1050, 850), ImGuiCond_FirstUseEver);
        ImGui::Begin("Visualizador", nullptr, ImGuiWindowFlags_NoCollapse);

        ImVec2 avail = ImGui::GetContentRegionAvail();
        float aspect = (float)app.camera.W / (float)app.camera.H;
        float drawW = avail.x, drawH = drawW / aspect;
        if (drawH > avail.y) { drawH = avail.y; drawW = drawH * aspect; }

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(intptr_t)app.textureId, ImVec2(drawW, drawH), ImVec2(0,0), ImVec2(1,1));

        ImGui::SetCursorScreenPos(cursor);
        ImGui::InvisibleButton("img", ImVec2(drawW, drawH), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

        // Pick e Drag
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            ImVec2 mp = ImGui::GetIO().MousePos;
            float nx = (mp.x - cursor.x) / drawW;
            float ny = (mp.y - cursor.y) / drawH;
            int px = (int)(nx * app.camera.W);
            int py = (int)(ny * app.camera.H);
            if (px >= 0 && px < app.camera.W && py >= 0 && py < app.camera.H) {
                Renderer R(app.scene, app.camera);
                int id = R.pick(px, py);
                app.selectedId = id;
                if (id >= 0) {
                    draggingObject = true;
                    lastDragPos = mp;
                }
                app.needsRedraw = true;
            }
        }

        if (draggingObject && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && app.selectedId >= 0) {
            ImVec2 mp = ImGui::GetIO().MousePos;
            float dx = (mp.x - lastDragPos.x) / drawW;
            float dy = (mp.y - lastDragPos.y) / drawH;
            lastDragPos = mp;
            
            auto& P = app.scene.prims[app.selectedId];
            Vec3 camU, camV, camW;
            app.camera.basis(camU, camV, camW);
            float scale = (app.camera.xmax - app.camera.xmin) * app.camera.d; 
            Vec3 deltaWorld = camU * (dx * scale) - camV * (dy * scale);
            
            Mat4 T = Mat4::translation(deltaWorld);
            P.transform = T * P.transform;
            P.bake();
            app.needsRedraw = true;
        }
        
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            draggingObject = false;
        }

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            ImVec2 d = ImGui::GetIO().MouseDelta;
            Vec3 dir = app.camera.eye - app.camera.at;
            float r = dir.length();
            float yaw   = std::atan2(dir.z, dir.x);
            float pitch = std::asin (dir.y / r);
            yaw   -= d.x * 0.01f;
            pitch -= d.y * 0.01f;
            pitch = std::max(-1.5f, std::min(1.5f, pitch));
            dir = Vec3(r*std::cos(pitch)*std::cos(yaw), r*std::sin(pitch), r*std::cos(pitch)*std::sin(yaw));
            app.camera.eye = app.camera.at + dir;
            app.quickPreview = true;
            app.needsRedraw = true;
        }

        if (ImGui::IsItemHovered()) {
            float w = ImGui::GetIO().MouseWheel;
            if (w > 0) { app.camera.zoomIn(0.9f); app.quickPreview = true; app.needsRedraw = true; }
            if (w < 0) { app.camera.zoomOut(1.1f); app.quickPreview = true; app.needsRedraw = true; }
        }

        ImGui::End();

        drawGUI(app);

        if (!ImGui::IsAnyItemActive()) app.quickPreview = false;

        ImGui::Render();
        int dw, dh; glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
