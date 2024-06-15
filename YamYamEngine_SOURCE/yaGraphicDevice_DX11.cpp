#include "yaGraphicDevice_DX11.h"
#include "yaApplication.h"
#include "yaRenderer.h"
#include "yaShader.h"
#include "yaMesh.h"
#include "yaTexture.h"
#include "yaMaterial.h"
#include "yaResources.h"

extern ya::Application application;

namespace ya::graphics
{
	GraphicDevice_DX11::GraphicDevice_DX11()
	{
		GetDevice() = this;

		if (!(CreateDevice()))
			assert(NULL && "Create Device Failed!");
	}

	GraphicDevice_DX11::~GraphicDevice_DX11()
	{
	}

	bool GraphicDevice_DX11::CreateDevice()
	{
		D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, creationFlags,
			featureLevels, ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, mDevice.GetAddressOf(),
			nullptr, mContext.GetAddressOf())))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateSwapchain(DXGI_SWAP_CHAIN_DESC desc)
	{
		Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice = nullptr;
		Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter = nullptr;
		Microsoft::WRL::ComPtr<IDXGIFactory> pFactory = nullptr;

		if (FAILED(mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)pDXGIDevice.GetAddressOf())))
			return false;

		if (FAILED(pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pAdapter.GetAddressOf())))
			return false;

		if (FAILED(pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pFactory.GetAddressOf())))
			return false;

		if (FAILED(pFactory->CreateSwapChain(mDevice.Get(), &desc, mSwapChain.GetAddressOf())))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
	{
		if (FAILED(mSwapChain->GetBuffer(Buffer, riid, ppSurface)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateRenderTargetView(ID3D11Resource* pResource,
	                                                const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
	                                                ID3D11RenderTargetView** ppRTView)
	{
		if (FAILED(mDevice->CreateRenderTargetView(pResource, pDesc, ppRTView)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateDepthStencilView(ID3D11Resource* pResource,
	                                                const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
	                                                ID3D11DepthStencilView** ppDepthStencilView)
	{
		if (FAILED(mDevice->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
	                                         const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
	{
		if (FAILED(mDevice->CreateTexture2D(pDesc, pInitialData, ppTexture2D)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
	                                            ID3D11SamplerState** ppSamplerState)
	{
		if (FAILED(mDevice->CreateSamplerState(pSamplerDesc, ppSamplerState)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateVertexShader(const std::wstring& fileName, ID3DBlob** ppCode,
	                                            ID3D11VertexShader** ppVertexShader)
	{
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

		ID3DBlob* errorBlob = nullptr;
		const std::wstring shaderFilePath = L"..\\Shaders_SOURCE\\";
		D3DCompileFromFile((shaderFilePath + fileName + L"VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
		                   , "main", "vs_5_0", shaderFlags, 0, ppCode, &errorBlob);

		if (errorBlob)
		{
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
			assert(NULL && "hlsl file have problem check message!");
			return false;
		}

		if (FAILED(
			mDevice->CreateVertexShader((*ppCode)->GetBufferPointer(), (*ppCode)->GetBufferSize(), nullptr,
				ppVertexShader)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreatePixelShader(const std::wstring& fileName, ID3DBlob** ppCode,
	                                           ID3D11PixelShader** ppPixelShader)
	{
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

		ID3DBlob* errorBlob = nullptr;
		const std::wstring shaderFilePath = L"..\\Shaders_SOURCE\\";
		D3DCompileFromFile((shaderFilePath + fileName + L"PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE
		                   , "main", "ps_5_0", shaderFlags, 0, ppCode, &errorBlob);

		if (errorBlob)
		{
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
			assert(NULL && "hlsl file have problem check message!");
			return false;
		}

		if (FAILED(
			mDevice->CreatePixelShader((*ppCode)->GetBufferPointer(), (*ppCode)->GetBufferSize(), nullptr, ppPixelShader
			)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements
	                                           , const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength,
	                                           ID3D11InputLayout** ppInputLayout)
	{
		if (FAILED(mDevice->CreateInputLayout(pInputElementDescs, NumElements
			, pShaderBytecodeWithInputSignature
			, BytecodeLength
			, ppInputLayout)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData,
	                                      ID3D11Buffer** ppBuffer)
	{
		if (FAILED(mDevice->CreateBuffer(pDesc, pInitialData, ppBuffer)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateShaderResourceView(ID3D11Resource* pResource,
	                                                  const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc,
	                                                  ID3D11ShaderResourceView** ppSRView)
	{
		if (FAILED(mDevice->CreateShaderResourceView(pResource, pDesc, ppSRView)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc,
	                                               ID3D11RasterizerState** ppRasterizerState)
	{
		if (FAILED(mDevice->CreateRasterizerState(pRasterizerDesc, ppRasterizerState)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState)
	{
		if (FAILED(mDevice->CreateBlendState(pBlendState, ppBlendState)))
			return false;

		return true;
	}

	bool GraphicDevice_DX11::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc,
	                                                 ID3D11DepthStencilState** ppDepthStencilState)
	{
		if (FAILED(mDevice->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState)))
			return false;

		return true;
	}

	void GraphicDevice_DX11::SetDataGpuBuffer(ID3D11Buffer* buffer, void* data, UINT size)
	{
		D3D11_MAPPED_SUBRESOURCE sub = {};
		mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
		memcpy(sub.pData, data, size);
		mContext->Unmap(buffer, 0);
	}

	void GraphicDevice_DX11::SetShaderResource(eShaderStage stage, UINT startSlot, ID3D11ShaderResourceView** ppSRV)
	{
		if (static_cast<UINT>(eShaderStage::VS) & static_cast<UINT>(stage))
			mContext->VSSetShaderResources(startSlot, 1, ppSRV);

		if (static_cast<UINT>(eShaderStage::HS) & static_cast<UINT>(stage))
			mContext->HSSetShaderResources(startSlot, 1, ppSRV);

		if (static_cast<UINT>(eShaderStage::DS) & static_cast<UINT>(stage))
			mContext->DSSetShaderResources(startSlot, 1, ppSRV);

		if (static_cast<UINT>(eShaderStage::GS) & static_cast<UINT>(stage))
			mContext->GSSetShaderResources(startSlot, 1, ppSRV);

		if (static_cast<UINT>(eShaderStage::PS) & static_cast<UINT>(stage))
			mContext->PSSetShaderResources(startSlot, 1, ppSRV);

		if (static_cast<UINT>(eShaderStage::CS) & static_cast<UINT>(stage))
			mContext->CSSetShaderResources(startSlot, 1, ppSRV);
	}

	void GraphicDevice_DX11::BindInputLayout(ID3D11InputLayout* pInputLayout)
	{
		mContext->IASetInputLayout(pInputLayout);
	}

	void GraphicDevice_DX11::BindPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY topology)
	{
		mContext->IASetPrimitiveTopology(topology);
	}

	void GraphicDevice_DX11::BindVS(ID3D11VertexShader* pVertexShader)
	{
		mContext->VSSetShader(pVertexShader, nullptr, 0);
	}

	void GraphicDevice_DX11::BindPS(ID3D11PixelShader* pPixelShader)
	{
		mContext->PSSetShader(pPixelShader, nullptr, 0);
	}

	void GraphicDevice_DX11::BindVertexBuffer(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers,
	                                          const UINT* pStrides, const UINT* pOffsets)
	{
		mContext->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
	}

	void GraphicDevice_DX11::BindIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
	{
		mContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
	}

	void GraphicDevice_DX11::BindConstantBuffer(eShaderStage stage, eCBType type, ID3D11Buffer* buffer)
	{
		UINT slot = static_cast<UINT>(type);
		switch (stage)
		{
		case eShaderStage::VS:
			mContext->VSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::HS:
			mContext->HSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::DS:
			mContext->DSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::GS:
			mContext->GSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::PS:
			mContext->PSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::CS:
			mContext->CSSetConstantBuffers(slot, 1, &buffer);
			break;
		case eShaderStage::All:
			mContext->VSSetConstantBuffers(slot, 1, &buffer);
			mContext->HSSetConstantBuffers(slot, 1, &buffer);
			mContext->DSSetConstantBuffers(slot, 1, &buffer);
			mContext->GSSetConstantBuffers(slot, 1, &buffer);
			mContext->PSSetConstantBuffers(slot, 1, &buffer);
			mContext->CSSetConstantBuffers(slot, 1, &buffer);
			break;
		default:
			break;
		}
	}

	void GraphicDevice_DX11::BindSampler(eShaderStage stage, UINT StartSlot, UINT NumSamplers,
	                                     ID3D11SamplerState* const* ppSamplers)
	{
		if (eShaderStage::VS == stage)
			mContext->VSSetSamplers(StartSlot, NumSamplers, ppSamplers);

		if (eShaderStage::HS == stage)
			mContext->HSSetSamplers(StartSlot, NumSamplers, ppSamplers);

		if (eShaderStage::DS == stage)
			mContext->DSSetSamplers(StartSlot, NumSamplers, ppSamplers);

		if (eShaderStage::GS == stage)
			mContext->GSSetSamplers(StartSlot, NumSamplers, ppSamplers);

		if (eShaderStage::PS == stage)
			mContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
	}

	void GraphicDevice_DX11::BindSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
	{
		BindSampler(eShaderStage::VS, StartSlot, NumSamplers, ppSamplers);
		BindSampler(eShaderStage::HS, StartSlot, NumSamplers, ppSamplers);
		BindSampler(eShaderStage::DS, StartSlot, NumSamplers, ppSamplers);
		BindSampler(eShaderStage::GS, StartSlot, NumSamplers, ppSamplers);
		BindSampler(eShaderStage::PS, StartSlot, NumSamplers, ppSamplers);
	}

	void GraphicDevice_DX11::BindRasterizerState(ID3D11RasterizerState* pRasterizerState)
	{
		mContext->RSSetState(pRasterizerState);
	}

	void GraphicDevice_DX11::BindBlendState(ID3D11BlendState* pBlendState, const FLOAT BlendFactor[4], UINT SampleMask)
	{
		mContext->OMSetBlendState(pBlendState, BlendFactor, SampleMask);
	}

	void GraphicDevice_DX11::BindDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
	{
		mContext->OMSetDepthStencilState(pDepthStencilState, StencilRef);
	}

	void GraphicDevice_DX11::BindViewPort()
	{
		D3D11_VIEWPORT viewPort =
		{
			0, 0,
			static_cast<float>(application.GetWidth()), static_cast<float>(application.GetHeight()),
			0.0f, 1.0f
		};

		mContext->RSSetViewports(1, &viewPort);
	}

	void GraphicDevice_DX11::BindRenderTargets(UINT NumViews
	                                           , ID3D11RenderTargetView* const* ppRenderTargetViews
	                                           , ID3D11DepthStencilView* pDepthStencilView)
	{
		mContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
	}

	void GraphicDevice_DX11::BindDefaultRenderTarget()
	{
		mContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	}

	void GraphicDevice_DX11::ClearRenderTargetView()
	{
		FLOAT backgroundColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
		mContext->ClearRenderTargetView(mRenderTargetView.Get(), backgroundColor);
	}

	void GraphicDevice_DX11::ClearDepthStencilView()
	{
		mContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void GraphicDevice_DX11::Initialize()
	{
#pragma region swapchain desc
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		swapChainDesc.OutputWindow = application.GetHwnd();
		swapChainDesc.Windowed = true;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferDesc.Width = application.GetWidth();
		swapChainDesc.BufferDesc.Height = application.GetHeight();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 144;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

#pragma region 4X MSAA surported check
		// 4X MSAA surported check
		UINT quility = 0;
		mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quility);
		//if (quility > 0)
		//{
		//	swapChainDesc.SampleDesc.Count = 4; // how many multisamples
		//	swapChainDesc.SampleDesc.Quality = quility - 1;
		//}
#pragma endregion

		swapChainDesc.SampleDesc.Count = 1; // how many multisamples
		swapChainDesc.SampleDesc.Quality = 0;
#pragma endregion
		if (!(CreateSwapchain(swapChainDesc)))
			assert(NULL && "Create Swapchain Failed!");

		if (!(GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(mRenderTarget.GetAddressOf()))))
			assert(NULL && "Couldn't bring rendertarget!");

		if (!(CreateRenderTargetView(mRenderTarget.Get(), nullptr, mRenderTargetView.GetAddressOf())))
			assert(NULL && "Create RenderTargetView Failed!");

#pragma region depthstencil desc
		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.Width = application.GetWidth();
		depthStencilDesc.Height = application.GetHeight();
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
#pragma endregion
		if (!(CreateTexture2D(&depthStencilDesc, nullptr, mDepthStencil.GetAddressOf())))
			assert(NULL && "Create depthstencil texture failed!");

		if (!(CreateDepthStencilView(mDepthStencil.Get(), nullptr, mDepthStencilView.GetAddressOf())))
			assert(NULL && "Create depthstencilview failed!");
	}

	void GraphicDevice_DX11::Draw(UINT vertexCount, UINT startVertexLocation) const
	{
		mContext->Draw(vertexCount, startVertexLocation);
	}

	void GraphicDevice_DX11::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation) const
	{
		mContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}

	void GraphicDevice_DX11::Present() const
	{
		mSwapChain->Present(1, 0);
	}
}
