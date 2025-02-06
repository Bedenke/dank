#include "AppleRenderer.hpp"
#include "modules/Foundation.hpp"
#include "modules/engine/Console.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/textures/Texture.hpp"
#include "modules/scene/Scene.hpp"
#include "os/apple/Metal.hpp"
#include <cstddef>

using namespace dank;

void apple::AppleRenderer::initOrUpdateView(MetalView *view) {
  if (this->view == nullptr) {
    this->view = view;
    init();
  } else {
    this->view = view;
  }
}

void apple::AppleRenderer::init() {
  this->commandQueue = this->view->device->newCommandQueue();

  // Indirect command buffer
  {
    MTL::IndirectCommandBufferDescriptor *icbDescriptor =
        MTL::IndirectCommandBufferDescriptor::alloc()->init();
    icbDescriptor->setCommandTypes(MTL::IndirectCommandTypeDrawIndexed);
    icbDescriptor->setInheritBuffers(true);
    icbDescriptor->setInheritPipelineState(true);
    indirectCommandBuffer = this->view->device->newIndirectCommandBuffer(
        icbDescriptor, instancePageSize, 0);
    indirectCommandBuffer->setLabel(NS::String::string(
        "IndirectDrawCommands", NS::StringEncoding::UTF8StringEncoding));
    icbDescriptor->release();
  }

  // Mesh Instances Buffer
  {
    meshInstanceBuffer = view->device->newBuffer(
        sizeof(instance::InstanceData) * instancePageSize,
        MTL::ResourceStorageModeShared);
    meshInstanceBuffer->setLabel(NS::String::string(
        "MeshInstanceBuffer", NS::StringEncoding::UTF8StringEncoding));
  }

  {
    MTL::Function *vertexFunction =
        this->view->shaderLibrary->newFunction(NS::String::string(
            "vertexMain", NS::StringEncoding::UTF8StringEncoding));
    MTL::Function *fragmentFunction =
        this->view->shaderLibrary->newFunction(NS::String::string(
            "fragmentMain", NS::StringEncoding::UTF8StringEncoding));

    MTL::RenderPipelineDescriptor *pipelineDescriptor =
        MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexFunction);
    pipelineDescriptor->setFragmentFunction(fragmentFunction);
    pipelineDescriptor->setSupportIndirectCommandBuffers(true);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormat::PixelFormatRGBA8Unorm);

    // Enable blending
    MTL::RenderPipelineColorAttachmentDescriptor *colorAttachment =
        pipelineDescriptor->colorAttachments()->object(0);
    colorAttachment->setBlendingEnabled(true);
    colorAttachment->setRgbBlendOperation(MTL::BlendOperationAdd);
    colorAttachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
    colorAttachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    colorAttachment->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    colorAttachment->setDestinationRGBBlendFactor(
        MTL::BlendFactorOneMinusSourceAlpha);
    colorAttachment->setDestinationAlphaBlendFactor(
        MTL::BlendFactorOneMinusSourceAlpha);

    NS::Error *error;
    pipelineState =
        view->device->newRenderPipelineState(pipelineDescriptor, &error);

    if (!pipelineState) {
      console::log("%s", error->localizedDescription()->utf8String());
      assert(false);
    }

    vertexArgEncoder = vertexFunction->newArgumentEncoder(0);
    fragmentArgEncoder = fragmentFunction->newArgumentEncoder(0);

    vertexFunction->release();
    fragmentFunction->release();
    pipelineDescriptor->release();

    // Camera Buffer
    cameraUBOBuffer = view->device->newBuffer(sizeof(dank::CameraUBO),
                                              MTL::ResourceStorageModeShared);
    cameraUBOBuffer->setLabel(NS::String::string(
        "CameraUBO", NS::StringEncoding::UTF8StringEncoding));
  }

  {
    // Create a buffer to hold the encoded arguments
    fragmentArgBuffer = view->device->newBuffer(
        fragmentArgEncoder->encodedLength(), MTL::ResourceStorageModeShared);
    fragmentArgBuffer->setLabel(NS::String::string(
        "TextureArgBuffer", NS::StringEncoding::UTF8StringEncoding));
    // Encode the arguments into the buffer
    fragmentArgEncoder->setArgumentBuffer(fragmentArgBuffer, 0);
  }
}

void apple::AppleRenderer::prepareMeshes(dank::FrameContext &ctx) {
  if (meshLibraryLastModified == ctx.meshLibrary.lastModified)
    return;
  meshLibraryLastModified = ctx.meshLibrary.lastModified;

  if (meshVertexBuffer != nullptr) {
    meshVertexBuffer->release();
  }
  if (meshIndexBuffer != nullptr) {
    meshIndexBuffer->release();
  }

  mesh::MeshLibraryData mld{};
  ctx.meshLibrary.getData(mld);

  meshVertexBuffer = view->device->newBuffer(mld.vertexDataSize,
                                             MTL::ResourceStorageModeShared);
  meshVertexBuffer->setLabel(NS::String::string(
      "MeshVertexBuffer", NS::StringEncoding::UTF8StringEncoding));

  meshIndexBuffer = view->device->newBuffer(mld.indexDataSize,
                                            MTL::ResourceStorageModeShared);
  meshIndexBuffer->setLabel(NS::String::string(
      "MeshIndexBuffer", NS::StringEncoding::UTF8StringEncoding));

  memcpy(meshVertexBuffer->contents(), mld.vbo.data(), mld.vertexDataSize);
  memcpy(meshIndexBuffer->contents(), mld.ibo.data(), mld.indexDataSize);

  if (vertexArgBuffer == nullptr) {
    vertexArgBuffer = view->device->newBuffer(vertexArgEncoder->encodedLength(),
                                              MTL::ResourceStorageModeShared);
    vertexArgBuffer->setLabel(NS::String::string(
        "VertexArgBuffer", NS::StringEncoding::UTF8StringEncoding));
  }

  // Encode the vertex buffers into the argument buffer
  vertexArgEncoder->setArgumentBuffer(vertexArgBuffer, 0);

  // TODO: support multiple vertex buffers
  vertexArgEncoder->setBuffer(meshVertexBuffer, 0, 0);

  dank::console::log("[AppleRenderer] vertex buffer updated");
}

void apple::AppleRenderer::prepareTextures(dank::FrameContext &ctx) {
  uint32_t activeTextureCount = 0;

  for (const auto &entry : ctx.textureLibrary.textures) {
    auto *texture = entry.second;

    auto &state = textureState[entry.first];

    texture::TextureData td{};
    texture->fetchData(td);

    if (td.state == ResourceState::Idle || td.state == ResourceState::Invalid) {

      state.active = false;
      if (state.mtlTexture != nullptr) {
        state.mtlTexture->release();
        state.mtlTexture = nullptr;
      }

      texture->releaseData(td);
      continue;
    }

    if (td.state == ResourceState::Loading) {
      state.active = false;
      continue;
    }

    if (state.lastModified == td.lastModified) {
      activeTextureCount++;
      continue;
    }

    state.lastModified = td.lastModified;

    if (state.mtlTexture == nullptr) {

      MTL::TextureDescriptor *textureDesc =
          MTL::TextureDescriptor::alloc()->init();
      textureDesc->setWidth(td.width);
      textureDesc->setHeight(td.height);

      // Set pixel format
      switch (td.format) {
      case dank::PixelFormat::BGRA32:
        textureDesc->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);
        break;
      case dank::PixelFormat::RGBA8Unorm:
        textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
        break;
      }
      textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);

      if (texture->getType() == texture::TextureType::Color) {
        textureDesc->setTextureType(MTL::TextureType2D);
      } else {
        throw std::runtime_error("Unsupported texture type");
      }
      textureDesc->setStorageMode(MTL::StorageModeShared);
      textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);

      state.mtlTexture = view->device->newTexture(textureDesc);
      state.index = activeTextureCount;
      textureDesc->release();

      fragmentArgEncoder->setTexture(state.mtlTexture, state.index);

      dank::console::log("[AppleRenderer] added new texture: %d",
                         activeTextureCount);
    }

    NS::UInteger bytesPerRow = td.width * td.channels;
    state.mtlTexture->replaceRegion(
        MTL::Region(0, 0, 0, td.width, td.height, 1), 0, td.data, bytesPerRow);
    texture->releaseData(td);

    state.active = true;
    activeTextureCount++;
  }
}

void apple::AppleRenderer::render(FrameContext &ctx, Scene *scene) {
  prepareMeshes(ctx);
  prepareTextures(ctx);

  MTL::RenderPassDescriptor *renderPassDescriptor =
      this->view->currentRenderPassDescriptor;
  if (renderPassDescriptor == nullptr)
    return;

  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer *commandBuffer = this->commandQueue->commandBuffer();
  MTL::RenderCommandEncoder *renderEncoder =
      commandBuffer->renderCommandEncoder(renderPassDescriptor);

  renderEncoder->setRenderPipelineState(this->pipelineState);
  renderEncoder->setVertexBuffer(vertexArgBuffer, 0, 0);

  // Camera
  auto cameraUBO =
      reinterpret_cast<dank::CameraUBO *>(cameraUBOBuffer->contents());
  scene->camera.getCameraUBO(cameraUBO);

  renderEncoder->setVertexBuffer(cameraUBOBuffer, 0, 1);

  // Set the argument buffer in the render command encoder
  renderEncoder->setFragmentBuffer(fragmentArgBuffer, 0, 0);

  for (const auto &entry : textureState) {
    if (entry.second.active) {
      renderEncoder->useResource(entry.second.mtlTexture,
                                 MTL::ResourceUsageSample);
    }
  }

  uint32_t meshInstanceCount = 0;
  auto view = ctx.draw.view<draw::Mesh>();
  for (auto [entity, mesh] : view.each()) {
    const auto textureDescriptor = textureState[mesh.textureId];
    if (!textureDescriptor.active)
      continue;

    const auto meshDescriptor = ctx.meshLibrary.get(mesh.meshId);

    instance::InstanceData *bufferData =
        reinterpret_cast<instance::InstanceData *>(
            meshInstanceBuffer->contents());
    bufferData[meshInstanceCount].transform = mesh.transform;
    bufferData[meshInstanceCount].color = mesh.color;
    bufferData[meshInstanceCount].bufferIndex = meshDescriptor->bufferIndex;
    bufferData[meshInstanceCount].textureIndex = textureDescriptor.index;

    MTL::IndirectRenderCommand *command =
        indirectCommandBuffer->indirectRenderCommand(meshInstanceCount);

    command->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        NS::UInteger(meshDescriptor->indexCount), MTL::IndexTypeUInt32,
        meshIndexBuffer,
        NS::UInteger(meshDescriptor->indexOffset * sizeof(uint32_t)), 1, 0,
        meshInstanceCount);

    meshInstanceCount++;
  }

  renderEncoder->setVertexBuffer(meshInstanceBuffer, 0, 2);
  renderEncoder->useResource(meshInstanceBuffer, MTL::ResourceUsageRead);

  renderEncoder->executeCommandsInBuffer(indirectCommandBuffer,
                                         NS::Range(0, meshInstanceCount));
  renderEncoder->endEncoding();
  commandBuffer->presentDrawable(this->view->currentDrawable);
  commandBuffer->commit();

  pool->release();
}

void apple::AppleRenderer::release() {
  if (meshVertexBuffer != nullptr) {
    meshVertexBuffer->release();
    meshVertexBuffer = nullptr;
  }
  if (meshIndexBuffer != nullptr) {
    meshIndexBuffer->release();
    meshIndexBuffer = nullptr;
  }

  if (cameraUBOBuffer != nullptr) {
    cameraUBOBuffer->release();
    cameraUBOBuffer = nullptr;
  }
  if (vertexArgBuffer != nullptr) {
    vertexArgBuffer->release();
    vertexArgBuffer = nullptr;
  }
  if (fragmentArgBuffer != nullptr) {
    fragmentArgBuffer->release();
    fragmentArgBuffer = nullptr;
  }
  if (meshInstanceBuffer != nullptr) {
    meshInstanceBuffer->release();
    meshInstanceBuffer = nullptr;
  }

  if (vertexArgEncoder != nullptr) {
    vertexArgEncoder->release();
    vertexArgEncoder = nullptr;
  }
  if (fragmentArgEncoder != nullptr) {
    fragmentArgEncoder->release();
    fragmentArgEncoder = nullptr;
  }
  if (indirectCommandBuffer != nullptr) {
    indirectCommandBuffer->release();
    indirectCommandBuffer = nullptr;
  }

  for (auto &entry : textureState) {
    entry.second.mtlTexture->release();
  }
  textureState.clear();

  if (pipelineState != nullptr) {
    pipelineState->release();
    pipelineState = nullptr;
  }

  if (commandQueue != nullptr) {
    commandQueue->release();
    commandQueue = nullptr;
  }

  dank::console::log("[AppleRenderer] released");
}