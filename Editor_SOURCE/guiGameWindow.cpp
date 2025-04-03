#include "guiGameWindow.h"

namespace gui
{
	GameWindow::GameWindow()
	{
		SetName("Game");
		SetSize(ImVec2(300, 600));
	}

	GameWindow::~GameWindow()
	{
	}

	void GameWindow::Initialize()
	{
	}

	void GameWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void GameWindow::OnGUI()
	{
		
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void GameWindow::Run()
	{
		bool Active = (bool)GetState();
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		ImGui::End();
	}

	void GameWindow::OnEnable()
	{
	}

	void GameWindow::OnDisable()
	{
	}

	void GameWindow::OnDestroy()
	{
	}

}