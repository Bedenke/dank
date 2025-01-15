#include "AppleRenderer.hpp"
#include "modules/engine/Console.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/renderer/meshes/Mesh.hpp"
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

    pVertexFn->release();
    pFragFn->release();
    pDesc->release();
  }
}

void apple::AppleRenderer::prepareMeshes(const dank::FrameContext &ctx) {
  if (meshesLastModified == ctx.meshLibrary.lastModified)
    return;
  meshesLastModified = ctx.meshLibrary.lastModified;

  if (meshVertexBuffer != nullptr) {
    meshVertexBuffer->release();
  }
  if (meshIndexBuffer != nullptr) {
    meshIndexBuffer->release();
  }

  meshVertexBuffer = view->device->newBuffer(ctx.meshLibrary.vertexDataSize,
                                             MTL::ResourceStorageModeShared);

  meshIndexBuffer = view->device->newBuffer(ctx.meshLibrary.indexDataSize,
                                            MTL::ResourceStorageModeShared);

  memcpy(meshVertexBuffer->contents(), ctx.meshLibrary.vbo.data(),
         ctx.meshLibrary.vertexDataSize);
  memcpy(meshIndexBuffer->contents(), ctx.meshLibrary.ibo.data(),
         ctx.meshLibrary.indexDataSize);

  dank::console::log("[AppleRenderer] vertex buffer updated");
}

void apple::AppleRenderer::render(const dank::FrameContext &ctx) {

  prepareMeshes(ctx);
  
  MTL::RenderPassDescriptor *pRpd = this->view->currentRenderPassDescriptor;
  if (pRpd == nullptr) return;
  
  NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();


  MTL::CommandBuffer *pCmd = this->commandQueue->commandBuffer();
  MTL::RenderCommandEncoder *pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(this->pipelineState);
  pEnc->setVertexBuffer(meshVertexBuffer, 0, 0);

  auto view = ctx.draw.view<draw::Mesh>();
  for (auto [entity, mesh] : view.each()) {
    const auto meshDescriptor = ctx.meshLibrary.get(mesh.meshId);
    pEnc->drawIndexedPrimitives(
        MTL::PrimitiveType::PrimitiveTypeTriangle, meshDescriptor->indexCount,
        MTL::IndexTypeUInt32, meshIndexBuffer, meshDescriptor->indexOffset);
  }

  pEnc->endEncoding();
  pCmd->presentDrawable(this->view->currentDrawable);
  pCmd->commit();

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

  if (pipelineState != nullptr) {
    pipelineState->release();
    pipelineState = nullptr;
  }
  
  if (commandQueue != nullptr) {
    commandQueue->release();
    commandQueue = nullptr;
  }
}