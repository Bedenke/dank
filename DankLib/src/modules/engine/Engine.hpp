#pragma once

#include "modules/Foundation.hpp"
#include "modules/scene/Scene.hpp"
#include "modules/renderer/Renderer.hpp"

namespace dank
{
    struct Context
    {
        bool paused{false};
        float deltaTime = 0;
        uint32_t framesPerSecond = 0;
        float absoluteTime = 0;
        uint32_t absoluteFrame = 0;
    };

    class Engine
    {
    private:
        struct FramePerSecondAccumulator
        {
            double time = 0;
            double lastTime = 0;
            int frames = 0;
        } framePerSecondAccumulator;

        double getTimeInMilliseconds() const;

    public:
        Context ctx;
        Scene *scene = nullptr;
        void init();
        void update();
        void render(Renderer *renderer);
    };

} // namespace dank
