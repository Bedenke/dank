#pragma once
#include "modules/renderer/Renderer.hpp"
#include "os/apple/AppleOS.hpp"

namespace dank
{
    namespace apple
    {

        class AppleRenderer : public dank::Renderer
        {
        private:
            MTL::CommandQueue *commandQueue;
            MTL::RenderPipelineState *pipelineState;
            MTL::RenderCommandEncoder *renderCommandEncoder;
        public:
            MetalView *view;
            void init() override;
            void clear() override;
        };
    } // namespace apple

} // namespace dank
