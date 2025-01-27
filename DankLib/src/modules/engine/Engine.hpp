#pragma once

#include "modules/Foundation.hpp"
#include "modules/FrameContext.hpp"
#include "modules/scene/Scene.hpp"

namespace dank
{
 
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
        FrameContext ctx;
        Scene *scene = nullptr;
        Engine();
        ~Engine();
        void onViewResize(float viewWidth, float viewHeight);
        void update();
    };

} // namespace dank
