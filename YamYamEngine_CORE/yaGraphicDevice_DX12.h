#pragma once
#include "yaGraphics.h"
#include "yaDescriptorAllocator.h"
#pragma comment(lib, "dxguid.lib")


// Sample Example Dx11
// https://github.com/kevinmoran/BeginnerDirect3D11


namespace ya::graphics
{
	class Texture;

	struct FrameContext
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
		UINT64 FenceValue = 0;
	};

	class GraphicDevice_DX12
	{
	public:
		explicit GraphicDevice_DX12(bool useWarpDevice = false);
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
		bool CreateCommittedResource(D3D12_HEAP_PROPERTIES* pHeapProperties,
			D3D12_HEAP_FLAGS HeapFlags,
			D3D12_RESOURCE_DESC* pDesc,
			D3D12_RESOURCE_STATES InitialResourceState,
			D3D12_CLEAR_VALUE* pOptimizedClearValue,
			REFIID riidResource,
			void** ppvResource);
		bool CreateVertexShader(const std::wstring& fileName, ID3DBlob** ppCode);
		bool CreatePixelShader(const std::wstring& fileName, ID3DBlob** ppCode);
		bool CreateGraphicsPipelineState(_In_  const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc/*, void** ppPipelineState*/);

		// binding command list...
		void BindVertexBuffer(UINT StartSlot, UINT NumViews, D3D12_VERTEX_BUFFER_VIEW* pViews);
        void BindViewportAndScissor(UINT width = 0, UINT height = 0);
        void BindFrameBuffer(bool clear = true, bool bindDepth = true);
        void Resize(UINT width, UINT height);
        UINT GetViewportWidth() const { return mViewportWidth; }
        UINT GetViewportHeight() const { return mViewportHeight; }

        DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
        void RetireResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            DescriptorHandle srv = {}, DescriptorHandle rtv = {}, DescriptorHandle dsv = {}, DescriptorHandle uav = {});
        void CollectRetiredResources();
        size_t GetFreeSrvCount() const { return mSrvAllocator.GetFreeCount(); }
        void UploadTexture(ID3D12Resource* texture, const D3D12_SUBRESOURCE_DATA& data,
            D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
		void SetBaseGraphicsRootSignature();

		// render ...
		void WaitForGpu();
		void SignalFrameCompletion();
		FrameContext* WaitForNextFrameResources();
		void MoveToNextFrame();
		void ResetCommandList();
		void ResetCommandAllocator();
		void TranstionResourceBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
		void PopulateCommandList();
		void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
		void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation);
		void Render();
		void CloseCommandList();
		void ExcuteCommandList();
		void Present();

		Microsoft::WRL::ComPtr<ID3D12Device> GetID3D12Device() { return mDevice; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return mCommandQueue; }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvHeap() { return mSrvAllocator.GetHeap(); }
		Microsoft::WRL::ComPtr<IDXGISwapChain3> GetSwapChain() { return mSwapChain; }
		Microsoft::WRL::ComPtr<ID3D12Resource> GetRenderTargetResource(int idx) { return mRenderTargets[idx]; }
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeap() { return mRtvHeap; } 
		Microsoft::WRL::ComPtr<ID3D12RootSignature>  GetRootSignature() { return mRootSignature; }

		//imgui
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return mCommandList; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetRnderTargetDescriptorHandle(int idx) { return mRenderTragetDesciptorHandle[idx]; }
		UINT GetFrameIndex() const { return mFrameIndex; }

	private:
		bool mbUseWarpDevice;
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
		Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		// framebuffer
		UINT mFrameIndex;
		UINT mRtvDescriptorSize;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		FrameContext mFrameContext[2]; //double buffering
		Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[2]; // Double buffering
		CD3DX12_CPU_DESCRIPTOR_HANDLE mRenderTragetDesciptorHandle[2];

		// fence
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
		UINT64 mFenceLastSignalValue;
		HANDLE mFenceEvent = nullptr;

		//imgui 
        DescriptorAllocator mSrvAllocator;
        DescriptorAllocator mOffscreenRtvAllocator;
        DescriptorAllocator mDsvAllocator;
        std::unique_ptr<Texture> mDepthBuffer;
        UINT mWidth = 0, mHeight = 0;
        UINT mViewportWidth = 0, mViewportHeight = 0;
        HANDLE mFrameLatencyEvent = nullptr;

        struct RetiredResource
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
            DescriptorHandle Srv, Rtv, Dsv, Uav;
            UINT64 FenceValue = UINT64_MAX; // Assigned only when the frame is submitted.
        };
        std::vector<RetiredResource> mRetiredResources;
        void SealRetiredResources(UINT64 fenceValue);
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
