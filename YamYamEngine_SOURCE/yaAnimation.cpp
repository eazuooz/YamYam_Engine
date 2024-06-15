#include "yaAnimation.h"
#include "yaTime.h"
#include "yaTransform.h"
#include "yaGameObject.h"
#include "yaAnimator.h"
#include "yaRenderer.h"
#include "yaCamera.h"

namespace ya
{
	Animation::Animation()
		: Resource(enums::eResourceType::Animation)
		, mAnimator(nullptr)
		, mTexture(nullptr)
		, mAnimationSheet{}
		, mIndex(-1)
		, mTime(0.0f)
		, mbComplete(false)
	{
	}

	Animation::~Animation()
	{
	}

	HRESULT Animation::Save(const std::wstring& path)
	{
		return E_NOTIMPL;
	}

	HRESULT Animation::Load(const std::wstring& path)
	{

		return E_NOTIMPL;
	}

	void Animation::Update()
	{
		if (mbComplete)
			return;

		mTime += Time::DeltaTime();

		if (mAnimationSheet[mIndex].Duration < mTime)
		{
			mTime = 0.0f;
			if (mIndex < mAnimationSheet.size() - 1)
				mIndex++;
			else
				mbComplete = true;
		}
	}

	void Animation::Render()
	{
		if (mTexture == nullptr)
			return;
	}

	void Animation::CreateAnimation(const std::wstring& name, graphics::Texture* spriteSheet
		, Vector2 leftTop, Vector2 size, Vector2 offset, UINT spriteLength, float duration)
	{
		SetName(name);
		mTexture = spriteSheet;
		for (size_t i = 0; i < spriteLength; i++)
		{
			Sprite sprite = {};
			sprite.LeftTop.x = leftTop.x + (size.x * i);
			sprite.LeftTop.y = leftTop.y;
			sprite.Size = size;
			sprite.Offset = offset;
			sprite.Duration = duration;

			mAnimationSheet.push_back(sprite);
		}
	}

	void Animation::Reset()
	{
		mTime = 0.0f;
		mIndex = 0;
		mbComplete = false;
	}
}
