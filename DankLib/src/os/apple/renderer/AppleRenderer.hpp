#pragma once

#include "modules/renderer/Renderer.hpp"
#include "os/apple/AppleOS.hpp"

namespace dank {
namespace apple {

class AppleRenderer : public dank::Renderer {
private:
  MTL::CommandQueue *commandQueue;
  MTL::RenderPipelineState *pipelineState = nullptr;
  
  MTL::Buffer *meshVertexBuffer = nullptr;
  MTL::Buffer *meshIndexBuffer = nullptr;
  uint32_t meshesLastModified = 0;
  void prepareMeshes(const dank::FrameContext &ctx);

public:
  MetalView *view;
  void init() override;
  void render(const dank::FrameContext &ctx) override;
  void release() override;
};
} // namespace apple

} // namespace dank
