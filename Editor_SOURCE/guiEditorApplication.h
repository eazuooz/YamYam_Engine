#pragma once
#include "guiEditorWindow.h"
#include "guiImguiEditor.h"

#include "..\\YamYamEngine_SOURCE\\yaRenderTarget.h"
#include "..\\YamYamEngine_SOURCE\\yaKeyEvent.h"
#include "..\\YamYamEngine_SOURCE\\yaMouseEvent.h"


namespace ya
{
	class KeyPressedEvent;
}

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

		/// <summary>
		/// EditorApplication �̺�Ʈ ó��
		/// </summary>
		static void OnEvent(ya::Event& e);

		static void OpenProject();
		static void NewScene();
		static void SaveScene();
		static void SaveSceneAs();
		static void OpenScene(const std::filesystem::path& path);

		static void OnImGuiRender();

		//Event
		static void SetKeyPressed(int keyCode, int scancode, int action, int mods);
		static void SetCursorPos(double x, double y);


		static bool OnKeyPressed(ya::KeyPressedEvent& e);


		static void SetGuizmoType(int type) { mGuizmoType = type; }

	private:
		static ImguiEditor* mImguiEditor;

		static std::map<std::wstring, EditorWindow*> mEditorWindows;
		static ImGuiWindowFlags mFlag;
		static ImGuiDockNodeFlags mDockspaceFlags;
		static eState mState;
		static bool mFullScreen;
		static ya::math::Vector2 mViewportBounds[2];
		static ya::math::Vector2 mViewportSize;
		static bool mViewportFocused;
		static bool mViewportHovered;
		static int mGuizmoType;

		static ya::graphics::RenderTarget* mFrameBuffer;
		static ya::EventCallbackFn mEventCallback;
	};
}

