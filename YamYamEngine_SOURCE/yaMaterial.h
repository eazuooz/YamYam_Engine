#pragma once
#include "yaResource.h"
#include "yaTexture.h"
#include "yaShader.h"

namespace ya
{
	class Material : public Resource
	{
	public:
		struct Data
		{
			std::wstring albedo; //difuuse
		};

		Material();
		virtual ~Material();

		virtual HRESULT Save(const std::wstring& path) override;
		virtual HRESULT Load(const std::wstring& path) override;

		void Bind();

		void SetShader(graphics::Shader* shader) { mShader = shader; }

	private:
		graphics::eRenderingMode mMode;
		Material::Data mData;

		//Texture* mTexture;
		graphics::Shader* mShader;
	};
}
