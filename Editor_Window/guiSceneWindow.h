#pragma once
#include "guiEditorWindow.h"
#include "guiEditor.h"
#include "..\\YamYamEngine_CORE\\yaGameObject.h"
#include "..\\YamYamEngine_CORE\\yaEditorCamera.h"

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

		void SetGuizmoType(int type) { GuizmoType = type; }

	private:
		std::vector<Editor*> mEditors;
		ya::GameObject* mEditorCameraObject;
		ya::EditorCamera* mEditorCamera;

		ya::math::Vector2 ViewportBounds[2];
		ya::math::Vector2 ViewportSize;
		bool ViewportFocused;
		bool ViewportHovered;
		int GuizmoType;
	};
}
