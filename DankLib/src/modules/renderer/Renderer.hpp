#pragma once

namespace dank
{
    class Renderer
    {
    public:
        virtual void init() = 0;
        virtual void clear() = 0;
    };
} // namespace dank
