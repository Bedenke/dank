#include "Engine.hpp"
#include "Console.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include "modules/renderer/textures/DebugTexture.hpp"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb/stb_image.h"

using namespace dank;

double Engine::getTimeInMilliseconds() const {
  auto now = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch());
  return static_cast<double>(duration.count());
}

Engine::Engine() {
  scene = new Scene();
  framePerSecondAccumulator.lastTime = getTimeInMilliseconds();
  console::log("Engine initialized");
}

Engine::~Engine() {
  delete scene;
  console::log("Engine released");
}

void Engine::onViewResize(float viewWidth, float viewHeight) {
  scene->camera.onViewResize(viewWidth, viewHeight);
}

void Engine::update() {

  // Calculate delta time
  double currentTime = getTimeInMilliseconds();
  double deltaTime = currentTime - framePerSecondAccumulator.lastTime;

  // Calculate frames per second
  framePerSecondAccumulator.lastTime = currentTime;
  framePerSecondAccumulator.time += deltaTime;
  framePerSecondAccumulator.frames++;
  if (framePerSecondAccumulator.time >= 1000) {
    ctx.framesPerSecond = framePerSecondAccumulator.frames;
    framePerSecondAccumulator.time = 0;
    framePerSecondAccumulator.frames = 0;
    console::log("[dank] fps: %d | %.2fms", ctx.framesPerSecond, deltaTime);
  }

  // Update time
  ctx.deltaTime = deltaTime;
  ctx.absoluteTime += deltaTime;
  ctx.absoluteFrame++;

  ctx.input.update(deltaTime);
  
  scene->update(ctx);
}
