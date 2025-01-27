#include "modules/renderer/Renderer.hpp"
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal.hpp"

#include "AppleOS.hpp"
#include "modules/engine/Console.hpp"
#include "modules/engine/Engine.hpp"
#include "os/apple/renderer/AppleRenderer.hpp"

using namespace dank;

Engine *engine;
apple::AppleRenderer *renderer;
bool viewResized = false;

void apple::onStart() {
  engine = new Engine();
  renderer = new apple::AppleRenderer();
  console::log("[AppleOS] starting");
}

void apple::onStop() {
  delete engine;
  delete renderer;
  console::log("[AppleOS] stopped");
}

void apple::onHotReload() {
  engine = new Engine();
  renderer = new apple::AppleRenderer();
  console::log("[AppleOS] hot reload");
}

void apple::onDraw(MetalView *view) {
  if (viewResized) {
    viewResized = false;
    int width = view->viewWidth;
    int height = view->viewHeight;
    engine->onViewResize(width, height);
    console::log("[AppleOS] view resized %dx%d", width, height);
  }

  engine->update();

  renderer->initOrUpdateView(view);
  renderer->render(engine->ctx, engine->scene);
}

void apple::onResize(int width, int height) { viewResized = true; }
