#include "DXRenderer.hpp"
#include "modules/engine/Console.hpp"
#include <d3dcompiler.h>

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
  this->outputWindow = outputWindow;
  this->width = width;
  this->height = height;

  initViewport();
  initDevice();
  initCommandQueue();
  initCommandLists();
  initSwapChain();
  initFences();
  initRenderTargets();
  initRootSignature();
  initShaderLibrary();
  initUploadCommandList();
  initGraphicsPipeline();

  dank::console::log("DXRenderer initialized");
}

void DXRenderer::initViewport() {
  rectScissor = {0, 0, width, height};

  viewport = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
              0.0f, 1.0f};
}
void DXRenderer::initDevice() {
  // Initialize DirectX 12
  UINT factoryFlags = 0;

  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    debugController->EnableDebugLayer();
    factoryFlags = factoryFlags | DXGI_CREATE_FACTORY_DEBUG;
    dank::console::log("DX12 debug enabled");
  }

  ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)),
                "CreateDXGIFactory2");

  ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)),
                "EnumWarpAdapter");

  ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                  IID_PPV_ARGS(&device)),
                "D3D12CreateDevice");
}
void DXRenderer::initCommandQueue() {

  // Create the command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ThrowIfFailed(
      device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)),
      "CreateCommandQueue");
}

void DXRenderer::initCommandLists() {
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                   IID_PPV_ARGS(&commandAllocators[i]));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                              commandAllocators[i].Get(), nullptr,
                              IID_PPV_ARGS(&commandLists[i]));
    commandLists[i]->Close();
  }
}

void DXRenderer::initSwapChain() {

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
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
  }
}

void DXRenderer::initFences() {

  currentFenceValue = 1;

  // Create fences for each frame so we can protect resources and wait for
  // any given frame
  for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
    frameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    fenceValues[i] = 0;
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                        IID_PPV_ARGS(&frameFences[i]));
  }
}

void DXRenderer::initRenderTargets() {
  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors = QUEUE_SLOT_COUNT;
  heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  device->CreateDescriptorHeap(&heapDesc,
                               IID_PPV_ARGS(&renderTargetDescriptorHeap));

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
      renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

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
}

void DXRenderer::initRootSignature() {
  // We have two root parameters, one is a pointer to a descriptor heap
  // with a SRV, the second is a constant buffer view
  CD3DX12_ROOT_PARAMETER parameters[1];

  // Our constant buffer view
  parameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

  CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;

  // Create the root signature
  descRootSignature.Init(
      1, parameters, 0, nullptr,
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> rootBlob;
  ComPtr<ID3DBlob> errorBlob;
  ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature,
                                            D3D_ROOT_SIGNATURE_VERSION_1,
                                            &rootBlob, &errorBlob),
                "D3D12SerializeRootSignature");

  ThrowIfFailed(device->CreateRootSignature(0, rootBlob->GetBufferPointer(),
                                            rootBlob->GetBufferSize(),
                                            IID_PPV_ARGS(&rootSignature)),
                "CreateRootSignature");
}

void DXRenderer::initShaderLibrary() {
  // Load precompiled shader bytecode
  ThrowIfFailed(D3DReadFileToBlob(L"VS_main.cso", &vertexShader),
                "D3DReadFileToBlob VS_main.cso");
  ThrowIfFailed(D3DReadFileToBlob(L"PS_main.cso", &pixelShader),
                "D3DReadFileToBlob PS_main.cso");

  // Create shader bytecode structures
  vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
  vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();

  pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
  pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
}

void DXRenderer::initGraphicsPipeline() {
  static const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
  psoDesc.pRootSignature = rootSignature.Get();
  psoDesc.VS = vertexShaderBytecode;
  psoDesc.PS = pixelShaderBytecode;
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.DepthStencilState.DepthEnable = FALSE;
  psoDesc.DepthStencilState.StencilEnable = FALSE;
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDesc.SampleDesc.Count = 1;
  ThrowIfFailed(device->CreateGraphicsPipelineState(
                    &psoDesc, IID_PPV_ARGS(&renderPipelineState)),
                "CreateGraphicsPipelineState");

  // D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};
  // pipelineDesc.VS = vertexShaderBytecode;
  // pipelineDesc.PS = pixelShaderBytecode;
  // pipelineDesc.pRootSignature = rootSignature.Get();
  // pipelineDesc.NumRenderTargets = 1;
  // pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  // pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
  // pipelineDesc.InputLayout.NumElements =
  // std::extent<decltype(layout)>::value;
  // pipelineDesc.InputLayout.pInputElementDescs = layout;
  // pipelineDesc.BlendState = GraphicsPipeline::BlendDesc;
  // pipelineDesc.SampleMask = UINT_MAX;
  // pipelineDesc.RasterizerState = GraphicsPipeline::RasterizerDesc;
  // pipelineDesc.DepthStencilState = GraphicsPipeline::DepthStencilDesc;
  // pipelineDesc.InputLayout = {};
  // pipelineDesc.PrimitiveTopologyType =
  // D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; pipelineDesc.SampleDesc = {1};
  // pipelineDesc.SampleMask = 0xFFFFFFFF;

  // ThrowIfFailed(device->CreateGraphicsPipelineState(
  //                   &pipelineDesc, IID_PPV_ARGS(&renderPipelineState)),
  //               "CreateGraphicsPipelineState");
}

void DXRenderer::initUploadCommandList() {

  ComPtr<ID3D12Fence> uploadFence;
  device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

  ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
  device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 IID_PPV_ARGS(&uploadCommandAllocator));
  ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
  device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                            uploadCommandAllocator.Get(), nullptr,
                            IID_PPV_ARGS(&uploadCommandList));

  initBuffers(uploadCommandList.Get());

  uploadCommandList->Close();

  // Execute the upload and finish the command list
  ID3D12CommandList *commandLists[] = {uploadCommandList.Get()};
  commandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value,
                                    commandLists);
  commandQueue->Signal(uploadFence.Get(), 1);

  auto waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  if (waitEvent == NULL) {
    throw std::runtime_error("Could not create wait event.");
  }

  WaitForFence(uploadFence.Get(), 1, waitEvent);

  // Cleanup our upload handle
  uploadCommandAllocator->Reset();

  CloseHandle(waitEvent);
}

void DXRenderer::initBuffers(
    ComPtr<ID3D12GraphicsCommandList> uploadCommandList) {
  struct Vertex {
    float position[3];
    float uv[2];
  };

  // Declare upload buffer data as 'static' so it persists after returning from
  // this function. Otherwise, we would need to explicitly wait for the GPU to
  // copy data from the upload buffer to vertex/index default buffers due to how
  // the GPU processes commands asynchronously.
  static const Vertex vertices[4] = {// Upper Left
                                     {{-1.0f, 1.0f, 0}, {0, 0}},
                                     // Upper Right
                                     {{1.0f, 1.0f, 0}, {1, 0}},
                                     // Bottom right
                                     {{1.0f, -1.0f, 0}, {1, 1}},
                                     // Bottom left
                                     {{-1.0f, -1.0f, 0}, {0, 1}}};

  static const int indices[6] = {0, 1, 2, 2, 3, 0};

  static const int uploadBufferSize = sizeof(vertices) + sizeof(indices);
  static const auto uploadHeapProperties =
      CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  static const auto uploadBufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

  // Create upload buffer on CPU
  device->CreateCommittedResource(
      &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer_));

  // Create vertex & index buffer on the GPU
  // HEAP_TYPE_DEFAULT is on GPU, we also initialize with COPY_DEST state
  // so we don't have to transition into this before copying into them
  static const auto defaultHeapProperties =
      CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

  static const auto vertexBufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
  device->CreateCommittedResource(
      &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer_));

  static const auto indexBufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
  device->CreateCommittedResource(
      &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc,
      D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&indexBuffer_));

  // Create buffer views
  vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
  vertexBufferView_.SizeInBytes = sizeof(vertices);
  vertexBufferView_.StrideInBytes = sizeof(Vertex);

  indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
  indexBufferView_.SizeInBytes = sizeof(indices);
  indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

  // Copy data on CPU into the upload buffer
  void *p;
  uploadBuffer_->Map(0, nullptr, &p);
  ::memcpy(p, vertices, sizeof(vertices));
  ::memcpy(static_cast<unsigned char *>(p) + sizeof(vertices), indices,
           sizeof(indices));
  uploadBuffer_->Unmap(0, nullptr);

  // Copy data from upload buffer on CPU into the index/vertex buffer on
  // the GPU
  uploadCommandList->CopyBufferRegion(vertexBuffer_.Get(), 0,
                                      uploadBuffer_.Get(), 0, sizeof(vertices));
  uploadCommandList->CopyBufferRegion(indexBuffer_.Get(), 0,
                                      uploadBuffer_.Get(), sizeof(vertices),
                                      sizeof(indices));

  // Barriers, batch them together
  const CD3DX12_RESOURCE_BARRIER barriers[2] = {
      CD3DX12_RESOURCE_BARRIER::Transition(
          vertexBuffer_.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
          D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
      CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer_.Get(),
                                           D3D12_RESOURCE_STATE_COPY_DEST,
                                           D3D12_RESOURCE_STATE_INDEX_BUFFER)};

  uploadCommandList->ResourceBarrier(2, barriers);
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
  // Set our root signature
  commandList->SetGraphicsRootSignature(rootSignature.Get());
  commandList->SetPipelineState(renderPipelineState.Get());

  renderCommandList(commandList);

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

void DXRenderer::renderCommandList(
    ComPtr<ID3D12GraphicsCommandList> commandList) {
  commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
  commandList->IASetIndexBuffer(&indexBufferView_);
  commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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