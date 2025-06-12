#include "guiProjectWindow.h"

namespace gui
{
	ProjectWindow::ProjectWindow()
	{
		SetName("Project");
		SetSize(ImVec2(300, 600));
	}

	ProjectWindow::~ProjectWindow()
	{
	}

	void ProjectWindow::Initialize()
	{
	}

	void ProjectWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void ProjectWindow::OnGUI()
	{
		
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void ProjectWindow::Run()
	{
		bool Active = (bool)GetState();
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		ImGui::End();
	}

	void ProjectWindow::OnEnable()
	{
	}

	void ProjectWindow::OnDisable()
	{
	}

	void ProjectWindow::OnDestroy()
	{
	}

}