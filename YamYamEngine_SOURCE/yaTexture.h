#pragma once
#include "yaResource.h"


namespace ya::graphics
{
	class Texture : public Resource
	{
	public:
		enum class eTextureType
		{
			Bmp,
			Png,
			None,
		};

		static Texture* Create(const std::wstring& name, UINT width, UINT height);

		Texture();
		~Texture();

		virtual HRESULT Save(const std::wstring& path) override;
		virtual HRESULT Load(const std::wstring& path) override;
		//COLORREF GetPixel(int x, int y);

		UINT GetWidth() const { return mWidth; }
		UINT GetHeight() const { return mHeight; }
		eTextureType GetTextureType() const { return mType; }
		bool IsAlpha() const { return mbAlpha; }

		void SetWidth(UINT width) { mWidth = width; }
		void SetHeight(UINT height) { mHeight = height; }

	private:
		bool mbAlpha;
		eTextureType mType;

		UINT mWidth;
		UINT mHeight;
	};
}
