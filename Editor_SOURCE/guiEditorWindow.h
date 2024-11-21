#pragma once
#include "guiEntity.h"

namespace gui
{
	/// <summary>
	/// EditorWindow�� �������� â�� �����Ͽ� �����Ϳ� ���ο� ������ ��ũ�÷ο츦 �߰��ϴ� �� ���˴ϴ�.
	/// ������ ���� ����Ǵ� ������ ���ο� ����� �������̽��� �����մϴ�.
	/// </summary>
	class EditorWindow : public Entity
	{
	public:
		EditorWindow();
		virtual ~EditorWindow();

		virtual void Initialize();
		virtual void Update();
		virtual void OnGUI(); 
		virtual void OnEnable();
		virtual void OnDisable();
		virtual void OnDestroy();

	private:

	};
}
