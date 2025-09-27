// Use the WRL smart pointers for COM objects

#include <dxgi.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "Shaders.h"
#include "Vertex.h"
using Microsoft::WRL::ComPtr;


#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

// Constants
const int WIDTH = 800;
const int HEIGHT = 600;

static const int APP_SRV_HEAP_SIZE = 64;


struct ExampleDescriptorHeapAllocator
{
	ID3D12DescriptorHeap* Heap = nullptr;
	D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
	UINT                        HeapHandleIncrement;
	ImVector<int>               FreeIndices;

	void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
	{
		IM_ASSERT(Heap == nullptr && FreeIndices.empty());
		Heap = heap;
		D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
		HeapType = desc.Type;
		HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
		HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
		HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
		FreeIndices.reserve((int)desc.NumDescriptors);
		for (int n = desc.NumDescriptors; n > 0; n--)
			FreeIndices.push_back(n - 1);
	}
	void Destroy()
	{
		Heap = nullptr;
		FreeIndices.clear();
	}
	void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
	{
		IM_ASSERT(FreeIndices.Size > 0);
		int idx = FreeIndices.back();
		FreeIndices.pop_back();
		out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
		out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
	}
	void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
	{
		int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
		int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
		IM_ASSERT(cpu_idx == gpu_idx);
		FreeIndices.push_back(cpu_idx);
	}
};


static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;

// Globals
ComPtr<ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
ComPtr<ID3D12DescriptorHeap> srvHeap;
ComPtr<ID3D12Resource> renderTargets[2];
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT64 fenceValue;
int rtvDescriptorSize;


ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12RootSignature> rootSignature;
ComPtr<ID3DBlob> vertexShader;
ComPtr<ID3DBlob> pixelShader;

ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


// Forward declarations
void InitD3D(HWND hWnd);
void Cleanup();
void Update();
void Render();
void WaitForPreviousFrame();

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZING:
		RECT x;
		GetWindowRect(hWnd, &x);
		int width = x.right - x.left;
		int height = x.bottom - x.top;
		std::cout << width << " " << height << std::endl;
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
int main(int argc, char** argv) {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	HINSTANCE hInstance;
	hInstance = GetModuleHandle(nullptr);




	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L,
					 hInstance, nullptr, nullptr, nullptr, nullptr,
					 L"DX12 Window", nullptr };
	RegisterClassEx(&wc);

	// Create window
	HWND hWnd = CreateWindow(wc.lpszClassName, L"DirectX 12 App",
		WS_OVERLAPPEDWINDOW, 100, 100, WIDTH, HEIGHT,
		nullptr, nullptr, hInstance, nullptr);

	// Initialize Direct3D
	InitD3D(hWnd);

	// Show the window
	ShowWindow(hWnd, SW_SHOW);

	ImGui_ImplWin32_Init(hWnd);

	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device= device.Get();
	init_info.CommandQueue = commandQueue.Get();
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
	init_info.NumFramesInFlight = 2;
	init_info.SrvDescriptorHeap = srvHeap.Get();
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };

	ImGui_ImplDX12_Init(&init_info);


	// Main loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update();
			Render();
		}
	}

	// Cleanup
	Cleanup();
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}
// Entry point


void InitD3D(HWND hWnd)
{
	// Create device
	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	ComPtr<IDXGIAdapter1> adapter;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
			break;
	}

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Width = WIDTH;
	swapChainDesc.Height = HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> tempSwapChain;
	factory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&tempSwapChain);

	tempSwapChain.As(&swapChain);

	// Create descriptor heap for render target views
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create frame resources
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < 2; i++)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = APP_SRV_HEAP_SIZE;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&srvHeap)) != S_OK)
			return;
		g_pd3dSrvDescHeapAlloc.Create(device.Get(), srvHeap.Get());
	}

	// Create command allocator and command list
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	commandList->Close();

	// Create synchronization objects
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	fenceValue = 1;
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);


	vertexShader = CompileShader(L"simple.vs.hlsl", "VSMain", "vs_5_0");
	pixelShader = CompileShader(L"simple.fs.hlsl", "PSMain", "ps_5_0");

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	// Define the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));


	// Define triangle vertices
	Vertex triangleVertices[] =
	{
		{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	// Create vertex buffer
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer));

	// Copy the triangle data to the vertex buffer
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0); // We don't intend to read from this resource on the CPU
	vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;


}

void Cleanup()
{
	WaitForPreviousFrame();
	CloseHandle(fenceEvent);
}

void Update()
{
	
	// Update game/app logic here
}

void Render()
{

	
	// Record commands
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), pipelineState.Get());

	commandList->SetGraphicsRootSignature(rootSignature.Get());

	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, WIDTH, HEIGHT };
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);




	// Transition back buffer to render target
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargets[swapChain->GetCurrentBackBufferIndex()].Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	// Clear the render target
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		swapChain->GetCurrentBackBufferIndex(),
		rtvDescriptorSize);

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 1.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);


	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Hello, world!");
	ImGui::Text("Hello, world!");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
	ImGui::EndFrame();

	ImGui::Render();
	
	commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());


	// Transition back buffer to present
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargets[swapChain->GetCurrentBackBufferIndex()].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);



	// Execute commands
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present
	swapChain->Present(1, 0);

	WaitForPreviousFrame();
}

void WaitForPreviousFrame()
{
	// Signal and increment the fence value
	const UINT64 currentFenceValue = fenceValue;
	commandQueue->Signal(fence.Get(), currentFenceValue);
	fenceValue++;

	// Wait until previous frame is finished
	if (fence->GetCompletedValue() < currentFenceValue)
	{
		fence->SetEventOnCompletion(currentFenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}