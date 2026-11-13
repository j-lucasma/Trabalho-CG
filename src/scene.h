#pragma once
#include "primitives.h"
#include "light.h"
#include "material.h"
#include "texture.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

struct Scene {
    std::vector<Primitive> prims;
    std::vector<Material>  materials;
    std::vector<Light>     lights;
    TextureManager         textures;

    int addMaterial(const Material& m) {
        materials.push_back(m);
        return (int)materials.size() - 1;
    }

    int addPrimitive(Primitive p) {
        p.bake();
        prims.push_back(std::move(p));
        return (int)prims.size() - 1;
    }

    // Loader de OBJ (malha) — requisito 1.3.1
    bool loadOBJ(const std::string& path,
                 std::vector<Triangle>& out,
                 float scaleToUnit = 1.0f) {
        std::ifstream in(path);
        if (!in) { std::fprintf(stderr, "[obj] Nao abriu %s\n", path.c_str()); return false; }
        std::vector<Vec3> V, N;
        std::vector<std::pair<int,int>> faceUV;
        std::vector<Vec3> UVs;

        std::string line;
        while (std::getline(in, line)) {
            std::istringstream ss(line);
            std::string tag; ss >> tag;
            if (tag == "v") {
                float x,y,z; ss >> x >> y >> z; V.push_back({x,y,z});
            } else if (tag == "vn") {
                float x,y,z; ss >> x >> y >> z; N.push_back({x,y,z});
            } else if (tag == "vt") {
                float u,v; ss >> u >> v; UVs.push_back({u,v,0});
            } else if (tag == "f") {
                std::vector<int> vi, ni, ti;
                std::string tok;
                while (ss >> tok) {
                    int v=0,t=0,n=0;
                    size_t p1 = tok.find('/');
                    v = std::stoi(tok.substr(0, p1)) - 1;
                    if (p1 != std::string::npos) {
                        size_t p2 = tok.find('/', p1+1);
                        if (p2 != p1+1 && p2 != std::string::npos) {
                            t = std::stoi(tok.substr(p1+1, p2-p1-1)) - 1;
                        } else if (p2 == std::string::npos && p1+1 < tok.size()) {
                            t = std::stoi(tok.substr(p1+1)) - 1;
                        }
                        if (p2 != std::string::npos && p2+1 < tok.size()) {
                            n = std::stoi(tok.substr(p2+1)) - 1;
                        }
                    }
                    vi.push_back(v); ti.push_back(t); ni.push_back(n);
                }
                // fan triangulation
                for (size_t k = 1; k + 1 < vi.size(); ++k) {
                    Triangle tri;
                    int idx[3] = { vi[0], vi[k], vi[k+1] };
                    int tid[3] = { ti[0], ti[k], ti[k+1] };
                    int nid[3] = { ni[0], ni[k], ni[k+1] };
                    Vec3 pts[3];
                    for (int j = 0; j < 3; ++j) pts[j] = V[idx[j]];
                    // UVs
                    for (int j = 0; j < 3; ++j) {
                        float uu = 0, vv = 0;
                        if (tid[j] >= 0 && tid[j] < (int)UVs.size()) { uu = UVs[tid[j]].x; vv = UVs[tid[j]].y; }
                        if (j==0) { tri.u0=uu; tri.v0u=vv; }
                        if (j==1) { tri.u1=uu; tri.v1u=vv; }
                        if (j==2) { tri.u2=uu; tri.v2u=vv; }
                    }
                    // normais
                    Vec3 fn = normalize(cross(pts[1]-pts[0], pts[2]-pts[0]));
                    for (int j = 0; j < 3; ++j) {
                        Vec3 nn = fn;
                        if (nid[j] >= 0 && nid[j] < (int)N.size()) nn = normalize(N[nid[j]]);
                        if (j==0) tri.n0 = nn;
                        if (j==1) tri.n1 = nn;
                        if (j==2) tri.n2 = nn;
                    }
                    tri.v0 = pts[0]; tri.v1 = pts[1]; tri.v2 = pts[2];
                    out.push_back(tri);
                }
            }
        }
        // Normaliza para escalaToUnit
        if (!out.empty() && scaleToUnit > 0) {
            Vec3 mn(FLT_MAX), mx(-FLT_MAX);
            for (auto& t : out) for (auto& p : {t.v0, t.v1, t.v2}) {
                mn.x=std::min(mn.x,p.x); mn.y=std::min(mn.y,p.y); mn.z=std::min(mn.z,p.z);
                mx.x=std::max(mx.x,p.x); mx.y=std::max(mx.y,p.y); mx.z=std::max(mx.z,p.z);
            }
            Vec3 c = (mn + mx) * 0.5f;
            Vec3 sz = mx - mn;
            float s = scaleToUnit / std::max({sz.x, sz.y, sz.z, 1e-6f});
            for (auto& t : out) {
                t.v0 = (t.v0 - c) * s;
                t.v1 = (t.v1 - c) * s;
                t.v2 = (t.v2 - c) * s;
            }
        }
        return true;
    }
};