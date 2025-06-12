#pragma once
#include "yaGraphics.h"


// Sample Example Dx11
// https://github.com/kevinmoran/BeginnerDirect3D11


namespace ya::graphics
{
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

	private:
		bool mbUseWarpDevice;
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice;
		Microsoft::WRL::ComPtr<IDXGIFactory4> mFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		UINT mRtvDescriptorSize;

		Microsoft::WRL::ComPtr<ID3D12Resource> mRenderTargets[2]; // Double buffering


		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

		UINT mFrameIndex;
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
