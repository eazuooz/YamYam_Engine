#include "yaShader.h"
#include "yaRenderer.h"
#include "yaResources.h"

namespace ya::graphics
{
	bool Shader::bWireframe = false;

	Shader::Shader()
		: Resource(eResourceType::Shader)
		  , mRasterizerState(eRasterizerState::SolidNone)
		  , mBlendState(eBlendState::Opaque)
		  , mDepthStencilState(eDepthStencilState::LessEqual)
	{
	}

	Shader::~Shader()
	{
	}

	HRESULT Shader::Save(const std::wstring& path)
	{
		return E_NOTIMPL;
	}

	HRESULT Shader::Load(const std::wstring& path)
	{
		size_t fineNameBeginOffset = path.rfind(L"\\") + 1;
		size_t fineNameEndOffset = path.length() - fineNameBeginOffset;
		const std::wstring fileName(path.substr(fineNameBeginOffset, fineNameEndOffset));

		if (!Create(eShaderStage::VS, fileName))
			return S_FALSE;
		if (!Create(eShaderStage::PS, fileName))
			return S_FALSE;

		return S_OK;
	}

	bool Shader::Create(const eShaderStage stage, const std::wstring& fileName)
	{
		if (stage == eShaderStage::VS)
			CreateVertexShader(fileName);
		if (stage == eShaderStage::PS)
			CreatePixelShader(fileName);

		return true;
	}

	bool Shader::CreateVertexShader(const std::wstring& fileName)
	{
		if (!GetDevice<GraphicDevice_DX11>()->CreateVertexShader(fileName, mVSBlob.GetAddressOf(), mVS.GetAddressOf()))
			return false;

		return true;
	}

	bool Shader::CreatePixelShader(const std::wstring& fileName)
	{
		if (!GetDevice<GraphicDevice_DX11>()->CreatePixelShader(fileName, mPSBlob.GetAddressOf(), mPS.GetAddressOf()))
			return false;

		return true;
	}

	void Shader::Bind()
	{
		if (bWireframe)
		{
			Shader* wireframeShader = Resources::Find<Shader>(L"WireframeShader");
			Microsoft::WRL::ComPtr<ID3D11VertexShader> wireframeShaderVS = wireframeShader->GetVS();
			Microsoft::WRL::ComPtr<ID3D11PixelShader> wireframeShaderPS = wireframeShader->GetPS();
			Microsoft::WRL::ComPtr<ID3D11RasterizerState> wireframeRasterizerState 
				= renderer::rasterizerStates[static_cast<UINT>(eRasterizerState::Wireframe)];

			GetDevice<GraphicDevice_DX11>()->BindVS(wireframeShaderVS.Get());
			GetDevice<GraphicDevice_DX11>()->BindPS(wireframeShaderPS.Get());
			GetDevice<GraphicDevice_DX11>()->BindRasterizerState(wireframeRasterizerState.Get());
			GetDevice<GraphicDevice_DX11>()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
			GetDevice<GraphicDevice_DX11>()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);

			return;
		}

		if (mVS)
			GetDevice<GraphicDevice_DX11>()->BindVS(mVS.Get());
		if (mPS)
			GetDevice<GraphicDevice_DX11>()->BindPS(mPS.Get());

		GetDevice<GraphicDevice_DX11>()->BindRasterizerState(renderer::rasterizerStates[static_cast<UINT>(mRasterizerState)].Get());
		GetDevice<GraphicDevice_DX11>()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
		GetDevice<GraphicDevice_DX11>()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);
	}
}
