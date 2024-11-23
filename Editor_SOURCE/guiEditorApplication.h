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
		enum class eState
		{
			Disable,
			Active,
			Destroy,
		};

		template <typename T>
		T* GetWindow(const std::wstring& name)
		{
			auto iter = mEditorWindows.find(name);
			if (iter == mEditorWindows.end())
				return nullptr; 

			return dynamic_cast<T*>(iter->second);
		}

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
		static void dockSpaceUpdate();
		static void dockSpaceOnGui();
		static void imGuiRender();

		static ImGuiWindowFlags mFlag;
		static ImGuiDockNodeFlags mDockspaceFlags;
		static eState mState;
		static bool mFullScreen;
		static bool mPadding;

		static std::map<std::wstring, EditorWindow*> mEditorWindows;
	};
}

