#pragma once
#include "yaResource.h"
#include "yaTexture.h"

namespace ya
{
	class Animation : public Resource
	{
	public:
		struct Sprite
		{
			Vector2 LeftTop;
			Vector2 Size;
			Vector2 Offset;
			float Duration;

			Sprite()
				: LeftTop(Vector2::Zero)
				, Size(Vector2::Zero)
				, Offset(Vector2::Zero)
				, Duration(0.0f)
			{

			}
		};

		Animation();
		virtual ~Animation();

		virtual HRESULT Save(const std::wstring& path) override;
		virtual HRESULT Load(const std::wstring& path) override;

		void Update();
		void Render(const Matrix& view, const Matrix& projection);

		void CreateAnimation(const std::wstring& name
			, graphics::Texture* spriteSheet
			, Vector2 leftTop
			, Vector2 size
			, Vector2 offset
			, UINT spriteLength
			, float duration);

		void Reset();

		bool IsComplete() const { return mbComplete; }
		void SetAnimator(class Animator* animator) { mAnimator = animator; }


	private:
		Animator* mAnimator;
		graphics::Texture* mTexture;

		std::vector<Sprite> mAnimationSheet;
		int mIndex;
		float mTime;
		bool mbComplete;
	};
	
}
