#pragma once

#include "Metal.hpp"

namespace dank
{
    namespace apple
    {
        struct MetalView
        {
            MTL::Device *device;
            MTL::Library *shaderLibrary;
            MTL::RenderPassDescriptor *currentRenderPassDescriptor;
            MTL::Drawable *currentDrawable;
        };

        extern "C" void onStart();
        extern "C" void onStop();
        extern "C" void onHotReload();
        extern "C" void onDraw(MetalView *graphics);
        extern "C" void onResize(int width, int height);

        typedef void (*OnStartFunctionType)();
        typedef void (*OnStopFunctionType)();
        typedef void (*OnHotReloadFunctionType)();
        typedef void (*OnDrawFunctionType)(MetalView*);
        typedef void (*OnResizeFunctionType)(int,int);
    }
}