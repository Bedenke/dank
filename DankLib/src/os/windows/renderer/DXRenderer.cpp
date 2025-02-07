#include "DXRenderer.hpp"
#include "DXFoundation.hpp"
#include "d3dx12.h"
#include "modules/engine/Console.hpp"

using namespace dank;
using namespace dank::dx;

static const size_t maxDescriptorsCount = 100; // TODO: Learn and adjust this

namespace {
void WaitForFence(ID3D12Fence *fence, UINT64 completionValue,
                  HANDLE waitEvent) {
  if (fence->GetCompletedValue() < completionValue) {
    fence->SetEventOnCompletion(completionValue, waitEvent);
    WaitForSingleObject(waitEvent, INFINITE);
  }
}
} // namespace

void DXRenderer::init(HWND outputWindow, int width, int height) {
  rectScissor = {0, 0, width, height};

  viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
              0.0f, 1.0f};

  // Initialize DirectX 12
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)),
                "CreateDXGIFactory1");

  ThrowIfFailed(factory->EnumWarpAdapter(IID_IDXGIAdapter1, &warpAdapter),
                "EnumWarpAdapter");

  // Create the device
  ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                  IID_PPV_ARGS(&device)),
                "D3D12CreateDevice");

  // Create the command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ThrowIfFailed(
      device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)),
      "CreateCommandQueue");

  // Create the swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDescWindowed = {};
  swapChainDescWindowed.BufferCount = QUEUE_SLOT_COUNT;
  swapChainDescWindowed.Width = width;
  swapChainDescWindowed.Height = height;
  swapChainDescWindowed.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDescWindowed.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDescWindowed.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
  swapChainDescWindowed.SampleDesc.Count = 1;
  swapChainDescWindowed.SampleDesc.Quality = 0;
  swapChainDescWindowed.Scaling = DXGI_SCALING_STRETCH;
  swapChainDescWindowed.Stereo = FALSE;
  swapChainDescWindowed.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainDescFullscreen = {};
  ThrowIfFailed(factory->CreateSwapChainForHwnd(
                    commandQueue.Get(), outputWindow, &swapChainDescWindowed,
                    &swapChainDescFullscreen, nullptr, &swapChain),
                "CreateSwapChainForHwnd");

  renderTargetViewDescriptorSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  currentFenceValue = 1;

  // Create fences for each frame so we can protect resources and wait for
  // any given frame
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    fenceValues[i] = 0;
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                        IID_PPV_ARGS(&frameFences[i]));
  }

  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
  }

  // Render targets
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = QUEUE_SLOT_COUNT;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  device->CreateDescriptorHeap(&heapDesc,
                               IID_PPV_ARGS(&renderTargetDescriptorHeap));

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{
      renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};

  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipSlice = 0;
    viewDesc.Texture2D.PlaneSlice = 0;

    device->CreateRenderTargetView(renderTargets[i].Get(), &viewDesc,
                                   rtvHandle);

    rtvHandle.Offset(renderTargetViewDescriptorSize);
  }

  // Add root signature
  D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
  ComPtr<ID3DBlob> signatureBlob = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;
  ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
                                            D3D_ROOT_SIGNATURE_VERSION_1,
                                            &signatureBlob, &errorBlob),
                "D3D12SerializeRootSignature");

  ThrowIfFailed(device->CreateRootSignature(0,
                                            signatureBlob->GetBufferPointer(),
                                            signatureBlob->GetBufferSize(),
                                            IID_PPV_ARGS(&rootSignature)),
                "CreateRootSignature");

  // Add graphics pipeline
  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
  pipelineDesc.pRootSignature = rootSignature.Get();
  pipelineDesc.BlendState = GraphicsPipeline::BlendDesc;
  pipelineDesc.SampleMask = UINT_MAX;
  pipelineDesc.RasterizerState = GraphicsPipeline::RasterizerDesc;
  // pipelineDesc.DepthStencilState = GraphicsPipeline::DepthStencilDesc;
  pipelineDesc.InputLayout = {};
  pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pipelineDesc.SampleDesc = {1};

  //   ThrowIfFailed(device->CreateGraphicsPipelineState(
  //                     &pipelineDesc, IID_PPV_ARGS(&renderPipelineState)),
  //                 "CreateGraphicsPipelineState");

  // Add command lists
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                   IID_PPV_ARGS(&commandAllocators[i]));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                              commandAllocators[i].Get(), nullptr,
                              IID_PPV_ARGS(&commandLists[i]));
    commandLists[i]->Close();
  }

  dank::console::log("DXRenderer initialized");
}

void DXRenderer::render(FrameContext &ctx, Scene *scene) {
  WaitForFence(frameFences[GetQueueSlot()].Get(), fenceValues[GetQueueSlot()],
               frameFenceEvents[GetQueueSlot()]);

  renderFrame(ctx, scene);
}

void DXRenderer::renderFrame(FrameContext &ctx, Scene *scene) {
  // Prepare render
  ThrowIfFailed(commandAllocators[currentBackBuffer]->Reset(),
                "commandAllocators[currentBackBuffer]->Reset()");

  auto commandList = commandLists[currentBackBuffer].Get();
  commandList->Reset(commandAllocators[currentBackBuffer].Get(), nullptr);

  D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
  CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(
      renderTargetHandle,
      renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
      currentBackBuffer, renderTargetViewDescriptorSize);

  commandList->OMSetRenderTargets(1, &renderTargetHandle, true, nullptr);
  commandList->RSSetViewports(1, &viewport);
  commandList->RSSetScissorRects(1, &rectScissor);

  // Transition back buffer
  {
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Transition.pResource = renderTargets[currentBackBuffer].Get();
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    commandList->ResourceBarrier(1, &barrier);

    static const float clearColor[] = {1.0f, 0.042f, 0.042f, 1};

    commandList->ClearRenderTargetView(renderTargetHandle, clearColor, 0,
                                       nullptr);
  }

  // Render
  // Set our state (shaders, etc.)
  //   commandList->SetPipelineState(renderPipelineState.Get());

  // Set our root signature
  commandList->SetGraphicsRootSignature(rootSignature.Get());

  // TODO: actual drawing

  // Finalize
  {
    // Transition the swap chain back to present
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Transition.pResource = renderTargets[currentBackBuffer].Get();
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    auto commandList = commandLists[currentBackBuffer].Get();
    commandList->ResourceBarrier(1, &barrier);

    commandList->Close();

    // Execute our commands
    ID3D12CommandList *commandLists[] = {commandList};
    commandQueue->ExecuteCommandLists(
        std::extent<decltype(commandLists)>::value, commandLists);
  }

  // Present
  swapChain->Present(1, 0);

  // Mark the fence for the current frame.
  const auto fenceValue = currentFenceValue;
  commandQueue->Signal(frameFences[currentBackBuffer].Get(), fenceValue);
  fenceValues[currentBackBuffer] = fenceValue;
  ++currentFenceValue;

  // Take the next back buffer from our chain
  currentBackBuffer = (currentBackBuffer + 1) % QUEUE_SLOT_COUNT;
}

void DXRenderer::release() {
  // Drain the queue, wait for everything to finish
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    WaitForFence(frameFences[i].Get(), fenceValues[i], frameFenceEvents[i]);
  }

  for (auto event : frameFenceEvents) {
    CloseHandle(event);
  }
}