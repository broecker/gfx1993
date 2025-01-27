#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "GridGeometry.h"
#include "PlyGeometry.h"
#include "Quad.h"
#include "Pipeline.h"
#include "Shader.h"

using namespace gfx1993;

class VisualizeDepthBufferFragShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    assert(renderDepth != nullptr);

    Fragment result;
    result.discard = false;

    float depth = renderDepth->getDepth(in.windowCoord.x, in.windowCoord.y);
    result.color = glm::vec4(glm::lerp(glm::vec3(0.0,1.0,0.0), 
                                       glm::vec3(1.0,0.0,0.0),
                                       glm::vec3(depth)), 1.0);
    return result;
  };

  std::shared_ptr<Depthbuffer> renderDepth;
};


// Creates a number of line vertices from a geometry containing triangles.
static IndexList extractLineIndices(const IndexList& indices) {
  IndexList result;
  if (indices.size() % 3 != 0) {
    std::cout << "Warning, geometry does not contain well-formed triangles.\n";
  }

  result.reserve(indices.size()*2);

  for (size_t i = 0; i < indices.size(); i += 3) {
    result.push_back(indices[i+0]);
    result.push_back(indices[i+1]);
    result.push_back(indices[i+1]);
    result.push_back(indices[i+2]);
    result.push_back(indices[i+2]);
    result.push_back(indices[i+0]);
  }
  return result;
}

class Demo08 : public DemoApp {
public:
  Demo08() : DemoApp("Demo 08 - Hidden Wireframe") {}

protected:
  void init() override {
    gridShader = std::make_shared<InputColorShader>();
    bunnyShader = std::make_shared<SingleColorShader>(glm::vec4(0.7, 0.0, 0.0, 1.0));
    depthBufferShader = std::make_shared<VisualizeDepthBufferFragShader>();

    renderConfig.vertexShader =
        std::make_shared<DefaultVertexTransform>();
    renderConfig.fragmentShader = std::make_shared<NormalColorShader>();

    assert(bunny.loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply"));
    bunny.transform = glm::scale(glm::vec3(125.f));
    bunny.center();

    dynamic_cast<OrbitCamera*>(camera.get())->setTarget(bunny.getCenter());

    // Filled-in by DemoApp. We'll save it so we can swap it out.
    renderTarget = renderConfig.framebuffer;
    depthBuffer = renderConfig.depthbuffer;
    visualizeBackbufferTarget = std::make_shared<Framebuffer>(renderTarget->getWidth(), renderTarget->getHeight());
  }

  void renderFrame() override {
    // reset the render matrices
    DefaultVertexTransform *dvt =
        dynamic_cast<DefaultVertexTransform *>(
            renderConfig.vertexShader.get());
    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();
    dvt->projectionMatrix = camera->getProjectionMatrix();

    renderConfig.framebuffer = renderTarget;
    renderConfig.depthbuffer = depthBuffer;

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // Draw all the bunny; first draw it as triangles, without a color buffer.
    // This will still fill the depth buffer.
    renderConfig.fragmentShader = bunnyShader;
    renderConfig.framebuffer = nullptr;
    dvt->modelMatrix = bunny.transform;
    rasterizer->drawTriangles(renderConfig, bunny.getVertices(),
                              bunny.getIndices());


    // Draw the bunny again, this time, we'll draw it as line strips only.
    renderConfig.framebuffer = renderTarget;
    rasterizer->drawLines(renderConfig, 
                          bunny.getVertices(), 
                          extractLineIndices(bunny.getIndices()));

    // Finally, draw the grid. We want to draw it last, so the bunny's
    // depth pass will block any grid lines behind the model.
    if (drawGrid) {
      renderConfig.framebuffer = renderTarget;
      renderConfig.fragmentShader = gridShader;
      dvt->modelMatrix = glm::mat4(1);
      rasterizer->drawLines(renderConfig, 
                            grid.getVertices(),
                            grid.getIndices());
    }

    // For debug purposes. We're also drawing to a different render target
    // to preserve what we have rendered in the framebuffer and depthbuffers.
    if (drawBackBuffer) {
      renderConfig.fragmentShader = depthBufferShader;
      renderConfig.framebuffer = visualizeBackbufferTarget;
      renderConfig.depthbuffer = nullptr;
      depthBufferShader->renderDepth = depthBuffer;
      rasterizer->drawScreenFillingQuad(renderConfig);
    }
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) override {
    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
      std::cout << "Drawing tri raster bounds: " << renderConfig.drawTriangleBounds << std::endl;
    }

    if (key == 'c') {
      renderConfig.cullBackFaces = !renderConfig.cullBackFaces;
      std::cout << "Culling backfaces: " << renderConfig.cullBackFaces << std::endl;
    }

    if (key == 'd') {
      drawBackBuffer = !drawBackBuffer;
    }

    if (key == 'g') {
      drawGrid = !drawGrid;
    }
  }

private:
  GridGeometry grid;
  PlyGeometry bunny;

  std::shared_ptr<FragmentShader> gridShader;
  std::shared_ptr<SingleColorShader> lineShader;
  std::shared_ptr<SingleColorShader> bunnyShader;
  std::shared_ptr<VisualizeDepthBufferFragShader> depthBufferShader;

  std::shared_ptr<Framebuffer> renderTarget;
  std::shared_ptr<Depthbuffer> depthBuffer;
  std::shared_ptr<Framebuffer> visualizeBackbufferTarget;

  bool drawBackBuffer = false;
  bool drawGrid = true;
};

int main(int argc, char **argv) {
  Demo08 demo;
  demo.run(argc, argv);

  return 0;
}
