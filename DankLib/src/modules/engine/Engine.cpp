#include "Engine.hpp"
#include "Console.hpp"
#include "modules/renderer/meshes/RectangleMesh.hpp"
#include "modules/renderer/meshes/TriangleMesh.hpp"
#include <chrono>

double dank::Engine::getTimeInMilliseconds() const
{
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<double>(duration.count());
}

void dank::Engine::init()
{
    mesh::Triangle triangleMesh{};
    mesh::Rectangle rectangleMesh{};
    ctx.meshLibrary.add(triangleMesh);
    ctx.meshLibrary.add(rectangleMesh);
    
    scene = new dank::Scene();
    framePerSecondAccumulator.lastTime = getTimeInMilliseconds();
    dank::console::log("Engine initialized");
}

void dank::Engine::update(Renderer *renderer)
{

    // Calculate delta time
    double currentTime = getTimeInMilliseconds();
    double deltaTime = currentTime - framePerSecondAccumulator.lastTime;

    // Calculate frames per second
    framePerSecondAccumulator.lastTime = currentTime;
    framePerSecondAccumulator.time += deltaTime;
    framePerSecondAccumulator.frames++;
    if (framePerSecondAccumulator.time >= 1000)
    {
        ctx.framesPerSecond = framePerSecondAccumulator.frames;
        framePerSecondAccumulator.time = 0;
        framePerSecondAccumulator.frames = 0;
        dank::console::log("fps: %d | %.2fms", ctx.framesPerSecond, deltaTime);
    }

    // Update context
    ctx.deltaTime = deltaTime;
    ctx.absoluteTime += deltaTime;
    ctx.absoluteFrame++;

    // Update scene
    if (scene != nullptr)
    {
        scene->update(ctx);
    }

    renderer->render(ctx);
}
