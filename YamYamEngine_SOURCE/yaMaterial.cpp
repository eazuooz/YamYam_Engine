#include "yaMaterial.h"

namespace ya
{

	Material::Material()
		: Resource(enums::eResourceType::Material)
		, mMode(graphics::eRenderingMode::Opaque)
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
		//mTexture->Bind(eShaderStage::All, 0);
		mShader->Bind();
	}

}
