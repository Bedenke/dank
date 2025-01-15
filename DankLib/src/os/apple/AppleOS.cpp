#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal.hpp"

#include "AppleOS.hpp"
#include "modules/engine/Engine.hpp"
#include "modules/engine/Console.hpp"
#include "os/apple/renderer/AppleRenderer.hpp"

dank::Engine engine;
dank::apple::AppleRenderer renderer;

void dank::apple::onStart()
{
    dank::console::log("[AppleOS] starting");
    engine.init();
}


void dank::apple::onStop()
{
    renderer.release();
    dank::console::log("[AppleOS] stopped");
}

void dank::apple::onHotReload()
{
    dank::console::log("[AppleOS] hot reload");
    engine.init();
}

void dank::apple::onDraw(MetalView *view)
{
    if (renderer.view == nullptr)
    {
        dank::console::log("[AppleOS] init renderer");
        renderer.view = view;
        renderer.init();
    }

    engine.update(&renderer);
}

void dank::apple::onResize(int width, int height)
{
    dank::console::log("[AppleOS] resized");
}
