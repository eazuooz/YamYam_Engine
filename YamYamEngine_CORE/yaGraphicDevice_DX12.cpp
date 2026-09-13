#include "yaGraphicDevice_DX12.h"
#include "yaApplication.h"
#include "yaResources.h"
#include "yaShader.h"
#include "yaTexture.h"
#include <stdexcept>


extern ya::Application application;
namespace ya::graphics
{
	GraphicDevice_DX12::GraphicDevice_DX12(bool useWarpDevice)
		: mbUseWarpDevice(useWarpDevice)
		, mFrameIndex(0)
		, mRtvDescriptorSize(0)
		, mFenceLastSignalValue(0)
	{
		GetDevice() = this;
		// Initialize the DirectX 12 device here
		// This is a placeholder, actual implementation will depend on your requirements
		if (!(CreateDevice()))
			assert(NULL && "Create Device Failed!");
	}

	GraphicDevice_DX12::~GraphicDevice_DX12()
	{
        if (mFence && mCommandQueue)
            WaitForGpu();
        mDepthBuffer.reset();
        mRetiredResources.clear();
        if (mFrameLatencyEvent) CloseHandle(mFrameLatencyEvent);
        if (mFenceEvent) CloseHandle(mFenceEvent);
        GetDevice() = nullptr;
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

        mWidth = swapChainDesc.Width;
        mHeight = swapChainDesc.Height;
        mSwapChain->SetMaximumFrameLatency(2);
        mFrameLatencyEvent = mSwapChain->GetFrameLatencyWaitableObject();

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

        // Shared by engine textures and ImGui, so their SRV slots never overlap.
        mSrvAllocator.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true);
        mOffscreenRtvAllocator.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256, false);
        mDsvAllocator.Create(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256, false);
        mDepthBuffer = std::make_unique<Texture>();
        if (!mDepthBuffer->Create(mWidth, mHeight, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            throw std::runtime_error("Failed to create back-buffer depth texture");

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
		// b0 : TransformCB (World / View / Projection)
        CD3DX12_DESCRIPTOR_RANGE textureRange;
        textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        CD3DX12_ROOT_PARAMETER rootParams[2] = {};
        rootParams[0].InitAsConstantBufferView(0); // b0: per-draw transform
        rootParams[1].InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL); // t0
        CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init(_countof(rootParams), rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> signature;
		Microsoft::WRL::ComPtr<ID3DBlob> error;
		if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
			assert(NULL && "SerializeRootSignature");

		if (FAILED(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))))
			assert(NULL && "CreateRootSignature");

		// load shader
		//Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
		//Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

//#if defined(_DEBUG)
//		// Enable better shader debugging with the graphics debugging tools.
//		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#else
//		UINT compileFlags = 0;
//#endif
//		ID3DBlob* errorVSBlob = nullptr;
//		D3DCompileFromFile(L"..\\Shaders_SOURCE\\TriangleVS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &vertexShader, &errorVSBlob);
//
//		ID3DBlob* errorPSBlob = nullptr;
//		D3DCompileFromFile(L"..\\Shaders_SOURCE\\TrianglePS.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &pixelShader, &errorPSBlob);

		//// Define the vertex input layout.
		//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		//};

		//Shader* triangleShader = Resources::Find<Shader>(L"Triangle");
		//Microsoft::WRL::ComPtr<ID3DBlob> vertexShader = triangleShader->GetVSBlob();
		//Microsoft::WRL::ComPtr<ID3DBlob> pixelShader = triangleShader->GetPSBlob();

		//// Describe and create the graphics pipeline state object (PSO).
		//D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		//psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		//psoDesc.pRootSignature = mRootSignature.Get();
		//psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		//psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		//psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState.DepthEnable = FALSE;
		//psoDesc.DepthStencilState.StencilEnable = FALSE;
		//psoDesc.SampleMask = UINT_MAX;
		//psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//psoDesc.NumRenderTargets = 1;
		//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		//psoDesc.SampleDesc.Count = 1;
		//
		//if (FAILED(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState))))
		//	assert(NULL, "CreateGraphicsPipelineState");

		// Create the command list.
		if (FAILED(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameContext[0].CommandAllocator.Get(), mPipelineState.Get(), IID_PPV_ARGS(&mCommandList))))
			assert(NULL, "CreateCommandList");



		// Command lists are created in the recording state, but there is nothing
		// to record yet. The main loop expects it to be closed, so close it now.
		if (FAILED(mCommandList->Close()))
			assert(NULL, "CommandList Close");

		// Create the vertex buffer.
		//{
		//	// Define the geometry for a triangle.
		//	float aspectRatio = 1.0f;
		//	Vertex triangleVertices[] =
		//	{
		//		{ { 0.0f, 0.25f * 1600.0f / 900.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		//		{ { 0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		//		{ { -0.25f, -0.25f * 1600.0f / 900.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		//	};

		//	const UINT vertexBufferSize = sizeof(triangleVertices);

		//	// Note: using upload heaps to transfer static data like vert buffers is not
		//	// recommended. Every time the GPU needs it, the upload heap will be marshalled
		//	// over. Please read up on Default Heap usage. An upload heap is used here for
		//	// code simplicity and because there are very few verts to actually transfer.
		//	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		//	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		//	
		//	if (FAILED(mDevice->CreateCommittedResource(
		//		&heapProps,
		//		D3D12_HEAP_FLAG_NONE,
		//		&bufferDesc,
		//		D3D12_RESOURCE_STATE_GENERIC_READ,
		//		nullptr,
		//		IID_PPV_ARGS(&mVertexBuffer))))
		//		assert(NULL, "CreateCommittedResource");

		//	// Copy the triangle data to the vertex buffer.
		//	UINT8* pVertexDataBegin;
		//	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		//	mVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
		//	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		//	mVertexBuffer->Unmap(0, nullptr);

		//	// Initialize the vertex buffer view.
		//	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
		//	mVertexBufferView.StrideInBytes = sizeof(Vertex);
		//	mVertexBufferView.SizeInBytes = vertexBufferSize;
		//}

		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			if (FAILED(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence))))
				assert(NULL, "CreateFence");
			// FenceValue starts at 0 for all contexts (0 = no pending GPU work)
			// mFenceLastSignalValue starts at 0 and is incremented monotonically

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
			// WaitForGpu();
		}
	}

	bool GraphicDevice_DX12::CreateCommittedResource(D3D12_HEAP_PROPERTIES* pHeapProperties,
		D3D12_HEAP_FLAGS HeapFlags,
		D3D12_RESOURCE_DESC* pDesc,
		D3D12_RESOURCE_STATES InitialResourceState,
		D3D12_CLEAR_VALUE* pOptimizedClearValue,
		REFIID riidResource,
		void** ppvResource)
	{
		if (FAILED(mDevice->CreateCommittedResource(
			pHeapProperties,
			HeapFlags,
			pDesc,
			InitialResourceState,
			pOptimizedClearValue,
			riidResource,
			ppvResource)))
			assert(NULL, "CreateCommittedResource");

		return true;
	}

	bool GraphicDevice_DX12::CreateVertexShader(const std::wstring& fileName, ID3DBlob** ppCode)
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ID3DBlob* errorBlob = nullptr;

		const HRESULT hr = D3DCompileFromFile((fileName + L"VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
			, "main", "vs_5_0", compileFlags, 0, ppCode, &errorBlob);

		if (errorBlob)
		{
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        return SUCCEEDED(hr);
	}

	bool GraphicDevice_DX12::CreatePixelShader(const std::wstring& fileName, ID3DBlob** ppCode)
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ID3DBlob* errorBlob = nullptr;

		const HRESULT hr = D3DCompileFromFile((fileName + L"PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
			, "main", "ps_5_0", compileFlags, 0, ppCode, &errorBlob);

		if (errorBlob)
		{
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        return SUCCEEDED(hr);
	}

	bool GraphicDevice_DX12::CreateGraphicsPipelineState(_In_  const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc/*, void** ppPipelineState*/)
	{
		if (FAILED(mDevice->CreateGraphicsPipelineState(pDesc, IID_PPV_ARGS(&mPipelineState))))
			assert(NULL, "CreateGraphicsPipelineState");

		return true;
	}

	void GraphicDevice_DX12::BindVertexBuffer(UINT StartSlot, UINT NumViews, D3D12_VERTEX_BUFFER_VIEW* pViews)
	{
		mCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
	}

	void GraphicDevice_DX12::BindViewportAndScissor(UINT width, UINT height)
	{
        if (!width) width = mWidth;
        if (!height) height = mHeight;
        mViewportWidth = width;
        mViewportHeight = height;
        CD3DX12_VIEWPORT viewport(0.0f, 0.0f, float(width), float(height));
        CD3DX12_RECT scissorRect(0, 0, LONG(width), LONG(height));
        mCommandList->RSSetViewports(1, &viewport);
        mCommandList->RSSetScissorRects(1, &scissorRect);
	}

	void GraphicDevice_DX12::BindFrameBuffer(bool clear, bool bindDepth)
	{
        const auto rtv = mRenderTragetDesciptorHandle[mFrameIndex];
        const auto dsv = mDepthBuffer->GetDSV();
        mCommandList->OMSetRenderTargets(1, &rtv, FALSE, bindDepth ? &dsv : nullptr);
        BindViewportAndScissor();
        if (clear)
        {
            const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
            mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
            if (bindDepth)
                mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        }
	}

	void GraphicDevice_DX12::SetBaseGraphicsRootSignature()
	{
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	}

	// GPU가 지금 당장 모든 작업을 끝낼 때까지 완전 대기하는 함수입니다. CPU와 GPU의 작업이 완전히 동기화됩니다.
	void GraphicDevice_DX12::WaitForGpu()
	{
		// Signal with the next monotonic fence value so the fence goes strictly upward.
		const UINT64 fence = mFenceLastSignalValue + 1;

		if (FAILED(mCommandQueue->Signal(mFence.Get(), fence)))
			assert(NULL, "CommandQueue Signal Failed!");

		if (FAILED(mFence->SetEventOnCompletion(fence, mFenceEvent)))
			assert(NULL, "SetEventOnCompletion Failed!");

		WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);

		mFenceLastSignalValue = fence;
        CollectRetiredResources();
	}

	// 이 프레임 작업 제출 후 Signal만 보내고 즉시 리턴하는 함수입니다.
	// GPU가 작업을 끝내는 시점에 대한 신호만 보내고, CPU는 계속해서 다음 프레임 작업을 준비할 수 있습니다.
	void GraphicDevice_DX12::SignalFrameCompletion()
	{
		UINT64 fenceValue = mFenceLastSignalValue + 1;
		mCommandQueue->Signal(mFence.Get(), fenceValue);
		mFenceLastSignalValue = fenceValue;

		// Store fence on the frame context that was ACTUALLY used this frame.
		// mFrameIndex here is still the index that was used during recording.
		mFrameContext[mFrameIndex].FenceValue = fenceValue;
        SealRetiredResources(fenceValue);

		// Advance to next back buffer AFTER storing the fence.
		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
	}

	// Editor 경로 전용. 프레임 시작 직전, CommandAllocator를 Reset하기 전에 반드시 호출한다.
	// ① 스왑체인이 새 프레임을 받을 준비가 됐는지
	// ② 이 인덱스의 Allocator를 GPU가 다 사용했는지
	// 두 조건을 동시에 확인하고, 모두 충족될 때까지 CPU를 블록한다.
	// mFrameIndex는 SignalFrameCompletion()에서 이미 다음 프레임 인덱스로 설정돼 있으므로
	// 이 함수에서는 절대 변경하지 않는다. (변경 시 Allocator/RenderTarget 인덱스 불일치 발생)
	FrameContext* GraphicDevice_DX12::WaitForNextFrameResources()
	{
		// 기본값: 스왑체인 레이턴시 대기 핸들만 준비. fence 슬롯은 아직 nullptr.
		HANDLE waitableObjects[] = { mFrameLatencyEvent, nullptr };
		DWORD numWaitableObjects = 1;

		// mFrameIndex 기준으로 이번 프레임에 사용할 Allocator의 컨텍스트를 가져온다.
		// FenceValue는 이 Allocator가 마지막으로 GPU에 제출됐을 때 기록해둔 fence 값이다.
		FrameContext* frameCtx = &mFrameContext[mFrameIndex];
		UINT64 fenceValue = frameCtx->FenceValue;
		if (fenceValue != 0) // 0 = 이 Allocator는 아직 GPU에 제출된 적 없음 (첫 2프레임)
		{
			frameCtx->FenceValue = 0; // 확인 완료 표시. 다음에 이 컨텍스트가 돌아왔을 때 중복 대기 방지.
			mFence->SetEventOnCompletion(fenceValue, mFenceEvent); // GPU가 fenceValue에 도달하면 이벤트 신호
			waitableObjects[1] = mFenceEvent; // fence 대기를 두 번째 핸들로 추가
			numWaitableObjects = 2;
		}

		// TRUE = 배열의 모든 핸들이 신호될 때까지 대기 (스왑체인 + fence 둘 다 충족해야 반환)
		WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

		CollectRetiredResources();
        return frameCtx;
	}

	// Game-only 경로 전용. 프레임 끝에서 Signal을 보내고, 필요하면 즉시 대기까지 처리합니다.
	// Editor 경로는 Signal(SignalFrameCompletion)과 Wait(WaitForNextFrameResources)가 분리됩니다.
	void GraphicDevice_DX12::MoveToNextFrame()
	{
		// fence 값은 반드시 단조 증가해야 GPU가 순서를 올바르게 추적할 수 있다.
		const UINT64 currentFenceValue = mFenceLastSignalValue + 1;
		mCommandQueue->Signal(mFence.Get(), currentFenceValue);
		mFenceLastSignalValue = currentFenceValue;

		// 현재 프레임의 알로케이터가 언제 재사용 가능해지는지 기록해둔다.
		// 다음에 이 인덱스가 돌아왔을 때 이 값을 보고 GPU 완료 여부를 확인한다.
		mFrameContext[mFrameIndex].FenceValue = currentFenceValue;
        SealRetiredResources(currentFenceValue);

		// Present() 이후 스왑 체인이 뒤집혔으므로, 이제 다음 프레임의 백버퍼 인덱스를 가져온다.
		mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

		// 새 mFrameIndex의 알로케이터가 GPU에서 아직 처리 중이면 여기서 바로 대기한다.
		// GPU가 충분히 빠르면 대기 없이 통과한다.
		// CPU는 Fence의 값이 기대하는 값(예: 1)에 도달했는지 확인합니다.
		// 아직 도달하지 않았다면 CPU는 해당 지점에서 실행을 멈추고 기다립니다.
		if (mFence->GetCompletedValue() < mFrameContext[mFrameIndex].FenceValue)
		{
			mFence->SetEventOnCompletion(mFrameContext[mFrameIndex].FenceValue, mFenceEvent);
			WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
		}
	}

	void GraphicDevice_DX12::ExcuteCommandList()
	{
		ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	void GraphicDevice_DX12::Render()
	{
		// Record all the commands we need to render the scene into the command list.
		//PopulateCommandList();
	}

	void GraphicDevice_DX12::CloseCommandList()
	{
		//Indicate that the back buffer will now be used to present.
			CD3DX12_RESOURCE_BARRIER resourceBarrierRT
			= CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &resourceBarrierRT);

		if (FAILED(mCommandList->Close()))
			assert(NULL, "mCommandList->Close()");
	}

	void GraphicDevice_DX12::ResetCommandList()
	{
		// Reset the command list to prepare for recording new commands.
		if (FAILED(mCommandList->Reset(mFrameContext[mFrameIndex].CommandAllocator.Get(), mPipelineState.Get())))
			assert(NULL, "mCommandList->Reset()");
	}

	void GraphicDevice_DX12::ResetCommandAllocator()
	{
		// Reset the command allocator to prepare for recording new commands.
		if (FAILED(mFrameContext[mFrameIndex].CommandAllocator->Reset()))
			assert(NULL, "mCommandAllocator->Reset()");
	}



	
	void GraphicDevice_DX12::TranstionResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
	{
		CD3DX12_RESOURCE_BARRIER resourceBarrierPR
			= CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), before, after);
		mCommandList->ResourceBarrier(1, &resourceBarrierPR);
	}



	void GraphicDevice_DX12::DrawInstanced(UINT VertexCountPerInstance,
		UINT InstanceCount,
		UINT StartVertexLocation,
		UINT StartInstanceLocation)
	{
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mCommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	void GraphicDevice_DX12::DrawIndexedInstanced(UINT IndexCountPerInstance,
		UINT InstanceCount,
		UINT StartIndexLocation,
		INT BaseVertexLocation,
		UINT StartInstanceLocation)
	{
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	void GraphicDevice_DX12::PopulateCommandList()
	{
		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
		mCommandList->DrawInstanced(3, 1, 0, 0);
	}

	void GraphicDevice_DX12::Present()
	{
		mSwapChain->Present(1, 0);
	}

    void GraphicDevice_DX12::Resize(UINT width, UINT height)
    {
        if (!width || !height || (width == mWidth && height == mHeight))
            return;
        // Called between frames, before resetting/recording the command list.
        WaitForGpu();
        for (auto& target : mRenderTargets) target.Reset();
        const HRESULT hr = mSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        if (FAILED(hr)) throw std::runtime_error("Swap-chain resize failed");
        mWidth = width;
        mHeight = height;
        mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
        for (UINT i = 0; i < 2; ++i)
        {
            if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mRenderTargets[i].GetAddressOf()))))
                throw std::runtime_error("Failed to get resized back buffer");
            mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, mRenderTragetDesciptorHandle[i]);
            mFrameContext[i].FenceValue = 0;
        }
        if (!mDepthBuffer->Create(width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            throw std::runtime_error("Failed to resize depth texture");
    }

    DescriptorHandle GraphicDevice_DX12::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV: return mSrvAllocator.Allocate();
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV: return mOffscreenRtvAllocator.Allocate();
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV: return mDsvAllocator.Allocate();
        default: throw std::runtime_error("Unsupported descriptor heap type");
        }
    }

    void GraphicDevice_DX12::RetireResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        DescriptorHandle srv, DescriptorHandle rtv, DescriptorHandle dsv, DescriptorHandle uav)
    {
        if (resource || srv || rtv || dsv || uav)
            mRetiredResources.push_back({ std::move(resource), srv, rtv, dsv, uav, UINT64_MAX });
    }

    void GraphicDevice_DX12::SealRetiredResources(UINT64 fenceValue)
    {
        // Only a frame-completion signal covers recorded draws and platform windows.
        // A synchronous texture upload may signal in the middle of frame recording.
        for (auto& retired : mRetiredResources)
            if (retired.FenceValue == UINT64_MAX) retired.FenceValue = fenceValue;
    }

    void GraphicDevice_DX12::CollectRetiredResources()
    {
        const UINT64 completed = mFence->GetCompletedValue();
        auto it = mRetiredResources.begin();
        while (it != mRetiredResources.end())
        {
            if (it->FenceValue == UINT64_MAX || it->FenceValue > completed) { ++it; continue; }
            mSrvAllocator.Free(it->Srv);
            mSrvAllocator.Free(it->Uav);
            mOffscreenRtvAllocator.Free(it->Rtv);
            mDsvAllocator.Free(it->Dsv);
            it = mRetiredResources.erase(it);
        }
    }

    void GraphicDevice_DX12::UploadTexture(ID3D12Resource* texture, const D3D12_SUBRESOURCE_DATA& data,
        D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        // Isolated command storage: uploading never resets the active frame's list.
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list;
        Microsoft::WRL::ComPtr<ID3D12Resource> upload;
        auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(GetRequiredIntermediateSize(texture, 0, 1));
        if (FAILED(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))) ||
            FAILED(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list))) ||
            FAILED(mDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
            throw std::runtime_error("Texture upload allocation failed");
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture, before, D3D12_RESOURCE_STATE_COPY_DEST);
        if (before != D3D12_RESOURCE_STATE_COPY_DEST) list->ResourceBarrier(1, &barrier);
        if (!UpdateSubresources(list.Get(), texture, upload.Get(), 0, 0, 1, &data))
            throw std::runtime_error("Texture upload failed");
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture, D3D12_RESOURCE_STATE_COPY_DEST, after);
        list->ResourceBarrier(1, &barrier);
        if (FAILED(list->Close())) throw std::runtime_error("Texture upload command list failed");
        ID3D12CommandList* lists[] = { list.Get() };
        mCommandQueue->ExecuteCommandLists(1, lists);
        WaitForGpu(); // Keep upload/allocator alive until their copy completes.
    }
}
