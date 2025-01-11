#include "AppleRenderer.hpp"

void dank::apple::AppleRenderer::init()
{
    this->commandQueue = this->view->device->newCommandQueue();
}

void dank::apple::AppleRenderer::clear()
{
    // Clear the screen

    NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();

    MTL::RenderPassDescriptor *pRpd = this->view->currentRenderPassDescriptor;

    MTL::CommandBuffer *pCmd = this->commandQueue->commandBuffer();
    MTL::RenderCommandEncoder *pEnc = pCmd->renderCommandEncoder(pRpd);
    pEnc->endEncoding();
    pCmd->presentDrawable(this->view->currentDrawable);
    pCmd->commit();

    pPool->release();
}