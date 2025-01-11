#pragma once

#include "modules/renderer/Renderer.hpp"

namespace dank
{
    class Scene
    {
    public:
        void update(const float dt) const;
        void draw(Renderer *renderer) const;
    };
} // namespace dank
