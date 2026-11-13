Este repositório contém a implementação de um **Ray Tracer interativo em C++** desenvolvido do zero. O projeto atende a todos os requisitos obrigatórios e bônus da disciplina, incluindo interseção com primitivas geométricas (esfera, cilindro, cone, malha), transformações (translação, rotação arbitrária via quatérnios, escala, cisalhamento e espelho), iluminação (point, spot, directional e ambient), sombras, projeções (perspectiva com 1/2/3 pontos de fuga, ortográfica, oblíqua), picking, texturas e interface gráfica para controle total.

## Pré-requisitos e Instalação (Linux)
O projeto foi testado no Ubuntu/Debian. Para compilar, instale o compilador, CMake e as bibliotecas do X11/OpenGL:
```bash
sudo apt update
sudo apt install -y build-essential cmake git xorg-dev libgl1-mesa-dev

# 1. Baixar Dependências
mkdir -p vendor && cd vendor
git clone --depth 1 --branch 3.4 https://github.com/glfw/glfw.git glfw
git clone --depth 1 https://github.com/ocornut/imgui.git imgui
cd ..
curl -LO https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -LO https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# 2. Baixar Assets (Opcional)
mkdir -p assets && cd assets
curl -L -o wood.jpg "https://dl.polyhaven.org/file/ph-assets/Textures/jpg/1k/dark_paneled_wood/dark_paneled_wood_diff_1k.jpg"
cd ..

# 3. Compilar e Executar
make
./raytracer_gui