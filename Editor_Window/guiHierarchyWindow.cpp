#include "guiHierarchyWindow.h"

namespace gui
{
	HierarchyWindow::HierarchyWindow()
	{
		SetName("Hierarchy");
		SetSize(ImVec2(300, 600));
	}

	HierarchyWindow::~HierarchyWindow()
	{
	}

	void HierarchyWindow::Initialize()
	{
	}

	void HierarchyWindow::Update()
	{
		for (Editor* editor : mEditors)
		{
			editor->Update();
		}
	}

	void HierarchyWindow::OnGUI()
	{
		
		for (Editor* editor : mEditors)
		{
			editor->OnGUI();
		}
	}

	void HierarchyWindow::Run()
	{
	
		bool Active = (bool)GetState();
		ImGui::Begin(GetName().c_str(), &Active, GetFlag());

		Update();
		OnGUI();

		ImGui::End();
	}

	void HierarchyWindow::OnEnable()
	{
	}

	void HierarchyWindow::OnDisable()
	{
	}

	void HierarchyWindow::OnDestroy()
	{
	}

}