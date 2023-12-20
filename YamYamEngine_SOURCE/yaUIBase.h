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
		/// ui�� �ε� �Ǿ����� �ʱ�ȭ ���ִ� �Լ�
		/// </summary>
		void Initialize();
		/// <summary>
		/// ui�� Ȱ��ȭ�Ǹ� ȣ��Ǵ� �Լ�
		/// </summary>
		void Active();
		/// <summary>
		/// ui�� ��Ȱ��ȭ�Ǹ� ȣ��Ǵ� �Լ�
		/// </summary>
		void InActive();
		void Update();
		void LateUpdate();
		void Render(HDC hdc);
		/// <summary>
		/// UI�� ������� ȣ��Ǵ� �Լ�
		/// </summary>
		void UIClear();

		virtual void OnInit();
		virtual void OnActive();
		virtual void OnInActive();
		virtual void OnUpdate();
		virtual void OnLateUpdate();
		virtual void OnRender(HDC hdc);
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
