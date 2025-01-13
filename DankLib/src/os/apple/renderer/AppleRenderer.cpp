#include "AppleRenderer.hpp"
#include "modules/engine/Console.hpp"
#include <simd/simd.h>

void dank::apple::AppleRenderer::init()
{
    this->commandQueue = this->view->device->newCommandQueue();

    {
        MTL::Function *pVertexFn = this->view->shaderLibrary->newFunction(NS::String::string("vertexMain", NS::StringEncoding::UTF8StringEncoding));
        MTL::Function *pFragFn = this->view->shaderLibrary->newFunction(NS::String::string("fragmentMain", NS::StringEncoding::UTF8StringEncoding));

        MTL::RenderPipelineDescriptor *pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
        pDesc->setVertexFunction(pVertexFn);
        pDesc->setFragmentFunction(pFragFn);
        pDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

        NS::Error *pError;
        pipelineState = view->device->newRenderPipelineState(pDesc, &pError);
        if (!pipelineState)
        {
            dank::console::log("%s", pError->localizedDescription()->utf8String());
            assert(false);
        }

        pVertexFn->release();
        pFragFn->release();
        pDesc->release();
    }

    {
        const size_t NumVertices = 3;

        simd::float3 positions[NumVertices] =
            {
                {-0.8f, 0.8f, 0.0f},
                {0.0f, -0.8f, 0.0f},
                {+0.8f, 0.8f, 0.0f}};

        simd::float3 colors[NumVertices] =
            {
                {1.0f, 0.0f, 0.0f},
                {0.0f, 1.0, 0.0f},
                {0.0f, 0.0f, 1.0}};

        const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
        const size_t colorDataSize = NumVertices * sizeof(simd::float3);

        MTL::Buffer *pVertexPositionsBuffer = view->device->newBuffer(positionsDataSize, MTL::ResourceStorageModeShared);
        MTL::Buffer *pVertexColorsBuffer = view->device->newBuffer(colorDataSize, MTL::ResourceStorageModeShared);

        _pVertexPositionsBuffer = pVertexPositionsBuffer;
        _pVertexColorsBuffer = pVertexColorsBuffer;

        memcpy(_pVertexPositionsBuffer->contents(), positions, positionsDataSize);
        memcpy(_pVertexColorsBuffer->contents(), colors, colorDataSize);
    }
}

void dank::apple::AppleRenderer::clear()
{
    // Clear the screen

    NS::AutoreleasePool *pPool = NS::AutoreleasePool::alloc()->init();

    MTL::RenderPassDescriptor *pRpd = this->view->currentRenderPassDescriptor;

    MTL::CommandBuffer *pCmd = this->commandQueue->commandBuffer();
    MTL::RenderCommandEncoder *pEnc = pCmd->renderCommandEncoder(pRpd);

    pEnc->setRenderPipelineState(this->pipelineState);
    pEnc->setVertexBuffer(_pVertexPositionsBuffer, 0, 0);
    pEnc->setVertexBuffer(_pVertexColorsBuffer, 0, 1);
    pEnc->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));

    pEnc->endEncoding();
    pCmd->presentDrawable(this->view->currentDrawable);
    pCmd->commit();

    pPool->release();
}