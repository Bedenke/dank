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
    dank::console::log("Starting platform [Apple]");
    engine.init();
}

void dank::apple::onDraw(MetalView *view)
{
    if (renderer.view == nullptr)
    {
        renderer.view = view;
        renderer.init();
    }

    engine.update();
    engine.render(&renderer);
}

void dank::apple::onResize(int width, int height)
{
    dank::console::log("Resized");
}
