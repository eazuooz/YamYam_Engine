#include "yaMaterial.h"

namespace ya
{
	Material::Material()
		: Resource(enums::eResourceType::Material)
		  , mMode(graphics::eRenderingMode::Opaque)
		  , mAlbedoTexture(nullptr)
		  , mShader(nullptr)
	{
	}

	Material::~Material()
	{
	}

	HRESULT Material::Save(const std::wstring& path)
	{
		return E_NOTIMPL;
	}

	HRESULT Material::Load(const std::wstring& path)
	{
		return E_NOTIMPL;
	}

	void Material::Bind()
	{
		if (mShader)
			mShader->Bind();

		if (mAlbedoTexture)
			mAlbedoTexture->Bind(graphics::eShaderStage::PS, static_cast<UINT>(graphics::eTextureType::Albedo));
	}

	void Material::BindShader()
	{
		if (mShader)
			mShader->Bind();
	}

	void Material::BindTextures()
	{
		if (mAlbedoTexture)
			mAlbedoTexture->Bind(graphics::eShaderStage::PS, static_cast<UINT>(graphics::eTextureType::Albedo));
	}

	void Material::SetRenderingMode(const graphics::eRenderingMode mode)
	{
		mMode = mode;

		switch (mode)
		{
		case graphics::eRenderingMode::Opaque:
			mShader->SetRasterizerState(graphics::eRasterizerState::SolidNone);
			mShader->SetBlendState(graphics::eBlendState::Opaque);
			mShader->SetDepthStencilState(graphics::eDepthStencilState::LessEqual);
			break;

		case graphics::eRenderingMode::CutOut:
			mShader->SetRasterizerState(graphics::eRasterizerState::SolidNone);
			mShader->SetBlendState(graphics::eBlendState::Cutout);
			mShader->SetDepthStencilState(graphics::eDepthStencilState::LessEqual);
			break;

		case graphics::eRenderingMode::Transparent:
			mShader->SetRasterizerState(graphics::eRasterizerState::SolidNone);
			mShader->SetBlendState(graphics::eBlendState::Transparent);
			mShader->SetDepthStencilState(graphics::eDepthStencilState::Always);
			break;

		default:
			break;
		}
	}
}
