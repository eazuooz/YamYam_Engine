#pragma once
#include "yaGraphics.h"


// Sample Example Dx11
// https://github.com/kevinmoran/BeginnerDirect3D11


namespace ya::graphics
{
	struct FrameContext
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
		UINT64 FenceValue = 0;
	};

	class GraphicDevice_DX12
	{
	public:
		GraphicDevice_DX12();
		~GraphicDevice_DX12();
		
		bool CreateDevice();
		void GetHardwareAdapter(
			_In_ IDXGIFactory1* pFactory, 
			_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

		/// <summary>
		/// gpu object make trought device
		/// </summary>
		void Initialize();
		void WaitForGpu();
		void SignalFrameCompletion();
		FrameContext* WaitForNextFrameResources();
		void MoveToNextFrame();
		void PopulateCommandList();
		void Render();
		void CloseCommandList();
		void ExcuteCommandList();
		void Present();

		Microsoft::WRL::ComPtr<ID3D12Device> GetID3D12Device() { return mDevice; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return mCommandQueue; }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvHeap() { return mSrvHeap; }
		Microsoft::WRL::ComPtr<IDXGISwapChain3> GetSwapChain() { return mSwapChain; }
		Microsoft::WRL::ComPtr<ID3D12Resource> GetRenderTargetResource(int idx) { return mRenderTargets[idx]; }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return mRtvHeap; }

		//imgui
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return mCommandList; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRnderTargetDescriptorHandle(int idx) { return mRenderTragetDesciptorHandle[idx]; }

	private:
		bool mbUseWarpDevice;
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
		Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		UINT mRtvDescriptorSize;

		Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[2]; // Double buffering
		CD3DX12_CPU_DESCRIPTOR_HANDLE mRenderTragetDesciptorHandle[2];
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
		FrameContext mFrameContext[2]; //double buffering

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		UINT mFrameIndex;
		
		// App resources.
		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		UINT64 mFenceLastSignalValue;
		HANDLE mFenceEvent;

		//imgui 
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap;
		

		//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mImguiCommandList;
		//HANDLE mImguiFenceEvent;
	};

	// This is a helper to get access to a global device instance
	//	- The engine uses this, but it is not necessary to use a single global device object
	//	- This is not a lifetime managing object, just a way to globally expose a reference to an object by pointer
	inline GraphicDevice_DX12*& GetDevice()
	{
		static GraphicDevice_DX12* device = nullptr;
		return device;
	}
}
