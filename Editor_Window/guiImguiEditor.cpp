#include "guiImguiEditor.h"
#include "guiEditorApplication.h"


#include "..\\YamYamEngine_CORE\\yaApplication.h"



extern ya::Application application;

namespace gui
{
	DescriptorHeapAllocator ImguiEditor::DescHeapAllocator;
	ImguiEditor::ImguiEditor()
		: mBlockEvent(false)
	{
	}

	ImguiEditor::~ImguiEditor()
	{
		Release();
	}

	void ImguiEditor::Initialize()
	{
		// Make process DPI aware and obtain main monitor scale
		//ImGui_ImplWin32_EnableDpiAwareness();
		//float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;
		//io.ConfigViewportsNoDefaultParent = true;
		//io.ConfigDockingAlwaysTabBar = true;
		//io.ConfigDockingTransparentPayload = true;
		//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
		//io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		
		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();
		//style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		//style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
		//io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		//io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(application.GetWindow().GetHwnd());
		
		ya::graphics::GraphicDevice_DX12* device = ya::graphics::GetDevice();
		DescHeapAllocator.Create(device->GetID3D12Device().Get(), device->GetSrvHeap().Get());;

		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = device->GetID3D12Device().Get();
		init_info.CommandQueue = device->GetCommandQueue().Get();
		init_info.NumFramesInFlight = 2; //double buuffering
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		
		
		init_info.SrvDescriptorHeap = device->GetSrvHeap().Get();
		init_info.SrvDescriptorAllocFn = 
			[] (ImGui_ImplDX12_InitInfo*, 
				D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
				D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) 
			{ 
				return ImguiEditor::DescHeapAllocator.Alloc(out_cpu_handle, out_gpu_handle);
			};

		init_info.SrvDescriptorFreeFn = 
			[] (ImGui_ImplDX12_InitInfo*,
				D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, 
				D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) 
			{ 
				return ImguiEditor::DescHeapAllocator.Free(cpu_handle, gpu_handle);
			};

		

		ImGui_ImplDX12_Init(&init_info);
	}
	void ImguiEditor::Update()
	{
	}
	void ImguiEditor::OnGUI()
	{
	}
	void ImguiEditor::Run()
	{
	}
	void ImguiEditor::OnEnable()
	{
	}
	void ImguiEditor::OnDisable()
	{
	}
	void ImguiEditor::OnDestroy()
	{

	}
	void ImguiEditor::OnEvent(ya::Event& e)
	{
		// Block event when ImGui is capturing mouse or keyboard
		if (mBlockEvent)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(ya::eEventCategory::EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(ya::eEventCategory::EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}
	void ImguiEditor::Begin()
	{
		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}
	void ImguiEditor::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)application.GetWindow().GetWidth()
			, (float)application.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		
		ya::graphics::GraphicDevice_DX12* graphicDevice = ya::graphics::GetDevice();
		ya::graphics::FrameContext* frameCtx = graphicDevice->WaitForNextFrameResources();
		UINT backBufferIdx = graphicDevice->GetSwapChain()->GetCurrentBackBufferIndex();
		//frameCtx->CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = graphicDevice->GetRenderTargetResource(backBufferIdx).Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList 
			= graphicDevice->GetCommandList();
		//commandList->Reset(frameCtx->CommandAllocator.Get(), nullptr);
		//commandList->ResourceBarrier(1, &barrier);

		// Render Dear ImGui graphics
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHep = graphicDevice->GetRTVHeap();

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = graphicDevice->GetRnderTargetDescriptorHandle(backBufferIdx);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap = graphicDevice->GetSrvHeap();
		//commandList->ClearRenderTargetView(handle, clear_color_with_alpha, 0, nullptr);
		//commandList->OMSetRenderTargets(1, &handle, FALSE, nullptr);
		commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		commandList->ResourceBarrier(1, &barrier);
		commandList->Close();
	}

	void ImguiEditor::UpdatePlatformWindows()
	{
		ImGuiIO& io = ImGui::GetIO();
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImguiEditor::Release()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
	void ImguiEditor::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
}
