#pragma once
#include "guiEditor.h"
#include "guiEditorWindow.h"


namespace gui
{
	/// <summary>
	/// EditorApplication 클래스는 에디터에서 사용되는 메인 클래스로
	/// 에디터의 메인 루프를 실행하고 에디터의 초기화 및 종료를 담당합니다.
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
		///에디터를 초기화합니다.
		///</summary>
		static bool Initialize();
		
		/// <summary>
		/// 에디터를 로직을 업데이트합니다.
		/// </summary>
		static void Update();

		/// <summary>
		/// 에디터 렌더링을 업데이트합니다.
		/// </summary>
		static void OnGUI();

		/// <summary>
		/// 에디터 업데이트
		/// </summary>
		static void Run();

		/// <summary>
		/// 에디터를 종료합니다.
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

