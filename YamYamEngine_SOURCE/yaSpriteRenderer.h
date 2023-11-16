#pragma once
#include "yaEntity.h"
#include "yaComponent.h"

namespace ya
{
	class SpriteRenderer : public Component
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

		void Initialize() override;
		void Update()  override;
		void LateUpdate()  override;
		void Render(HDC hdc)  override;

		void ImageLoad(const std::wstring& path);

	private:
		Gdiplus::Image* mImgae;
		UINT mWidth;
		UINT mHeight;
	};
}