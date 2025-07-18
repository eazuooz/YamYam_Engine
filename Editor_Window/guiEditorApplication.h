#pragma once
#include "guiEditorWindow.h"
#include "guiImguiEditor.h"


#include "..\\YamYamEngine_CORE\\yaRenderTarget.h"
#include "..\\YamYamEngine_CORE\\yaKeyEvent.h"
#include "..\\YamYamEngine_CORE\\yaMouseEvent.h"
#include "..\\YamYamEngine_CORE\\yaEditorCamera.h"


namespace ya
{
	class KeyPressedEvent;
}

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
			auto iter = EditorWindows.find(name);
			if (iter == EditorWindows.end())
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
		/// 도킹된 창들을 바깥으로 꺼내서 진짜 윈도우처럼 보여주게 업데이트합니다.
		/// </summary>
		static void UpdatePlatformWindows();

		/// <summary>
		/// 에디터를 종료합니다.
		/// </summary>
		static void Release();

		/// <summary>
		/// EditorApplication 이벤트 처리
		/// </summary>
		static void OnEvent(ya::Event& e);

		static void SetupInitialDockLayout();
		static void OpenProject();
		static void NewScene();
		static void SaveScene();
		static void SaveSceneAs();
		static void OpenScene(const std::filesystem::path& path);
		static void OnImGuiRender();

		/// <summary>
		/// Event Callback
		/// </summary>
		static void SetKeyPressed(int keyCode, int scancode, int action, int mods);
		static void SetCursorPos(double x, double y);
		static bool OnKeyPressed(ya::KeyPressedEvent& e);

		/// <summary>
		/// Getter and Setter
		/// </summary>
		static void SetGuizmoType(int type) { GuizmoType = type; }
		static ImguiEditor* GetImguiEditor() { return ImguiEditor; }

	private:
		static ImguiEditor* ImguiEditor;

		static std::map<std::wstring, EditorWindow*> EditorWindows;
		static ImGuiWindowFlags Flag;
		static ImGuiDockNodeFlags DockspaceFlags;
		static eState State;
		static bool FullScreen;
		//static ya::math::Vector2 ViewportBounds[2];
		static ya::math::Vector2 ViewportSize;
		static bool ViewportFocused;
		static bool ViewportHovered;
		static int GuizmoType;

		static ya::graphics::RenderTarget* FrameBuffer;
		static ya::EventCallbackFn EventCallback;

		
	};
}

