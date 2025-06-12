#include "yaGraphicDevice_DX12.h"
#include "yaApplication.h"
//#include <d3dx12/d3dx12.h>


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
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1; // No MSAA

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
		//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		//for (int i = 0; i < 2; i++)
		//{
		//	if (FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])))) // Added parentheses around the expression
		//		assert(NULL && "Get Swap Chain Buffer Failed!");

		//	mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
		//	rtvHandle.Offset(1, mRtvDescriptorSize);
		//}
	}
}
