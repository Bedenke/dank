#pragma once

// #include "DXFoundation.hpp"
#include "DXFoundation.hpp"
#include "modules/FrameContext.hpp"
#include "modules/renderer/Renderer.hpp"
#include "modules/scene/Scene.hpp"


namespace dank {
namespace dx {

class DXRenderer : public Renderer {
private:
  static const int QUEUE_SLOT_COUNT = 3;

  D3D12_VIEWPORT viewport;
  D3D12_RECT rectScissor;

  ComPtr<IDXGIFactory4> factory;
  ComPtr<IDXGIAdapter1> warpAdapter = NULL;
  ComPtr<ID3D12Device> device;
  ComPtr<ID3D12CommandQueue> commandQueue;
  ComPtr<IDXGISwapChain1> swapChain;
  ComPtr<ID3D12RootSignature> rootSignature;

  ComPtr<ID3DBlob> vertexShader;
  ComPtr<ID3DBlob> pixelShader;
  D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
  D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
  ComPtr<ID3D12PipelineState> renderPipelineState;

  int currentBackBuffer = 0;
  UINT renderTargetViewDescriptorSize;
  int GetQueueSlot() const { return currentBackBuffer; }

  ComPtr<ID3D12CommandAllocator> commandAllocators[QUEUE_SLOT_COUNT];
  ComPtr<ID3D12GraphicsCommandList> commandLists[QUEUE_SLOT_COUNT];
  ComPtr<ID3D12Resource> renderTargets[QUEUE_SLOT_COUNT];
  HANDLE frameFenceEvents[QUEUE_SLOT_COUNT];
  ComPtr<ID3D12Fence> frameFences[QUEUE_SLOT_COUNT];
  UINT64 currentFenceValue;
  UINT64 fenceValues[QUEUE_SLOT_COUNT];

  ComPtr<ID3D12DescriptorHeap> renderTargetDescriptorHeap;

  HWND outputWindow;
  int width;
  int height;

  void initViewport();
  void initDevice();
  void initCommandQueue();
  void initCommandLists();
  void initSwapChain();
  void initFences();
  void initRenderTargets();
  void initRootSignature();
  void initShaderLibrary();
  void initGraphicsPipeline();
  void initUploadCommandList();
  void initBuffers(ComPtr<ID3D12GraphicsCommandList> uploadCommandList);

  void renderCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
  void renderFrame(FrameContext &ctx, Scene *scene);

  ComPtr<ID3D12Resource> uploadBuffer_;

  ComPtr<ID3D12Resource> vertexBuffer_;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

  ComPtr<ID3D12Resource> indexBuffer_;
  D3D12_INDEX_BUFFER_VIEW indexBufferView_;

  ComPtr<ID3D12Debug> debugController;

public:
  void init(HWND outputWindow, int width, int height);
  void render(FrameContext &ctx, Scene *scene) override;
  void release();
};

namespace GraphicsPipeline {
const D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = {
    FALSE,                      // DepthEnable
    D3D12_DEPTH_WRITE_MASK_ALL, // DepthWriteMask
    D3D12_COMPARISON_FUNC_LESS, // DepthFunc
    FALSE,                      // StencilEnable
    0,                          // StencilReadMask
    0,                          // StencilWriteMask
    {
        // FrontFace
        D3D12_STENCIL_OP_KEEP,       // StencilFailOp
        D3D12_STENCIL_OP_KEEP,       // StencilDepthFailOp
        D3D12_STENCIL_OP_KEEP,       // StencilPassOp
        D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
    },
    {
        // BackFace
        D3D12_STENCIL_OP_KEEP,       // StencilFailOp
        D3D12_STENCIL_OP_KEEP,       // StencilDepthFailOp
        D3D12_STENCIL_OP_KEEP,       // StencilPassOp
        D3D12_COMPARISON_FUNC_ALWAYS // StencilFunc
    },
};

const D3D12_RASTERIZER_DESC RasterizerDesc = {
    D3D12_FILL_MODE_SOLID,                    // FillMode
    D3D12_CULL_MODE_BACK,                     // CullMode
    FALSE,                                    // FrontCounterClockwise
    D3D12_DEFAULT_DEPTH_BIAS,                 // DepthBias
    D3D12_DEFAULT_DEPTH_BIAS_CLAMP,           // DepthBiasClamp
    D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,    // SlopeScaledDepthBias
    TRUE,                                     // DepthClipEnable
    FALSE,                                    // MultisampleEnable
    FALSE,                                    // AntialiasedLineEnable
    0,                                        // ForcedSampleCount
    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF // ConservativeRaster
};

const D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc = {
    TRUE,                         // BlendEnable
    FALSE,                        // LogicOpEnable
    D3D12_BLEND_ONE,              // SrcBlend
    D3D12_BLEND_ZERO,             // DestBlend
    D3D12_BLEND_OP_ADD,           // BlendOp
    D3D12_BLEND_ONE,              // SrcBlendAlpha
    D3D12_BLEND_ZERO,             // DestBlendAlpha
    D3D12_BLEND_OP_ADD,           // BlendOpAlpha
    D3D12_LOGIC_OP_NOOP,          // LogicOp
    D3D12_COLOR_WRITE_ENABLE_ALL, // RenderTargetWriteMask
};

const D3D12_BLEND_DESC BlendDesc = {
    FALSE,
    FALSE,
    {RenderTargetBlendDesc, RenderTargetBlendDesc, RenderTargetBlendDesc,
     RenderTargetBlendDesc, RenderTargetBlendDesc, RenderTargetBlendDesc,
     RenderTargetBlendDesc, RenderTargetBlendDesc}};
}; // namespace GraphicsPipeline

} // namespace dx
} // namespace dank