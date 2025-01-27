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

Engine engine{};
apple::AppleRenderer renderer{};

void apple::onStart() { console::log("[AppleOS] starting"); }

void apple::onStop() {
  renderer.release();
  console::log("[AppleOS] stopped");
}

void apple::onHotReload() { console::log("[AppleOS] hot reload"); }

void apple::onDraw(MetalView *view) {
  engine.update();

  renderer.initOrUpdateView(view);
  renderer.render(engine.ctx, engine.scene);
}

void apple::onResize(int width, int height) {
  engine.onViewResize(width, height);
  console::log("[AppleOS] view resized %dx%d", width, height);
}
