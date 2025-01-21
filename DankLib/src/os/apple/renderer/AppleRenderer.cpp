#include "AppleRenderer.hpp"
#include "modules/engine/Console.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
#include "modules/renderer/textures/Texture.hpp"
#include "os/apple/Metal.hpp"
#include <cstdint>
#include <simd/simd.h>
#include <sys/types.h>

using namespace dank;

void apple::AppleRenderer::init() {
  this->commandQueue = this->view->device->newCommandQueue();

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
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

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
  }
}

void apple::AppleRenderer::prepareMeshes(dank::FrameContext &ctx) {
  if (meshesLastModified == ctx.meshLibrary.lastModified)
    return;
  meshesLastModified = ctx.meshLibrary.lastModified;

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

  meshIndexBuffer = view->device->newBuffer(mld.indexDataSize,
                                            MTL::ResourceStorageModeShared);

  memcpy(meshVertexBuffer->contents(), mld.vbo.data(), mld.vertexDataSize);
  memcpy(meshIndexBuffer->contents(), mld.ibo.data(), mld.indexDataSize);

  // TODO: free previous vertexArgBuffer?
  vertexArgBuffer = view->device->newBuffer(vertexArgEncoder->encodedLength(),
                                            MTL::ResourceStorageModeShared);

  // Encode the vertex buffers into the argument buffer
  vertexArgEncoder->setArgumentBuffer(vertexArgBuffer, 0);

  // TODO: support multiple vertex buffers
  vertexArgEncoder->setBuffer(meshVertexBuffer, 0, 0);

  dank::console::log("[AppleRenderer] vertex buffer updated");
}

void apple::AppleRenderer::prepareTextures(dank::FrameContext &ctx) {
  if (texturesLastModified == ctx.textureLibrary.lastModified)
    return;

  texturesLastModified = ctx.textureLibrary.lastModified;

  textures.clear();

  // TODO: Release previous fragmentArgBuffer?

  // Create a buffer to hold the encoded arguments
  fragmentArgBuffer = view->device->newBuffer(
      fragmentArgEncoder->encodedLength(), MTL::ResourceStorageModeShared);

  // Encode the arguments into the buffer
  fragmentArgEncoder->setArgumentBuffer(fragmentArgBuffer, 0);

  for (const auto &entry : ctx.textureLibrary.descriptors) {

    const texture::TextureDescriptor &desc = entry.second;

    texture::TextureData td{};
    desc.texture->getData(td);

    MTL::TextureDescriptor *textureDesc =
        MTL::TextureDescriptor::alloc()->init();
    textureDesc->setWidth(td.width);
    textureDesc->setHeight(td.height);
    if (desc.texture->getType() == texture::TextureType::Color) {
      textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
      textureDesc->setTextureType(MTL::TextureType2D);
    }
    textureDesc->setStorageMode(MTL::StorageModeShared);
    textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);

    MTL::Texture *texture = view->device->newTexture(textureDesc);
    texture->replaceRegion(MTL::Region(0, 0, 0, td.width, td.height, 1), 0,
                           td.data.data(), td.width * td.channels);
    textureDesc->release();

    textures[desc.index] = texture;

    fragmentArgEncoder->setTexture(texture, desc.index);
  }

  dank::console::log("[AppleRenderer] textures updated: %d", textures.size());
}

void apple::AppleRenderer::render(dank::FrameContext &ctx) {

  prepareMeshes(ctx);
  prepareTextures(ctx);

  MTL::RenderPassDescriptor *renderPassDescriptor = this->view->currentRenderPassDescriptor;
  if (renderPassDescriptor == nullptr)
    return;

  NS::AutoreleasePool *pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer *commandBuffer = this->commandQueue->commandBuffer();
  MTL::RenderCommandEncoder *renderEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

  renderEncoder->setRenderPipelineState(this->pipelineState);
  renderEncoder->setVertexBuffer(vertexArgBuffer, 0, 0);

  // Set the argument buffer in the render command encoder
  renderEncoder->setFragmentBuffer(fragmentArgBuffer, 0, 0);

  for (const auto &entry : textures) {
    renderEncoder->useResource(entry.second, MTL::ResourceUsageSample);
  }

  auto view = ctx.draw.view<draw::Mesh>();
  for (auto [entity, mesh] : view.each()) {
    const auto meshDescriptor = ctx.meshLibrary.get(mesh.meshId);
    renderEncoder->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        NS::UInteger(meshDescriptor->indexCount), MTL::IndexTypeUInt32,
        meshIndexBuffer,
        NS::UInteger(meshDescriptor->indexOffset * sizeof(uint32_t)));
  }

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

  for (auto &entry : textures) {
    entry.second->release();
  }
  textures.clear();

  if (pipelineState != nullptr) {
    pipelineState->release();
    pipelineState = nullptr;
  }

  if (commandQueue != nullptr) {
    commandQueue->release();
    commandQueue = nullptr;
  }
}