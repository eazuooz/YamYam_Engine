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
			auto iter = EditorWindows.find(name);
			if (iter == EditorWindows.end())
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
		/// ��ŷ�� â���� �ٱ����� ������ ��¥ ������ó�� �����ְ� ������Ʈ�մϴ�.
		/// </summary>
		static void UpdatePlatformWindows();

		/// <summary>
		/// �����͸� �����մϴ�.
		/// </summary>
		static void Release();

		/// <summary>
		/// EditorApplication �̺�Ʈ ó��
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

