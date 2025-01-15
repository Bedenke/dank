#pragma once

#include "modules/Foundation.hpp"
#include "modules/FrameContext.hpp"

namespace dank
{
    class Scene
    {
    public:
        entt::registry entities{};
        void update(FrameContext &ctx);
    };
} // namespace dank
