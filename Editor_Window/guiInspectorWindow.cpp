#include "guiInspectorWindow.h"


namespace gui
{
	InspectorWindow::InspectorWindow()
	{
		SetName("Inspector");
		SetSize(ImVec2(300, 600));
	}

	InspectorWindow::~InspectorWindow()
	{
	}

	void InspectorWindow::Initialize()
	{
	}

	void InspectorWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void InspectorWindow::OnGUI()
	{
		
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void InspectorWindow::Run()
	{
		bool Active = (bool)GetState();
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		ImGui::End();
	}

	void InspectorWindow::OnEnable()
	{
	}

	void InspectorWindow::OnDisable()
	{
	}

	void InspectorWindow::OnDestroy()
	{
	}

}
