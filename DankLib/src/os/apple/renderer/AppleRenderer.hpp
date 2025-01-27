#pragma once

#include "modules/renderer/Renderer.hpp"
#include "os/apple/AppleOS.hpp"
#include "os/apple/Metal.hpp"

namespace dank {
namespace apple {

class AppleRenderer : public dank::Renderer {
private:
  MTL::CommandQueue *commandQueue;
  MTL::IndirectCommandBuffer *indirectCommandBuffer = nullptr;
  MTL::RenderPipelineState *pipelineState = nullptr;
  
  MTL::Buffer *meshVertexBuffer = nullptr;
  MTL::Buffer *meshIndexBuffer = nullptr;
  uint32_t meshesLastModified = 0;
  uint32_t texturesLastModified = 0;
  std::map<uint32_t, MTL::Texture*> textures{};
  void init();
  void prepareMeshes(dank::FrameContext &ctx);
  void prepareTextures(dank::FrameContext &ctx);

  MTL::ArgumentEncoder *vertexArgEncoder;
  MTL::Buffer *vertexArgBuffer;
  MTL::ArgumentEncoder *fragmentArgEncoder;
  MTL::Buffer *fragmentArgBuffer;
  MTL::Buffer *cameraUBOBuffer;

  uint32_t instancePageSize = 1024;
  MTL::Buffer *meshInstanceBuffer = nullptr;
  MetalView *view;
public:
  ~AppleRenderer() {
    release();
  }
  void initOrUpdateView(MetalView *view);
  void render(FrameContext &ctx, Scene *scene) override;
  void release();
};
} // namespace apple

} // namespace dank
