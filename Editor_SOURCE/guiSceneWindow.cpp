#include "guiSceneWindow.h"

namespace gui
{
	SceneWindow::SceneWindow()
	{
		SetName("Scene");
		SetSize(ImVec2(300, 600));
	}

	SceneWindow::~SceneWindow()
	{
	}

	void SceneWindow::Initialize()
	{
	}

	void SceneWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void SceneWindow::OnGUI()
	{
		
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void SceneWindow::Run()
	{
		bool Active = (bool)GetState();
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		ImGui::End();
	}

	void SceneWindow::OnEnable()
	{
	}

	void SceneWindow::OnDisable()
	{
	}

	void SceneWindow::OnDestroy()
	{
	}

}