#include "yaGraphicDevice_DX12.h"
#include "yaApplication.h"



extern ya::Application application;
namespace ya::graphics
{
	GraphicDevice_DX12::GraphicDevice_DX12()
		: mbUseWarpDevice(false)
		, mFrameIndex(0)
		, mRtvDescriptorSize(0)
	{
		GetDevice() = this;
		// Initialize the DirectX 12 device here
		// This is a placeholder, actual implementation will depend on your requirements
		if (!(CreateDevice()))
			assert(NULL && "Create Device Failed!");
	}

	GraphicDevice_DX12::~GraphicDevice_DX12()
	{

	}

	bool GraphicDevice_DX12::CreateDevice()
	{
		UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
#endif
		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory))))
			assert(NULL && "Create DXGI Factory Failed!");

		if (mbUseWarpDevice)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
			if (FAILED(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))))
				assert(NULL && "Enum Warp Adapter Failed!");

			if (FAILED(D3D12CreateDevice(
				warpAdapter.Get()
				, D3D_FEATURE_LEVEL_11_0
				, IID_PPV_ARGS(&mDevice))))
				assert(NULL && "Create Device with Warp Adapter Failed!");
		}
		else
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(mFactory.Get(), &hardwareAdapter);

			if (FAILED(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&mDevice))))
				assert(NULL && "Create Device with Hardware Adapter Failed!");
		}

		return true;
	}

	// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
	// If no such adapter can be found, *ppAdapter will be set to nullptr.
	void GraphicDevice_DX12::GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

		Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter)));
					++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		if (adapter.Get() == nullptr)
		{
			for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}

	void GraphicDevice_DX12::Initialize()
	{
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		if (FAILED(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue))))
			assert(NULL && "Create Command Queue Failed!");

		// Decribe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 2; // Double buffering maybe upgrade 3buffering later multithread rendering
		swapChainDesc.Width = application.GetWindow().GetWidth();
		swapChainDesc.Height = application.GetWindow().GetHeight();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1; // No MSAA
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.Stereo = FALSE;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
		HWND hwnd = application.GetWindow().GetHwnd(); // Get the window handle
		if (FAILED(mFactory->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			hwnd,
			&swapChainDesc,
			nullptr, nullptr,
			&swapChain)))
		{
			assert(NULL && "Create Swap Chain Failed!");
		}

		// This sample does not support fullscreen transitions.
		if (FAILED(mFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)))
			assert(NULL && "Make Window Association Failed!");

		if (FAILED(swapChain.As(&mSwapChain)))
			assert(NULL && "Swap Chain As Failed!");

		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps
		// Rtv Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 2; // Double buffering
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap))))
			assert(NULL && "Create RTV Heap Failed!"); \

		mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		
		// Create a RTV for each frame.
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < 2; i++)
		{
			if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])))) // Added parentheses around the expression
				assert(NULL && "Get Swap Chain Buffer Failed!");

			mRenderTragetDesciptorHandle[i] = rtvHandle;
			mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, mRtvDescriptorSize);
		}

		// Create the command allocator for the current frame
		//if (FAILED(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator))))
		//	assert(NULL && "Create Command Allocator Failed!");

		for (size_t i = 0; i < 2; i++)
		{
			if (FAILED(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameContext[i].CommandAllocator))))
				assert(NULL && "Create Command Allocator Failed!");
		}

		// imgui
		const int APP_SRV_HEAP_SIZE = 64;
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = APP_SRV_HEAP_SIZE;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap))))
			assert(NULL && "Create SRV Heap Failed!");



		//// Create the command list.
		//if (FAILED(mDevice->CreateCommandList(
		//	0, 
		//	D3D12_COMMAND_LIST_TYPE_DIRECT, 
		//	mFrameContext[0].CommandAllocator.Get(),
		//	nullptr, 
		//	IID_PPV_ARGS(&mImguiCommandList))))
		//	assert(NULL, "CreateCommandList");
		//
		//mImguiCommandList->Close();

		///=====================Load Asset===============================//

		// create root signature
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
			assert(NULL && "SerializeRootSignature");

		if (FAILED(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))))
			assert(NULL && "CreateRootSignature");

		// load shader
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ID3DBlob* errorVSBlob = nullptr;
		D3DCompileFromFile(L"..\\Shaders_SOURCE\\TriangleVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &vertexShader, &errorVSBlob);

		ID3DBlob* errorPSBlob = nullptr;
		D3DCompileFromFile(L"..\\Shaders_SOURCE\\TrianglePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &pixelShader, &errorPSBlob);

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		
		if (FAILED(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState))))
			assert(NULL, "CreateGraphicsPipelineState");

		// Create the command list.
		if (FAILED(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameContext[0].CommandAllocator.Get(), mPipelineState.Get(), IID_PPV_ARGS(&mCommandList))))
			assert(NULL, "CreateCommandList");



		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		if (FAILED(mCommandList->Close()))
			assert(NULL, "CommandList Close");

		// Create the vertex buffer.
		{
			// Define the geometry for a triangle.
			float aspectRatio = 1.0f;
			Vertex triangleVertices[] =
			{
				{ { 0.0f, 0.25f * 1600.0f / 900.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
				{ { 0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
				{ { -0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
			};

			const UINT vertexBufferSize = sizeof(triangleVertices);

			// Note: using upload heaps to transfer static data like vert buffers is not 
			// recommended. Every time the GPU needs it, the upload heap will be marshalled 
			// over. Please read up on Default Heap usage. An upload heap is used here for 
			// code simplicity and because there are very few verts to actually transfer.
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

			if (FAILED(mDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mVertexBuffer))))
				assert(NULL, "CreateCommittedResource");

			// Copy the triangle data to the vertex buffer.
			UINT8* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
			mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
			memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
			mVertexBuffer->Unmap(0, nullptr);

			// Initialize the vertex buffer view.
			mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
			mVertexBufferView.StrideInBytes = sizeof(Vertex);
			mVertexBufferView.SizeInBytes = vertexBufferSize;
		}

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			if (FAILED(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))))
				assert(NULL, "CreateFence");
			mFenceValue = 1;

			// Create an event handle to use for frame synchronization.
			mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (mFenceEvent == nullptr)
			{
				HRESULT_FROM_WIN32(GetLastError());
				assert(NULL, "Create Fence Event");
			}

			// Wait for the command list to execute; we are reusing the same command 
			// list in our main loop but for now, we just want to wait for setup to 
			// complete before continuing.
			WaitForPreviousFrame();
		}
	}

	void GraphicDevice_DX12::WaitForPreviousFrame()
	{
		// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
		// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
		// sample illustrates how to use fences for efficient resource usage and to
		// maximize GPU utilization.

		// Signal and increment the fence value.
		const UINT64 fence = mFenceValue;
		if (FAILED(mCommandQueue->Signal(mFence.Get(), fence)))
			assert(NULL, "mCommandQueue->Signal");
		mFenceValue++;

		// Wait until the previous frame is finished.
		if (mFence->GetCompletedValue() < fence)
		{
			if (FAILED(mFence->SetEventOnCompletion(fence, mFenceEvent)))
				assert(NULL, "SetEventOnCompletion");
			WaitForSingleObject(mFenceEvent, INFINITE);
		}

		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
	}

	FrameContext* GraphicDevice_DX12::WaitForNextFrameResources()
	{
		UINT nextFrameIndex = mFrameIndex + 1;
		mFrameIndex = nextFrameIndex;

		HANDLE waitableObjects[] = { mSwapChain->GetFrameLatencyWaitableObject(), nullptr};
		DWORD numWaitableObjects = 1;

		FrameContext* frameCtx = &mFrameContext[nextFrameIndex % 2];
		UINT64 fenceValue = frameCtx->FenceValue;
		if (fenceValue != 0) // means no fence was signaled
		{
			frameCtx->FenceValue = 0;
			mFence->SetEventOnCompletion(fenceValue, mFenceEvent);
			waitableObjects[1] = mFenceEvent;
			numWaitableObjects = 2;
		}

		WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

		return frameCtx;
	}

	void GraphicDevice_DX12::PopulateCommandList()
	{
		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		//if (FAILED(mCommandAllocator->Reset()))
		//	assert(NULL, "mCommandAllocator->Reset()");
		
		
		if (FAILED(mFrameContext[mFrameIndex].CommandAllocator->Reset()))
			assert(NULL, "mCommandAllocator->Reset()");

		// However, when ExecuteCommandList() is called on a particular command 
		// list, that command list can then be reset at any time and must be before 
		// re-recording.
		if (FAILED(mCommandList->Reset(mFrameContext[mFrameIndex].CommandAllocator.Get(), mPipelineState.Get())))
			assert(NULL, "mCommandList->Reset()");

		// Set necessary state.
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		int width = application.GetWindow().GetWidth();
		int height = application.GetWindow().GetHeight();
		CD3DX12_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		CD3DX12_RECT scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));

		mCommandList->RSSetViewports(1, &viewport);
		mCommandList->RSSetScissorRects(1, &scissorRect);

		// Indicate that the back buffer will be used as a render target.
		CD3DX12_RESOURCE_BARRIER resourceBarrierPR
			= CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mCommandList->ResourceBarrier(1, &resourceBarrierPR);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRtvDescriptorSize);
		mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
		mCommandList->DrawInstanced(3, 1, 0, 0);

		// Indicate that the back buffer will now be used to present.
		CD3DX12_RESOURCE_BARRIER resourceBarrierRT 
			= CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		// now this code is commented out because it is not needed for the current rendering flow
		// that is because we are not using the command allocator for imgui rendering
		//mCommandList->ResourceBarrier(1, &resourceBarrierRT);
		//if (FAILED(mCommandList->Close()))
		//	assert(NULL, "mCommandList->Close()");
	}

	void GraphicDevice_DX12::ExcuteCommandList()
	{
		ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	void GraphicDevice_DX12::Render()
	{
		// Record all the commands we need to render the scene into the command list.
		PopulateCommandList();
	}

	void GraphicDevice_DX12::CloseCommandList()
	{
		if (FAILED(mCommandList->Close()))
			assert(NULL, "mCommandList->Close()");
	}

	void GraphicDevice_DX12::Present()
	{
		mSwapChain->Present(1, 0);
		WaitForPreviousFrame();
	}
}
