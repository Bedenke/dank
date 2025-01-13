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
            MTL::RenderCommandEncoder *renderCommandEncoder;

            MTL::RenderPipelineState *pipelineState = nullptr;
            MTL::Buffer *_pVertexPositionsBuffer = nullptr;
            MTL::Buffer *_pVertexColorsBuffer = nullptr;

        public:
            MetalView *view;
            void init() override;
            void clear() override;
        };
    } // namespace apple

} // namespace dank
