#pragma once
#include "guiEditorWindow.h"
#include "guiEditor.h"

namespace gui
{
	class GameWindow : public EditorWindow
	{
	public:
		GameWindow();
		~GameWindow();

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
