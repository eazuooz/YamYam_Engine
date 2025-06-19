#pragma once
#include "..\\YamYamEngine_CORE\\yaEvent.h"

#include "guiEditor.h"
#include "guiEditorWindow.h"
#include "guiDescriptorAllocator.h"

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
		void UpdatePlatformWindows();
		void Release();
		void SetDarkThemeColors();

		void BlockEvent(bool block) { mBlockEvent = block; }

	private:
		static DescriptorHeapAllocator DescHeapAllocator;

		bool mBlockEvent;
		// descriptor allocator
	};
}
