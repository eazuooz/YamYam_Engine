#pragma once
#include "guiEntity.h"

namespace gui
{
	/// <summary>
	/// EditorWindow는 독립적인 창을 생성하여 에디터에 새로운 도구나 워크플로우를 추가하는 데 사용됩니다.
	/// 에디터 에서 실행되는 완전히 새로운 사용자 인터페이스를 제공합니다.
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
