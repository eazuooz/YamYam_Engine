#include "yaMaterial.h"
#include <stdexcept>

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
		BindShader();

		if (mAlbedoTexture)
			mAlbedoTexture->Bind(graphics::eShaderStage::PS, static_cast<UINT>(graphics::eTextureType::Albedo));
	}

	void Material::BindShader()
	{
		if (mShader)
			mShader->Bind(mMode);
	}

	void Material::BindTextures()
	{
		if (mAlbedoTexture)
			mAlbedoTexture->Bind(graphics::eShaderStage::PS, static_cast<UINT>(graphics::eTextureType::Albedo));
	}

	void Material::SetRenderingMode(const graphics::eRenderingMode mode)
	{
		if (mode < graphics::eRenderingMode::Opaque || mode >= graphics::eRenderingMode::End)
			throw std::invalid_argument("Invalid material rendering mode");
		// A shader can be shared by materials with different rendering modes.
		// Select its PSO at draw time instead of changing shared shader defaults.
		mMode = mode;
	}
}
