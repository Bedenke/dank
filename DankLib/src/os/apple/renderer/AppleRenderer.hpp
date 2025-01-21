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
  uint32_t texturesLastModified = 0;
  std::map<uint32_t, MTL::Texture*> textures{};
  void prepareMeshes(dank::FrameContext &ctx);
  void prepareTextures(dank::FrameContext &ctx);

  MTL::ArgumentEncoder *vertexArgEncoder;
  MTL::Buffer *vertexArgBuffer;
  MTL::ArgumentEncoder *fragmentArgEncoder;
  MTL::Buffer *fragmentArgBuffer;
public:
  MetalView *view;
  void init() override;
  void render(dank::FrameContext &ctx) override;
  void release() override;
};
} // namespace apple

} // namespace dank
