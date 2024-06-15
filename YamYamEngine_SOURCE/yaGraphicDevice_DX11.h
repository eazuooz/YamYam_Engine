#pragma once
#include "yaGraphics.h"

// Sample Example Dx11
// https://github.com/kevinmoran/BeginnerDirect3D11

namespace ya::graphics
{
	class GraphicDevice_DX11
	{
	public:
		GraphicDevice_DX11();
		~GraphicDevice_DX11();

		/// <summary>
		/// gpu object make trought device 
		/// </summary>
		/// <returns></returns>
		bool CreateDevice();
		bool CreateSwapchain(DXGI_SWAP_CHAIN_DESC desc);
		bool GetBuffer(UINT Buffer, REFIID riid, void** ppSurface);
		bool CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
		                            ID3D11RenderTargetView** ppRTView);
		bool CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
		                            ID3D11DepthStencilView** ppDepthStencilView);
		bool CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData,
		                     ID3D11Texture2D** ppTexture2D);
		bool CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState);
		bool CreateVertexShader(const std::wstring& fileName, ID3DBlob** ppCode, ID3D11VertexShader** ppVertexShader);
		bool CreatePixelShader(const std::wstring& fileName, ID3DBlob** ppCode, ID3D11PixelShader** ppPixelShader);
		bool CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements
		                       , const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength,
		                       ID3D11InputLayout** ppInputLayout);
		bool CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData,
		                  ID3D11Buffer** ppBuffer);
		bool CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
		                              ID3D11ShaderResourceView** ppSRView);
		bool CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc,
		                           ID3D11RasterizerState** ppRasterizerState);
		bool CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState);
		bool CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc,
		                             ID3D11DepthStencilState** ppDepthStencilState);
		/// <summary>
		/// context swtich
		/// </summary>
		void SetDataGpuBuffer(ID3D11Buffer* buffer, void* data, UINT size);
		void SetShaderResource(eShaderStage stage, UINT startSlot, ID3D11ShaderResourceView** ppSRV);

		void BindInputLayout(ID3D11InputLayout* pInputLayout);
		void BindPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void BindVS(ID3D11VertexShader* pVertexShader);
		void BindPS(ID3D11PixelShader* pPixelShader);
		void BindVertexBuffer(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers,
		                      const UINT* pStrides, const UINT* pOffsets);
		void BindIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset);
		void BindConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer);
		void BindSampler(eShaderStage stage, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		void BindSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers);
		void BindRasterizerState(ID3D11RasterizerState* pRasterizerState);
		void BindBlendState(ID3D11BlendState* pBlendState, const FLOAT BlendFactor[4], UINT SampleMask);
		void BindDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef);
		void BindViewPort();
		void BindRenderTargets(UINT NumViews = 1, ID3D11RenderTargetView* const* ppRenderTargetViews = nullptr,
		                       ID3D11DepthStencilView* pDepthStencilView = nullptr);
		void BindDefaultRenderTarget();

		void ClearRenderTargetView();
		void ClearDepthStencilView();

		void Initialize();
		void Draw(UINT vertexCount, UINT startVertexLocation) const;
		void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation) const;
		void Present() const;

		[[discard]] Microsoft::WRL::ComPtr<ID3D11Device> GetID3D11Device() { return mDevice; }

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> mContext;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mSamplers;
	};

	// This is a helper to get access to a global device instance
	//	- The engine uses this, but it is not necessary to use a single global device object
	//	- This is not a lifetime managing object, just a way to globally expose a reference to an object by pointer
	inline GraphicDevice_DX11*& GetDevice()
	{
		static GraphicDevice_DX11* device = nullptr;
		return device;
	}
}
