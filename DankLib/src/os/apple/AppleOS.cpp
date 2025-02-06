#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal.hpp"

#include "AppleOS.hpp"
#include "modules/engine/Console.hpp"
#include "modules/engine/Engine.hpp"
#include "modules/input/Input.hpp"
#include "modules/os/OS.hpp"
#include "os/apple/renderer/AppleRenderer.hpp"

using namespace dank;

Engine *engine;
apple::AppleRenderer *renderer;
bool viewResized = false;
dank::OS *dank::os = nullptr;

void apple::onStart(void *os) {
  console::log("[AppleOS] starting ");
  dank::os = (OS *)os;
  engine = new Engine();
  renderer = new apple::AppleRenderer();
}

void apple::onStop() {
  delete engine;
  delete renderer;
  console::log("[AppleOS] stopped");
}

void apple::onHotReload() { console::log("[AppleOS] hot reloaded!"); }

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

void apple::onInputEvent(InputEvent &event) {
  switch (event.type) {
  case InputEventType::KeyDown:
    dank::input.handleKeyDown(event.key);
    break;
  case InputEventType::KeyUp:
    dank::input.handleKeyUp(event.key);
    break;
  case InputEventType::KeyTyped:
    dank::input.handleKeyTyped(event.key, event.character);
    break;
  case InputEventType::MouseDrag: {
    TouchData touchData{};
    touchData.x = (float)event.x;
    touchData.y = (float)event.y;
    touchData.button = event.button;
    dank::input.handleTouchMove(touchData);
  } break;
  case InputEventType::MouseMove: {
    TouchData touchData{};
    touchData.x = (float)event.x;
    touchData.y = (float)event.y;
    touchData.button = event.button;
    dank::input.handleTouchMove(touchData);
  } break;
  case InputEventType::MouseDown: {
    TouchData touchData{};
    touchData.x = (float)event.x;
    touchData.y = (float)event.y;
    touchData.button = event.button;
    dank::input.handleTouchDown(touchData);
  } break;
  case InputEventType::MouseUp: {
    TouchData touchData{};
    touchData.x = (float)event.x;
    touchData.y = (float)event.y;
    touchData.button = event.button;
    dank::input.handleTouchUp(touchData);
  } break;
  case InputEventType::MouseScroll: {
    dank::input.handleWheel(event.wheelDelta);

  } break;
  }
}
