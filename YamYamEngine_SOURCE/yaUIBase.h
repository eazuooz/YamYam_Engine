#pragma once
#include "yaEntity.h"

namespace ya
{
	using namespace enums;
	class UIBase : public Entity
	{
	public:
		UIBase();
		~UIBase();

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
		virtual void OnClear();

		eUIType GetType() { return mType; }
		void SetFullScreen(bool enable) { mbFullScreen = enable; }
		bool IsFullScreen() { return mbFullScreen; }

	private:
		eUIType mType;
		bool mbFullScreen;
		bool mbEnabled;
	};
}
