#pragma once
#include "guiEditor.h"
#include "guiEditorWindow.h"

#include "..\\YamYamEngine_CORE\\yaEvent.h"


namespace gui
{
	class ImguiEditor : public EditorWindow
	{
	public:
		ImguiEditor();
		~ImguiEditor();

		void Initialize() override;
		void Update() override;
		void OnGUI() override;
		void Run() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;
		void OnEvent(ya::Event& e) override;

		void Begin();
		void End();
		void Release();
		void SetDarkThemeColors();

		void BlockEvent(bool block) { mBlockEvent = block; }

	private:
		bool mBlockEvent;
	};
}
