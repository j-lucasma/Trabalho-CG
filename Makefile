CXX       := g++
CXXFLAGS  := -O2 -std=c++17 -Wall -Wextra -Wno-unused-parameter
INCLUDES  := -Isrc -I. -Ivendor/imgui -Ivendor/imgui/backends -Ivendor/glfw/include

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
    GLFW_LIB   := vendor/glfw/build/src/libglfw3.a
    GL_LIBS    := -lGL -ldl -lpthread
    GLFW_BUILD_CMD := cmake -S vendor/glfw -B vendor/glfw/build -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && cmake --build vendor/glfw/build -j
endif
ifeq ($(UNAME_S), Darwin)
    GLFW_LIB   := vendor/glfw/build/src/libglfw3.a
    GL_LIBS    := -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lpthread -ldl
    GLFW_BUILD_CMD := cmake -S vendor/glfw -B vendor/glfw/build -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && cmake --build vendor/glfw/build -j
endif

IMGUI_SRC := \
    vendor/imgui/imgui.cpp \
    vendor/imgui/imgui_draw.cpp \
    vendor/imgui/imgui_tables.cpp \
    vendor/imgui/imgui_widgets.cpp \
    vendor/imgui/backends/imgui_impl_glfw.cpp \
    vendor/imgui/backends/imgui_impl_opengl3.cpp

SRC := src/main.cpp $(IMGUI_SRC)

OUT := raytracer_gui

all: $(OUT)

$(GLFW_LIB):
	@echo "[build] compilando GLFW..."
	$(GLFW_BUILD_CMD)

$(OUT): $(GLFW_LIB) $(SRC) src/*.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) $(GLFW_LIB) -o $(OUT) $(GL_LIBS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT) out_*.png
	rm -rf vendor/glfw/build

.PHONY: all run clean