#include "Engine.hpp"
#include "Console.hpp"
#include <chrono>

double dank::Engine::getTimeInMilliseconds() const
{
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<double>(duration.count());
}

void dank::Engine::init()
{
    dank::console::log("Engine initialized");
    scene = new dank::Scene();
    framePerSecondAccumulator.lastTime = getTimeInMilliseconds();
}

void dank::Engine::update()
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
        dank::console::log("fps: %d %.2f", ctx.framesPerSecond, deltaTime);
    }

    // Update context
    ctx.deltaTime = deltaTime;
    ctx.absoluteTime += deltaTime;
    ctx.absoluteFrame++;

    // Update scene
    if (scene != nullptr)
    {
        scene->update(deltaTime);
    }
}

void dank::Engine::render(Renderer *renderer)
{
    if (scene != nullptr)
    {
        scene->draw(renderer);
    }
}