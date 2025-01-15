#pragma once
#include "modules/FrameContext.hpp"

namespace dank
{
    class Renderer
    {
    public:
        virtual void init() = 0;
        virtual void render(const FrameContext &ctx) = 0;
        virtual void release() = 0;
    };

    namespace draw
    {
        struct Mesh
        {
            uint32_t meshId;
        };
    }

}
