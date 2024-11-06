#include "yaShader.h"
#include "yaRenderer.h"
#include "yaResources.h"

namespace ya::graphics
{
	bool Shader::bWireframe = true;

	Shader::Shader()
		: Resource(eResourceType::Shader)
		  , mRasterizerState(eRasterizerState::SolidBack)
		  , mBlendState(eBlendState::AlphaBlend)
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
		if (!GetDevice()->CreateVertexShader(fileName, mVSBlob.GetAddressOf(), mVS.GetAddressOf()))
			return false;

		return true;
	}

	bool Shader::CreatePixelShader(const std::wstring& fileName)
	{
		if (!GetDevice()->CreatePixelShader(fileName, mPSBlob.GetAddressOf(), mPS.GetAddressOf()))
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

			GetDevice()->BindVS(wireframeShaderVS.Get());
			GetDevice()->BindPS(wireframeShaderPS.Get());
			GetDevice()->BindRasterizerState(wireframeRasterizerState.Get());
			GetDevice()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
			GetDevice()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);

			return;
		}

		if (mVS)
			GetDevice()->BindVS(mVS.Get());
		if (mPS)
			GetDevice()->BindPS(mPS.Get());

		GetDevice()->BindRasterizerState(renderer::rasterizerStates[static_cast<UINT>(mRasterizerState)].Get());
		GetDevice()->BindBlendState(renderer::blendStates[static_cast<UINT>(mBlendState)].Get(), nullptr, 0xffffff);
		GetDevice()->BindDepthStencilState(renderer::depthStencilStates[static_cast<UINT>(mDepthStencilState)].Get(), 0);
	}
}
