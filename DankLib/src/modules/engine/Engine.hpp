#pragma once

#include "modules/Foundation.hpp"
#include "modules/FrameContext.hpp"
#include "modules/scene/Scene.hpp"
#include "modules/renderer/Renderer.hpp"

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
        void init();
        void update(Renderer *renderer);
    };

} // namespace dank
