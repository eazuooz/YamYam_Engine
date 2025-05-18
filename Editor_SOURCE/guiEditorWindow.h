#pragma once
#include "guiEntity.h"

namespace gui
{
	/// <summary>
	/// EditorWindow�� �������� â�� �����Ͽ� �����Ϳ� ���ο� ������ ��ũ�÷ο츦 �߰��ϴ� �� ���˴ϴ�.
	/// ������ ���� ����Ǵ� ������ ���ο� ����� �������̽��� �����մϴ�.
	/// </summary>
	class EditorWindow : public Labelled
	{
	public:
		enum class eState
		{
			Disable,
			Active,
			Destroy,
		};

		EditorWindow();
		virtual ~EditorWindow();

		virtual void Initialize();
		virtual void Update();
		virtual void OnGUI(); 
		virtual void Run();
		virtual void OnEnable();
		virtual void OnDisable();
		virtual void OnDestroy();
		virtual void OnEvent(ya::Event& e) {}

		ImGuiWindowFlags GetFlag() const { return Flag; }
		eState GetState() const { return State; }
		void SetState(eState state) { State = state; }
		ImVec2 GetSize() { return mSize; }
		void SetSize(ImVec2 size) { mSize = size; }

	private:
		ImGuiWindowFlags Flag;
		eState State;
		ImVec2 mSize;
	};
}
