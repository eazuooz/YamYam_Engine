#pragma once
#include "yaEntity.h"

namespace ya
{
	using namespace enums;
	class UIBase : public Entity
	{
	public:
		struct Event
		{
			void operator=(std::function<void()> func)
			{
				mEvent = std::move(func);
			}
			void operator()()
			{
				if (mEvent)
					mEvent();
			}
			std::function<void()> mEvent;
		};

		UIBase(eUIType type);
		virtual ~UIBase();

		/// <summary>
		/// ui가 로드 되었을떄 초기화 해주는 함수
		/// </summary>
		void Initialize();
		/// <summary>
		/// ui가 활성화되면 호출되는 함수
		/// </summary>
		void Active();
		/// <summary>
		/// ui가 비활성화되면 호출되는 함수
		/// </summary>
		void InActive();
		void Update();
		void LateUpdate();
		void Render();
		/// <summary>
		/// UI가 사라질떄 호출되는 함수
		/// </summary>
		void UIClear();

		virtual void OnInit();
		virtual void OnActive();
		virtual void OnInActive();
		virtual void OnUpdate();
		virtual void OnLateUpdate();
		virtual void OnRender();
		virtual void OnClear();

		eUIType GetType() { return mType; }
		void SetType(eUIType type) { mType = type; }
		void SetFullScreen(bool enable) { mbFullScreen = enable; }
		bool IsFullScreen() { return mbFullScreen; }
		Vector2 GetPos() { return mPosition; }
		void SetPos(Vector2 position) { mPosition = position; }
		Vector2 GetSize() { return mSize; }
		void SetSize(Vector2 size) { mSize = size; }


	protected:
		Vector2 mPosition;
		Vector2 mSize;
		bool mbMouseOn;

	private:
		eUIType mType;
		bool mbFullScreen;
		bool mbEnabled;
		
		//std::vector<UIBase*> mChilds;
		UIBase* mParent;
	};
}
