#pragma once

#include "Metal.hpp"
#include "modules/input/InputEvent.hpp"

namespace dank {
namespace apple {
struct MetalView {
  MTL::Device *device;
  MTL::Library *shaderLibrary;
  MTL::RenderPassDescriptor *currentRenderPassDescriptor;
  MTL::Drawable *currentDrawable;
  int viewWidth;
  int viewHeight;
};


extern "C" void onStart(void *os);
extern "C" void onStop();
extern "C" void onHotReload();
extern "C" void onDraw(MetalView *graphics);
extern "C" void onResize(int width, int height);
extern "C" void onInputEvent(InputEvent &event);

typedef void (*OnStartFunctionType)(void *);
typedef void (*OnStopFunctionType)();
typedef void (*OnHotReloadFunctionType)();
typedef void (*OnDrawFunctionType)(MetalView *);
typedef void (*OnResizeFunctionType)(int, int);
typedef void (*OnInputEventFunctionType)(InputEvent &);
} // namespace apple
} // namespace dank