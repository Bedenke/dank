#pragma once

#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/textures/Texture.hpp"
#include "os/apple/AppleOS.hpp"
#include "os/apple/Metal.hpp"
#include <cstdint>
#include <vector>

namespace dank {
namespace apple {

  struct TextureState {
    uint32_t lastModified{0};
    uint32_t index;
    MTL::Texture* mtlTexture{nullptr};
    bool active;
  };

class AppleRenderer : public dank::Renderer {
private:
  MTL::CommandQueue *commandQueue;
  MTL::IndirectCommandBuffer *indirectCommandBuffer = nullptr;
  MTL::RenderPipelineState *pipelineState = nullptr;
  
  MTL::Buffer *meshVertexBuffer = nullptr;
  MTL::Buffer *meshIndexBuffer = nullptr;
  uint32_t meshLibraryLastModified = 0;

  std::map<uint32_t, TextureState> textureState{};
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
