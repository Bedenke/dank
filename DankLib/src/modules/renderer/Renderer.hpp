#pragma once
#include "modules/FrameContext.hpp"
#include "modules/scene/Scene.hpp"

namespace dank
{
    class Renderer
    {
    public:
        virtual void render(FrameContext &ctx, Scene *scene) = 0;
    };

    namespace draw
    {
        struct Mesh
        {
            uint32_t meshId;
        };
    }

}
