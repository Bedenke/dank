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
    MTL::Function *pVertexFn =
        this->view->shaderLibrary->newFunction(NS::String::string(
            "vertexMain", NS::StringEncoding::UTF8StringEncoding));
    MTL::Function *pFragFn =
        this->view->shaderLibrary->newFunction(NS::String::string(
            "fragmentMain", NS::StringEncoding::UTF8StringEncoding));

    MTL::RenderPipelineDescriptor *pDesc =
        MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setVertexFunction(pVertexFn);
    pDesc->setFragmentFunction(pFragFn);
    pDesc->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

    NS::Error *pError;
    pipelineState = view->device->newRenderPipelineState(pDesc, &pError);
    if (!pipelineState) {
      console::log("%s", pError->localizedDescription()->utf8String());
      assert(false);
    }

    vertexArgEncoder = pVertexFn->newArgumentEncoder(0);
    fragmentArgEncoder = pFragFn->newArgumentEncoder(0);

    pVertexFn->release();
    pFragFn->release();
    pDesc->release();
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
  vertexArgBuffer = view->device->newBuffer(vertexArgEncoder->encodedLength(), MTL::ResourceStorageModeShared);

  // Encode the vertex buffers into the argument buffer
  vertexArgEncoder->setArgumentBuffer(vertexArgBuffer, 0);
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

    MTL::TextureDescriptor *pTextureDesc =
        MTL::TextureDescriptor::alloc()->init();
    pTextureDesc->setWidth(td.width);
    pTextureDesc->setHeight(td.height);
    if (desc.texture->getType() == texture::TextureType::Color) {
      pTextureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
      pTextureDesc->setTextureType(MTL::TextureType2D);
    }
    pTextureDesc->setStorageMode(MTL::StorageModeShared);
    pTextureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);

    MTL::Texture *pTexture = view->device->newTexture(pTextureDesc);
    pTexture->replaceRegion(MTL::Region(0, 0, 0, td.width, td.height, 1), 0,
                            td.data.data(), td.width * td.channels);
    pTextureDesc->release();

    textures[desc.index] = pTexture;

    fragmentArgEncoder->setTexture(pTexture, desc.index);
  }

  dank::console::log("[AppleRenderer] textures updated: %d", textures.size());
}

void apple::AppleRenderer::render(dank::FrameContext &ctx) {

  prepareMeshes(ctx);
  prepareTextures(ctx);

  MTL::RenderPassDescriptor *pRpd = this->view->currentRenderPassDescriptor;
  if (pRpd == nullptr)
    return;

  NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer *pCmd = this->commandQueue->commandBuffer();
  MTL::RenderCommandEncoder *pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(this->pipelineState);
  pEnc->setVertexBuffer(vertexArgBuffer, 0, 0);
  //pEnc->useResource(meshVertexBuffer, MTL::ResourceUsageRead);

  // Set the argument buffer in the render command encoder
  pEnc->setFragmentBuffer(fragmentArgBuffer, 0, 0);

  for (const auto &entry : textures) {
    pEnc->useResource(entry.second, MTL::ResourceUsageSample);
  }

  auto view = ctx.draw.view<draw::Mesh>();
  for (auto [entity, mesh] : view.each()) {
    const auto meshDescriptor = ctx.meshLibrary.get(mesh.meshId);
    pEnc->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle,
        NS::UInteger(meshDescriptor->indexCount), MTL::IndexTypeUInt32,
        meshIndexBuffer,
        NS::UInteger(meshDescriptor->indexOffset * sizeof(uint32_t)));
  }

  pEnc->endEncoding();
  pCmd->presentDrawable(this->view->currentDrawable);
  pCmd->commit();

  // Release the argument encoder and buffer
  // argEncoder->release();
  // argBuffer->release();

  pPool->release();
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