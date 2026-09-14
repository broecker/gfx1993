#include "DemoApp.h"

#include "config.h"
#include "Profiler.h"
#include "RenderDebugInfo.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <glm/ext.hpp>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#if GFX1993_DEMO_USE_OPENMP
  #include <omp.h>
#endif

using namespace gfx1993;

DemoApp *DemoApp::appInstance = nullptr;
DemoApp::DemoApp(const std::string &name)
    : name(name), width(gfx1993::VGA_WIDTH), height(gfx1993::VGA_HEIGHT),
      showStatsOverlay(true), mousePosition(0,0) {
  rasterizer = std::make_unique<Rasterizer>();

  renderConfig.viewport =
      std::make_shared<Viewport>(0, 0, width, height);
  renderConfig.framebuffer =
      std::make_shared<Framebuffer>(width, height);
  renderConfig.depthbuffer =
      std::make_shared<Depthbuffer>(width, height);

  camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), 30.0f);

  srand(time(0));
  handleResize(width, height);
}

void DemoApp::run(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
	if (window == nullptr) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return;
	}

  // No PRESENTVSYNC: this app deliberately runs uncapped so the FPS
  // counter reflects the rasterizer's actual throughput.
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  }
  if (renderer == nullptr) {
    std::cerr << "Error creating SDL renderer: " << SDL_GetError() << std::endl;
    return;
  }
  createFrameTexture();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  // Demo app has no use for a persisted layout file.
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  // Ugly hack :-/
  appInstance = this;

  this->init();

  running = true;

  const int64_t startTicks = SDL_GetTicks64();
	int64_t lastTicks = startTicks;
  int64_t lastSecond = startTicks;

  while (running) {
    // Performance measurements.
		frames++;
		totalFrames++;
		const int64_t nowTicks = SDL_GetTicks64();

    const int64_t ticks = nowTicks - lastTicks;
    float dt = static_cast<float>(ticks) / 1000.f;
    lastTicks = nowTicks;
    frameTimeMs = dt * 1000.f;
    if (dt > 0.f) {
      peakFpsThisSecond = std::max(peakFpsThisSecond, 1.f / dt);
    }

    handleEvents();

    appInstance->updateFrame(dt);

    rasterizer->resetDebugInfo();
    appInstance->renderFrame();

    updateFrameTexture();
    GFX1993_FRAME_MARK();

		if ((nowTicks - lastSecond) > 1000) {
      currentFPS = static_cast<float>(frames) / (nowTicks - lastSecond) * 1000.f;
      avgFPS = static_cast<float>(totalFrames) / (nowTicks - startTicks) * 1000.f;
			frames = 0;
			lastSecond = nowTicks;

      fpsHistory[fpsHistoryNext] = peakFpsThisSecond;
      fpsHistoryNext = (fpsHistoryNext + 1) % kFpsHistorySeconds;
      fpsHistoryCount = std::min(fpsHistoryCount + 1, kFpsHistorySeconds);
      peakFpsThisSecond = 0.f;
		}

    {
      GFX1993_ZONE_N("app.imgui");
      ImGui_ImplSDLRenderer2_NewFrame();
      ImGui_ImplSDL2_NewFrame();
      ImGui::NewFrame();

      if (showStatsOverlay) {
        drawStatsOverlay();
      }

      ImGui::Render();
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, frameTexture, nullptr, nullptr);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(0);
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyTexture(frameTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
	SDL_Quit();
}

void DemoApp::createFrameTexture() {
  if (frameTexture != nullptr) {
    SDL_DestroyTexture(frameTexture);
  }
  frameTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING, width, height);
}

void DemoApp::updateFrameTexture() {
  GFX1993_ZONE_N("app.blit");

  // Resample buffer and switch to BGRA format.
  {
    GFX1993_ZONE_N("app.blit.resample");
    #pragma omp parallel for
    for (unsigned int h = 0; h < height; ++h) {
      for (unsigned int w = 0; w < width; ++w) {
        glm::vec2 coord(static_cast<float>(w) / width,
                        static_cast<float>(h) / height);

        const glm::vec4& pixel = renderConfig.framebuffer->getPixel(coord);
        // Also flip the y-axis.
        SDL_Color& c = pixels[w + h*width];

        // Also switch to BGRA.
        c.r = static_cast<Uint8>(pixel.b * 255);
        c.g = static_cast<Uint8>(pixel.g * 255);
        c.b = static_cast<Uint8>(pixel.r * 255);
        c.a = static_cast<Uint8>(pixel.a * 255);
      }
    }
  }

  {
    GFX1993_ZONE_N("app.blit.copy");
    SDL_UpdateTexture(frameTexture, nullptr, &pixels[0], width * sizeof(SDL_Color));
  }
}

namespace {

void drawDebugInfoRow(const char* label, const gfx1993::PrimitiveDebugInfo& info) {
  const uint32_t total = info.fragmentsDrawn + info.fragmentsDiscarded;
  const float effective = total > 0 ? static_cast<float>(info.fragmentsDrawn) / total : 0.f;

  ImGui::TableNextRow();
  ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
  ImGui::TableNextColumn(); ImGui::Text("%u", info.drawn);
  ImGui::TableNextColumn(); ImGui::Text("%u", info.backfaceCulled);
  ImGui::TableNextColumn(); ImGui::Text("%u", info.fragmentsDrawn);
  ImGui::TableNextColumn(); ImGui::Text("%u", info.fragmentsDiscarded);
  ImGui::TableNextColumn(); ImGui::Text("%.1f%%", effective * 100.f);
}

}  // namespace

void DemoApp::drawStatsOverlay() {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Render Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("FPS: %.1f (avg %.1f)", currentFPS, avgFPS);
    ImGui::Text("Frame time: %.2f ms", frameTimeMs);

    if (fpsHistoryCount > 0) {
      const int lastIndex = (fpsHistoryNext - 1 + kFpsHistorySeconds) % kFpsHistorySeconds;
      char overlay[32];
      snprintf(overlay, sizeof(overlay), "peak %.0f fps", fpsHistory[lastIndex]);

      const int offset = fpsHistoryCount < kFpsHistorySeconds ? 0 : fpsHistoryNext;
      ImGui::PlotLines("##peakFps", fpsHistory, fpsHistoryCount, offset, overlay,
                        0.f, FLT_MAX, ImVec2(0, 60));
      ImGui::TextUnformatted("Peak FPS, last 60s");
    }

    ImGui::Separator();

    const gfx1993::DebugInfo& info = rasterizer->getDebugInfo();
    if (ImGui::BeginTable("debugInfo", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
      ImGui::TableSetupColumn("Primitive");
      ImGui::TableSetupColumn("Drawn");
      ImGui::TableSetupColumn("Culled");
      ImGui::TableSetupColumn("Frag drawn");
      ImGui::TableSetupColumn("Frag discarded");
      ImGui::TableSetupColumn("Effective");
      ImGui::TableHeadersRow();

      drawDebugInfoRow("Points", info.points);
      drawDebugInfoRow("Lines", info.lines);
      drawDebugInfoRow("Triangles", info.triangles);
      drawDebugInfoRow("ScreenQuad", info.screenFillingQuad);
      drawDebugInfoRow("AABBs", info.aabbs);

      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void DemoApp::handleEvents() {
  GFX1993_ZONE_N("app.events");

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT) {
      running = false;
    }
    handleEvent(event);
  }
}

void DemoApp::handleEvent(const SDL_Event& event) {
  const ImGuiIO& io = ImGui::GetIO();

 if (event.type == SDL_KEYDOWN) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      running = false;
      break;
    default:
      break;
    }

    if (!io.WantCaptureKeyboard) {
      appInstance->handleKeyboard(event.key.keysym.sym, mousePosition);
    }
  }

  // TODO: handle keyup.
  if (event.type == SDL_MOUSEMOTION && !io.WantCaptureMouse) {
    appInstance->handleMotion(glm::ivec2(event.motion.x, event.motion.y));
  }

  if ((event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) && !io.WantCaptureMouse) {
    appInstance->handleMouse(event.button.button, event.button.state, glm::ivec2(event.button.x, event.button.y));
  }

  if (event.type == SDL_MOUSEWHEEL && !io.WantCaptureMouse) {
    appInstance->handleMouseWheel(event.wheel.y, mousePosition);
  }

  if (event.type == SDL_WINDOWEVENT) {
    if (event.window.event ==  SDL_WINDOWEVENT_RESIZED) {
      handleResize(event.window.data1, event.window.data2);
      std::cout << "Resized window to " << event.window.data1 << "x" << event.window.data2 << std::endl;
    }
  }
}

void DemoApp::handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) {
  if (key == '-') {
    camera->handleInputTranslate(glm::vec3(0,0,1));
  }
  if (key == '+' || key == '=') {
    camera->handleInputTranslate(glm::vec3(0,0,-1));
  }
}

void DemoApp::handleMouse(int button, int state, const glm::ivec2& mousePosition) {
  this->mousePosition = mousePosition;
}

void DemoApp::handleMotion(const glm::ivec2& newMousePosition) {
  const glm::ivec2 delta = newMousePosition - mousePosition;
  this->mousePosition = newMousePosition;
  
  camera->handleInputRotate(glm::vec3(delta.y, delta.x, 0.f));
}

void DemoApp::handleMouseWheel(int wheel, const glm::ivec2& mousePosition) {
  // For an orbit camera, translation along Z means adjusting the radius.
  camera->handleInputTranslate(glm::vec3(0,0, wheel));
}

void DemoApp::updateFrame(float dt) {
  camera->update(dt);
}

void DemoApp::handleResize(unsigned int w, unsigned int h) {
  pixels.resize(w*h*4);
  width = w;
  height = h;

  // Renderer isn't created yet the first time this runs, from the
  // constructor; the run() loop creates the initial texture itself.
  if (renderer != nullptr) {
    createFrameTexture();
  }
}
