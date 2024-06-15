#pragma once
#include "yaComponent.h"
#include "yaAnimation.h"


namespace ya
{
	class Animator : public Component
	{
	public:
		struct Event
		{
			void operator=(std::function<void()> func)
			{
				Action = std::move(func);
			}

			void operator()() const
			{
				if (Action)
					Action();
			}

			std::function<void()> Action;
		};

		struct Events
		{
			Event StartEvent;
			Event CompleteEvent;
			Event EndEvent;
		};

		Animator();
		virtual ~Animator();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		void CreateAnimation(const std::wstring& name
		                     , graphics::Texture* spriteSheet
		                     , Vector2 leftTop
		                     , Vector2 size
		                     , Vector2 offset
		                     , UINT spriteLength
		                     , float duration);
		void CreateAnimationByFolder(/*const std::wstring& name
			, const std::wstring& path
			, Vector2 Offset, float Duration*/);

		Animation* FindAnimation(const std::wstring& name);
		void PlayAnimation(const std::wstring& name, bool loop = true);

		Events* FindEvents(const std::wstring& name);
		std::function<void()>& GetStartEvent(const std::wstring& name);
		std::function<void()>& GetCompleteEvent(const std::wstring& name);
		std::function<void()>& GetEndEvent(const std::wstring& name);

		[[nodiscard]] bool IsComplete() const { return mActiveAnimation->IsComplete(); }

	private:
		std::map<std::wstring, Animation*> mAnimations;
		Animation* mActiveAnimation;
		bool mbLoop;

		//Action
		std::map<std::wstring, Events*> mEvents;
	};
}
