#include "yaAnimator.h"
#include "yaResources.h"
#include "yaTexture.h"

namespace ya
{
	Animator::Animator()
		: Component(enums::eComponentType::Animator)
		, mAnimations{}
		, mActiveAnimation(nullptr)
		, mbLoop(false)
	{
	}

	Animator::~Animator()
	{
		for (auto& iter : mAnimations)
		{
			delete iter.second;
			iter.second = nullptr;
		}

		for (auto& iter : mEvents)
		{
			delete iter.second;
			iter.second = nullptr;
		}
	}

	void Animator::Initialize()
	{
	}

	void Animator::Update()
	{
		if (mActiveAnimation)
		{
			mActiveAnimation->Update();

			Events* events 
				= FindEvents(mActiveAnimation->GetName());

			if (mActiveAnimation->IsComplete() == true)
			{
				if (events)
					events->CompleteEvent();

				if (mbLoop == true)
					mActiveAnimation->Reset();
			}
		}
	}

	void Animator::LateUpdate()
	{
	}

	void Animator::Render()
	{
		//if (mActiveAnimation)
		//	mActiveAnimation->Render(hdc);
	}

	void Animator::CreateAnimation(const std::wstring& name
		, graphics::Texture* spriteSheet
		, Vector2 leftTop
		, Vector2 size, Vector2 offset
		, UINT spriteLength, float duration)
	{
		Animation* animation = nullptr;
		animation = FindAnimation(name);
		if (animation != nullptr)
			return;

		animation = new Animation();
		animation->SetName(name);
		animation->CreateAnimation(name, spriteSheet
			,leftTop, size, offset, spriteLength, duration);

		animation->SetAnimator(this);

		Events* events = new Events();
		mEvents.insert(std::make_pair(name, events));

		mAnimations.insert(std::make_pair(name, animation));
	}

	void Animator::CreateAnimationByFolder(/*const std::wstring& name
		, const std::wstring& path
		, Vector2 Offset
		, float Duration*/)
	{
		//Animation* animation = nullptr;
		//animation = FindAnimation(name);
		//if (animation != nullptr)
		//	return;
		////D:\JunJae\AR50\YamYam_Engine\Resources\Mushroom\1.bmp
		//int fileCount = 0;
		//std::filesystem::path fs(path);
		//std::vector<graphics::Texture*> images = {};
		//for (auto& p : std::filesystem::recursive_directory_iterator(fs))
		//{
		//	std::wstring fileName = p.path().filename();
		//	std::wstring fullName = p.path();

		//	graphics::Texture* texture = Resources::Load<graphics::Texture>(fileName, fullName);
		//	images.push_back(texture);
		//	fileCount++;
		//}

		//UINT sheetWidth = images[0]->GetWidth() * fileCount;
		//UINT sheetHeight = images[0]->GetHeight();
		//graphics::Texture* spriteSheet = graphics::Texture::Create(name, sheetWidth, sheetHeight);

		//UINT imageWidth = images[0]->GetWidth();
		//UINT imageHeight = images[0]->GetHeight();
		//for (UINT i = 0; i < images.Size(); i++)
		//{
		//	BitBlt(spriteSheet->GetHdc(), i * imageWidth, 0
		//		, imageWidth, imageHeight 
		//		, images[i]->GetHdc(), 0, 0, SRCCOPY);
		//}

		//CreateAnimation(name, spriteSheet
		//	, Vector2(0.0f, 0.0f), Vector2(imageWidth, imageHeight)
		//	, Offset, fileCount, Duration);
	}


	Animation* Animator::FindAnimation(const std::wstring& name)
	{
		auto iter = mAnimations.find(name);
		if (iter == mAnimations.end())
			return nullptr;

		return iter->second;
	}

	void Animator::PlayAnimation(const std::wstring& name, bool loop)
	{
		Animation* animation = FindAnimation(name);
		if (animation == nullptr)
			return;


		if (mActiveAnimation)
		{
			Events* currentEvents
				= FindEvents(mActiveAnimation->GetName());
		
			if (currentEvents)
				currentEvents->EndEvent();
		}


		Events* nextEvents
			= FindEvents(animation->GetName());

		if (nextEvents)
			nextEvents->StartEvent();

		mActiveAnimation = animation;
		mActiveAnimation->Reset();
		mbLoop = loop;
	}

	Animator::Events* Animator::FindEvents(const std::wstring& name)
	{
		auto iter = mEvents.find(name);
		if (iter == mEvents.end())
			return nullptr;

		return iter->second;
	}

	std::function<void()>& Animator::GetStartEvent(const std::wstring& name)
	{
		// TODO: 여기에 return 문을 삽입합니다.
		Events* events = FindEvents(name);
		return events->StartEvent.Action;
	}

	std::function<void()>& Animator::GetCompleteEvent(const std::wstring& name)
	{
		// TODO: 여기에 return 문을 삽입합니다.
		Events* events = FindEvents(name);
		return events->CompleteEvent.Action;
	}

	std::function<void()>& Animator::GetEndEvent(const std::wstring& name)
	{
		// TODO: 여기에 return 문을 삽입합니다.
		Events* events = FindEvents(name);
		return events->EndEvent.Action;
	}

}
