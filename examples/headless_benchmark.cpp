#include "Rasterizer.h"
#include "RenderConfig.h"
#include "Geometry.h"
#include "Shader.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"
#include <iostream>
#include <memory>
#include <glm/ext.hpp>

using namespace gfx1993;

int main() {
    const int width = 800;
    const int height = 600;

    auto fb = std::make_shared<Framebuffer>(width, height);
    auto db = std::make_shared<Depthbuffer>(width, height);
    auto vp = std::make_shared<Viewport>(glm::ivec2(0, 0), glm::ivec2(width, height));

    RenderConfig config;
    config.framebuffer = fb;
    config.depthbuffer = db;
    config.viewport = vp;
    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));

    auto* dvt = static_cast<DefaultVertexTransform*>(config.vertexShader.get());
    dvt->projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width/height, 0.1f, 100.0f);
    dvt->viewMatrix = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
    dvt->modelMatrix = glm::mat4(1.0f);

    Sphere sphere(1.0f, 60, 60); // High-poly sphere for testing
    Rasterizer rasterizer;

    std::cout << "Starting Headless Benchmark (100 iterations)..." << std::endl;
    for(int i = 0; i < 100; ++i) {
        rasterizer.drawTriangles(config, sphere.getVertices(), sphere.getIndices());
    }

    rasterizer.getProfile().print();
    return 0;
}