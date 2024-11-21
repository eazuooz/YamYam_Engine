#pragma once
#include "guiEditor.h"
#include "guiEditorWindow.h"


namespace gui
{
	/// <summary>
	/// EditorApplication Ŭ������ �����Ϳ��� ���Ǵ� ���� Ŭ������
	/// �������� ���� ������ �����ϰ� �������� �ʱ�ȭ �� ���Ḧ ����մϴ�.
	/// </summary>
	class EditorApplication
	{
	public:
		///<summary>
		///�����͸� �ʱ�ȭ�մϴ�.
		///</summary>
		static bool Initialize();
		
		/// <summary>
		/// �����͸� ������ ������Ʈ�մϴ�.
		/// </summary>
		static void Update();

		/// <summary>
		/// ������ �������� ������Ʈ�մϴ�.
		/// </summary>
		static void OnGUI();

		/// <summary>
		/// ������ ������Ʈ
		/// </summary>
		static void Run();

		/// <summary>
		/// �����͸� �����մϴ�.
		/// </summary>
		static void Release();

	private:
		static bool imGguiInitialize();
		static void imGuiRender();
	};
}

