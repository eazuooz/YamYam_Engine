#pragma once
#include "guiEditorWindow.h"
#include "guiEditor.h"

namespace gui
{
	class SceneWindow : public EditorWindow
	{
	public:
		SceneWindow();
		~SceneWindow();

		void Initialize() override;
		void Update() override;
		void OnGUI() override; 
		void Run() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

	private:
		std::vector<Editor*> mEditors;
	};
}
