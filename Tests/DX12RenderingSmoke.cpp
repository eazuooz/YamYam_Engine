#include "yaApplication.h"
#include "yaRenderer.h"
#include "yaResources.h"
#include "yaShader.h"
#include "yaMesh.h"
#include "yaTexture.h"
#include "yaEditorCamera.h"
#include "yaSpriteRenderer.h"
#include "yaTransform.h"
#include "guiImguiEditor.h"
#include <d3d12sdklayers.h>
#include <array>
#include <set>
#include <stdexcept>

ya::Application application;
using namespace ya;
using namespace ya::graphics;
using Microsoft::WRL::ComPtr;

static void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

struct Readback
{
    ComPtr<ID3D12Resource> Buffer;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    UINT Width = 0, Height = 0;

    std::array<BYTE, 4> Pixel(UINT x, UINT y) const
    {
        BYTE* pixels = nullptr;
        const SIZE_T size = SIZE_T(Buffer->GetDesc().Width);
        CD3DX12_RANGE range(0, size);
        Require(SUCCEEDED(Buffer->Map(0, &range, reinterpret_cast<void**>(&pixels))), "Readback map failed");
        const BYTE* pixel = pixels + Footprint.Offset + SIZE_T(y) * Footprint.Footprint.RowPitch + SIZE_T(x) * 4;
        std::array<BYTE, 4> result = { pixel[0], pixel[1], pixel[2], pixel[3] };
        CD3DX12_RANGE written(0, 0);
        Buffer->Unmap(0, &written);
        return result;
    }
};

static Readback CopyResourceToReadback(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
    Readback result;
    const auto desc = resource->GetDesc();
    UINT64 bytes = 0;
    auto device = GetDevice()->GetID3D12Device();
    device->GetCopyableFootprints(&desc, 0, 1, 0, &result.Footprint, nullptr, nullptr, &bytes);
    result.Width = UINT(desc.Width);
    result.Height = desc.Height;
    auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bytes);
    Require(SUCCEEDED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&result.Buffer))), "Readback allocation failed");
    auto list = GetDevice()->GetCommandList();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, state, D3D12_RESOURCE_STATE_COPY_SOURCE);
    list->ResourceBarrier(1, &barrier);
    CD3DX12_TEXTURE_COPY_LOCATION source(resource, 0);
    CD3DX12_TEXTURE_COPY_LOCATION destination(result.Buffer.Get(), result.Footprint);
    GetDevice()->GetCommandList()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COPY_SOURCE, state);
    list->ResourceBarrier(1, &barrier);
    return result;
}

static Readback CopyToReadback(Texture* texture)
{
    return CopyResourceToReadback(texture->GetResource(), texture->GetState());
}

static void BeginFrame()
{
    GetDevice()->ResetCommandAllocator();
    GetDevice()->ResetCommandList();
    renderer::BeginFrame();
    GetDevice()->SetBaseGraphicsRootSignature();
    GetDevice()->TranstionResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    GetDevice()->BindFrameBuffer();
}

static void SubmitFrame()
{
    GetDevice()->CloseCommandList();
    GetDevice()->ExcuteCommandList();
    GetDevice()->Present();
    GetDevice()->MoveToNextFrame();
}

static D3D12_GPU_VIRTUAL_ADDRESS Draw(Texture& texture, float x, float viewOffset = 0.0f,
    Material* material = nullptr, float depth = 0.5f)
{
    TransformCB data = {};
    data.World = Matrix::CreateScale(0.6f, 0.8f, 1.0f) * Matrix::CreateTranslation(x, 0.0f, depth);
    data.View = Matrix::CreateTranslation(viewOffset, 0.0f, 0.0f);
    data.Projection = Matrix::Identity;
    auto* cb = renderer::constantBuffers[CBSLOT_TRANSFORM];
    cb->SetData(&data);
    cb->Bind(eShaderStage::All);
    if (material) material->Bind();
    else Resources::Find<Shader>(L"SpriteDefaultShader")->Bind();
    texture.Bind(eShaderStage::PS, UINT(eTextureType::Sprite));
    auto* mesh = Resources::Find<Mesh>(L"RectMesh");
    mesh->Bind();
    GetDevice()->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
    return cb->GetCurrentGpuAddress();
}

static void RequirePixel(const Readback& result, const std::array<BYTE, 4>& expected, const char* message)
{
    const auto actual = result.Pixel(result.Width / 2, result.Height / 2);
    for (size_t i = 0; i < 4; ++i)
    {
        if (std::abs(int(actual[i]) - int(expected[i])) <= 1) continue;
        std::cerr << message << " (RGBA: ";
        for (auto value : actual) std::cerr << int(value) << ' ';
        std::cerr << "; expected: ";
        for (auto value : expected) std::cerr << int(value) << ' ';
        std::cerr << ")\n";
        throw std::runtime_error(message);
    }
}

static void CheckRenderingModes()
{
    auto* shader = Resources::Find<Shader>(L"SpriteDefaultShader");
    Material opaque, cutout, transparent;
    // Setting a mode before assigning a shader must be safe. All materials
    // deliberately share one Shader, as they do in actual sprite scenes.
    cutout.SetRenderingMode(eRenderingMode::CutOut);
    transparent.SetRenderingMode(eRenderingMode::Transparent);
    for (auto* material : { &opaque, &cutout, &transparent }) material->SetShader(shader);
    Texture red, green, blue, halfRed, halfGreen, zeroRed, lowRed, edgeRed;
    Require(red.CreateSolidColor(0xff0000ffu) && green.CreateSolidColor(0xff00ff00u)
        && blue.CreateSolidColor(0xffff0000u) && halfRed.CreateSolidColor(0x800000ffu)
        && halfGreen.CreateSolidColor(0x8000ff00u) && zeroRed.CreateSolidColor(0x000000ffu)
        && lowRed.CreateSolidColor(0x020000ffu) && edgeRed.CreateSolidColor(0x030000ffu),
        "Rendering-mode texture upload failed");
    RenderTargetSpecification spec;
    spec.Width = spec.Height = 64;
    spec.Attachments = { eRenderTragetFormat::RGBA8, eRenderTragetFormat::Depth };
    RenderTarget target(spec), reverseTarget(spec);
    struct ExpectedResult { Readback Pixels; std::array<BYTE, 4> Expected; const char* Message; };
    std::vector<ExpectedResult> results;
    struct ModeDraw { Material* MaterialToBind; Texture* Sprite; float Depth; };
    BeginFrame();
    auto check = [&](std::initializer_list<ModeDraw> draws, std::array<BYTE, 4> expected, const char* message)
    {
        target.Bind();
        for (const auto& draw : draws) Draw(*draw.Sprite, 0.0f, 0.0f, draw.MaterialToBind, draw.Depth);
        target.Unbind();
        results.push_back({ CopyToReadback(target.GetAttachmentTexture(0)), expected, message });
    };
    check({ {&opaque, &blue, .8f}, {&opaque, &zeroRed, .2f} }, {255, 0, 0, 0},
        "Opaque incorrectly blended or clipped zero-alpha fragments");
    check({ {&opaque, &red, .2f}, {&opaque, &green, .8f} }, {255, 0, 0, 255},
        "Opaque did not test/write depth");
    check({ {&opaque, &red, .5f}, {&opaque, &green, .5f} }, {0, 255, 0, 255},
        "Opaque lost the legacy LessEqual depth comparison");
    check({ {&opaque, &blue, .8f}, {&cutout, &zeroRed, .2f} }, {0, 0, 255, 255},
        "CutOut did not discard zero alpha");
    check({ {&opaque, &blue, .8f}, {&cutout, &lowRed, .2f}, {&opaque, &green, .5f} }, {0, 255, 0, 255},
        "Discarded CutOut fragments wrote depth");
    check({ {&opaque, &blue, .8f}, {&cutout, &edgeRed, .2f} }, {255, 0, 0, 3},
        "CutOut rejected alpha above the 0.01 threshold");
    check({ {&opaque, &blue, .8f}, {&cutout, &halfRed, .2f}, {&opaque, &green, .5f} }, {255, 0, 0, 128},
        "CutOut blended accepted fragments or failed to write depth");
    check({ {&opaque, &blue, .2f}, {&transparent, &halfRed, .8f} }, {128, 0, 127, 128},
        "Transparent lost alpha blending or the legacy Always depth rule");
    check({ {&transparent, &halfRed, .2f}, {&opaque, &green, .5f} }, {0, 255, 0, 255},
        "Transparent incorrectly wrote depth");
    check({ {&opaque, &blue, .2f}, {&transparent, &lowRed, .8f} }, {2, 0, 253, 2},
        "Transparent incorrectly used the CutOut alpha threshold");
    // Change mode between draws in the same command list, then switch back.
    transparent.SetRenderingMode(eRenderingMode::Opaque);
    check({ {&opaque, &blue, .8f}, {&transparent, &halfRed, .2f} }, {255, 0, 0, 128},
        "Runtime material mode change did not select the opaque PSO");
    transparent.SetRenderingMode(eRenderingMode::Transparent);
    check({ {&opaque, &blue, .8f}, {&transparent, &halfRed, .2f} }, {128, 0, 127, 128},
        "Runtime material mode change did not restore the transparent PSO");
    // Direct Shader::Bind must still use its own defaults, unaffected by materials.
    check({ {&opaque, &blue, .8f}, {nullptr, &halfRed, .2f} }, {255, 0, 0, 128},
        "A material changed the shared shader's default state");

    // Verify the real collector, group order and distance sorting in two cameras.
    // Add objects in an intentionally wrong rendering order, with coplanar opaque
    // and cutout sprites so swapping their queues changes the resulting pixel.
    Scene objects;
    auto add = [&](Material& material, Texture& texture, float z)
    {
        auto* object = new GameObject();
        auto* transform = object->GetComponent<Transform>();
        transform->SetPosition(0.0f, 0.0f, z);
        transform->LateUpdate();
        auto* sprite = object->AddComponent<SpriteRenderer>();
        sprite->SetMaterial(&material);
        sprite->SetSprite(&texture);
        objects.AddGameObject(object, enums::eLayerType::Player);
    };
    add(transparent, halfGreen, -2.0f);
    add(cutout, blue, 0.0f);
    add(transparent, halfRed, 2.0f);
    add(opaque, red, 0.0f);
    GameObject frontCameraObject, backCameraObject;
    auto* frontCamera = frontCameraObject.AddComponent<EditorCamera>();
    auto* backCamera = backCameraObject.AddComponent<EditorCamera>();
    frontCameraObject.GetComponent<Transform>()->SetPosition(0.0f, 0.0f, -10.0f);
    backCameraObject.GetComponent<Transform>()->SetPosition(0.0f, 0.0f, 10.0f);
    backCameraObject.GetComponent<Transform>()->SetRotation(0.0f, 180.0f, 0.0f);
    frontCameraObject.GetComponent<Transform>()->LateUpdate();
    backCameraObject.GetComponent<Transform>()->LateUpdate();
    for (auto* camera : {frontCamera, backCamera})
    {
        camera->SetProjectionType(Camera::eProjectionType::Orthographic);
        camera->SetSize(32.0f);
    }
    target.Bind();
    renderer::RenderSceneFromCamera(&objects, frontCamera);
    target.Unbind();
    results.push_back({CopyToReadback(target.GetAttachmentTexture(0)), {64, 128, 63, 128},
        "Scene rendering did not draw Opaque/CutOut/Transparent in order, far transparent first"});
    reverseTarget.Bind();
    renderer::RenderSceneFromCamera(&objects, backCamera);
    reverseTarget.Unbind();
    results.push_back({CopyToReadback(reverseTarget.GetAttachmentTexture(0)), {128, 64, 63, 128},
        "Reverse camera did not re-sort transparency or culled the double-sided sprite"});
    SubmitFrame();
    GetDevice()->WaitForGpu();
    for (const auto& result : results) RequirePixel(result.Pixels, result.Expected, result.Message);
}

int main(int argc, char** argv)
{
    int exitCode = 0;
    const bool warp = !(argc > 1 && std::string(argv[1]) == "--hardware");
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HWND hwnd = nullptr;
    try
    {
        ComPtr<ID3D12Debug1> debug;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
        {
            debug->EnableDebugLayer();
            debug->SetEnableGPUBasedValidation(TRUE);
        }
        WNDCLASSW wc = {};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"YamYamDX12Smoke";
        RegisterClassW(&wc);
        hwnd = CreateWindowW(wc.lpszClassName, L"DX12 smoke", WS_OVERLAPPEDWINDOW,
            0, 0, 640, 480, nullptr, nullptr, wc.hInstance, nullptr);
        Require(hwnd != nullptr, "Test window creation failed");
        application.GetWindow().SetHwnd(hwnd);
        application.GetWindow().SetWidth(640);
        application.GetWindow().SetHeight(480);
        GraphicDevice_DX12 device(warp);
        device.Initialize();
        ComPtr<ID3D12InfoQueue> messages;
        device.GetID3D12Device().As(&messages);
        renderer::Initialize();
        auto* loadedImage = Resources::Load<Texture>(L"ImageSmoke", L"..\\Resources\\CloudOcean.png");
        Require(loadedImage->GetResource() && loadedImage->GetSRV().ptr, "WIC image load/upload failed");
        const size_t freeBefore = device.GetFreeSrvCount();

        struct FrameResult { Readback Game, Scene; bool Reversed = false; };
        std::vector<FrameResult> results;
        std::set<UINT> frameSlots;
        {
            Texture red, green;
            Require(red.CreateSolidColor(0xff0000ffu), "Red texture upload failed");
            Require(green.CreateSolidColor(0xff00ff00u), "Green texture upload failed");
            RenderTargetSpecification spec;
            spec.Width = spec.Height = 64;
            spec.Attachments = { eRenderTragetFormat::RGBA8, eRenderTragetFormat::Depth };
            RenderTarget game(spec), scene(spec);
            Require(game.GetAttachmentTexture(0)->GetSRV().ptr != scene.GetAttachmentTexture(0)->GetSRV().ptr,
                "Scene and Game share an SRV slot");

            // Multiple submitted frames, different camera matrices, >256 draws
            // per frame (crosses an upload page), and repeated RT replacement.
            for (UINT frame = 0; frame < 8; ++frame)
            {
                frameSlots.insert(device.GetFrameIndex());
                BeginFrame();
                const UINT size = frame % 2 ? 80 : 64;
                game.RequestResize(size, size);
                scene.RequestResize(size, size);
                game.RequestResize(0, 0); // Minimized/collapsed panels must not destroy the last valid target.
                game.Bind();
                Require(game.GetSpecification().Width == size, "Pending RT resize was lost");
                Require(game.GetDisplaySRV().ptr != game.GetAttachmentTexture(0)->GetSRV().ptr,
                    "Display and original RGBA must have separate SRVs");
                std::set<D3D12_GPU_VIRTUAL_ADDRESS> addresses;
                Texture& left = frame % 2 ? green : red;
                Texture& right = frame % 2 ? red : green;
                addresses.insert(Draw(left, -0.5f));
                addresses.insert(Draw(right, 0.5f));
                for (UINT draw = 0; draw < 300; ++draw)
                    Require(addresses.insert(Draw(red, 3.0f)).second, "Two draws share a constant-buffer address");
                game.Unbind();
                scene.Bind();
                Require(addresses.insert(Draw(left, -0.5f, 0.5f)).second, "Two cameras share a constant-buffer address");
                scene.Unbind();
                FrameResult result;
                result.Game = CopyToReadback(game.GetAttachmentTexture(0));
                result.Scene = CopyToReadback(scene.GetAttachmentTexture(0));
                result.Reversed = frame % 2 != 0;
                results.push_back(std::move(result));
                SubmitFrame();
            }
            device.WaitForGpu();
            for (const auto& result : results)
            {
                const UINT w = result.Game.Width, h = result.Game.Height;
                const std::array<BYTE, 4> redPixel = {255, 0, 0, 255};
                const std::array<BYTE, 4> greenPixel = {0, 255, 0, 255};
                const auto expectedLeft = result.Reversed ? greenPixel : redPixel;
                const auto expectedRight = result.Reversed ? redPixel : greenPixel;
                Require(result.Game.Pixel(w/4, h/2) == expectedLeft, "Game left draw was overwritten or not textured");
                Require(result.Game.Pixel(3*w/4, h/2) == expectedRight, "Game right draw was overwritten or not textured");
                Require(result.Scene.Pixel(w/2, h/2) == expectedLeft, "Scene camera output is incorrect");
                Require(result.Scene.Pixel(w/4, h/2)[3] == 0, "Scene camera matrix leaked from the Game pass");
            }

            CheckRenderingModes();

            // Exercise the engine's actual object/renderer/camera path too, with
            // unequal panel aspect ratios and two independent camera transforms.
            {
                Scene objects;
                for (int i = 0; i < 2; ++i)
                {
                    auto* object = new GameObject();
                    auto* transform = object->GetComponent<Transform>();
                    transform->SetPosition(i ? 0.5f : -0.5f, 0.0f, 0.0f);
                    transform->SetScale(0.6f, 0.8f, 1.0f);
                    transform->LateUpdate();
                    object->AddComponent<SpriteRenderer>()->SetSprite(i ? &red : &green);
                    objects.AddGameObject(object, enums::eLayerType::Player);
                }
                auto* untextured = new GameObject();
                untextured->GetComponent<Transform>()->SetScale(0.2f, 0.2f, 1.0f);
                untextured->GetComponent<Transform>()->LateUpdate();
                untextured->AddComponent<SpriteRenderer>();
                objects.AddGameObject(untextured, enums::eLayerType::Player);
                GameObject gameCameraObject, sceneCameraObject;
                auto* gameCamera = gameCameraObject.AddComponent<EditorCamera>();
                auto* sceneCamera = sceneCameraObject.AddComponent<EditorCamera>();
                gameCameraObject.GetComponent<Transform>()->SetPosition(0.0f, 0.0f, -10.0f);
                sceneCameraObject.GetComponent<Transform>()->SetPosition(-0.5f, 0.0f, -10.0f);
                gameCameraObject.GetComponent<Transform>()->LateUpdate();
                sceneCameraObject.GetComponent<Transform>()->LateUpdate();
                gameCamera->SetProjectionType(Camera::eProjectionType::Orthographic);
                sceneCamera->SetProjectionType(Camera::eProjectionType::Orthographic);
                gameCamera->SetSize(80.0f);
                sceneCamera->SetSize(64.0f);
                BeginFrame();
                game.RequestResize(160, 80);
                game.Bind();
                renderer::RenderSceneFromCamera(&objects, gameCamera);
                game.Unbind();
                scene.RequestResize(128, 96);
                scene.Bind();
                renderer::RenderSceneFromCamera(&objects, sceneCamera);
                scene.Unbind();
                auto gamePixels = CopyToReadback(game.GetAttachmentTexture(0));
                auto scenePixels = CopyToReadback(scene.GetAttachmentTexture(0));
                SubmitFrame();
                device.WaitForGpu();
                const std::array<BYTE, 4> greenPixel = {0, 255, 0, 255};
                Require(gamePixels.Pixel(40, 40) == greenPixel, "Game camera/object renderer path failed");
                const std::array<BYTE, 4> whitePixel = {255, 255, 255, 255};
                Require(gamePixels.Pixel(80, 40) == whitePixel, "Missing sprite did not use the white fallback texture");
                Require(scenePixels.Pixel(64, 48) == greenPixel, "Editor camera/object renderer path failed");
                Require(scenePixels.Pixel(32, 48)[3] == 0, "Camera viewport size/transform was not independent");
            }

            // Run the actual editor backend without desktop input or visible
            // platform windows. Read back the composed swap-chain image.
            {
                Texture halfRed;
                Require(halfRed.CreateSolidColor(0x800000ffu), "ImGui transparency texture upload failed");
                Material transparent;
                transparent.SetRenderingMode(eRenderingMode::Transparent);
                transparent.SetShader(Resources::Find<Shader>(L"SpriteDefaultShader"));
                RenderTarget blended(spec);
                gui::ImguiEditor editor;
                editor.Initialize();
                auto& io = ImGui::GetIO();
                io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
                io.IniFilename = nullptr;
                BeginFrame();
                blended.Bind();
                Draw(green, 0.0f);
                Draw(halfRed, 0.0f, 0.0f, &transparent);
                blended.Unbind();
                auto blendedPixels = CopyToReadback(blended.GetAttachmentTexture(0));
                editor.Begin();
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImVec2(300, 160));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("GPU smoke images", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
                ImGui::Image(ImTextureID(game.GetDisplaySRV().ptr), ImVec2(80, 80));
                const ImVec2 gameOrigin = ImGui::GetItemRectMin();
                ImGui::SameLine();
                ImGui::Image(ImTextureID(scene.GetDisplaySRV().ptr), ImVec2(80, 80));
                const ImVec2 sceneOrigin = ImGui::GetItemRectMin();
                ImGui::SameLine();
                ImGui::Image(ImTextureID(blended.GetDisplaySRV().ptr), ImVec2(80, 80));
                const ImVec2 blendedOrigin = ImGui::GetItemRectMin();
                ImGui::End();
                ImGui::PopStyleVar();
                editor.End(); // Binds the main RTV, renders ImGui, and closes the list.
                device.ExcuteCommandList();
                device.WaitForGpu();
                device.ResetCommandAllocator();
                device.ResetCommandList();
                auto composition = CopyResourceToReadback(device.GetRenderTargetResource(device.GetFrameIndex()).Get(),
                    D3D12_RESOURCE_STATE_PRESENT);
                Require(SUCCEEDED(device.GetCommandList()->Close()), "Composition readback list failed");
                device.ExcuteCommandList();
                device.Present();
                device.MoveToNextFrame();
                device.WaitForGpu();
                const std::array<BYTE, 4> greenPixel = {0, 255, 0, 255};
                const std::array<BYTE, 4> redPixel = {255, 0, 0, 255};
                Require(composition.Pixel(UINT(gameOrigin.x) + 20, UINT(gameOrigin.y) + 40) == greenPixel,
                    "ImGui Game image did not sample the engine SRV");
                Require(composition.Pixel(UINT(gameOrigin.x) + 60, UINT(gameOrigin.y) + 40) == redPixel,
                    "ImGui Game image lost its second draw");
                Require(composition.Pixel(UINT(sceneOrigin.x) + 40, UINT(sceneOrigin.y) + 40) == greenPixel,
                    "ImGui Scene image did not sample its own SRV");
                RequirePixel(blendedPixels, {128, 127, 0, 128}, "Original render-target alpha was not preserved");
                const auto displayed = composition.Pixel(UINT(blendedOrigin.x) + 40, UINT(blendedOrigin.y) + 40);
                Require(std::abs(int(displayed[0]) - 128) <= 1 && std::abs(int(displayed[1]) - 127) <= 1
                    && displayed[2] == 0 && displayed[3] == 255,
                    "ImGui applied alpha a second time to the already-composited camera image");
                const std::array<BYTE, 4> black = {0, 0, 0, 255};
                Require(composition.Pixel(UINT(blendedOrigin.x) + 4, UINT(blendedOrigin.y) + 4) == black,
                    "Camera clear RGB was replaced by the ImGui window background");
            }
        }
        // Destruction defers descriptors until the next submission; upload-only
        // waits must not prematurely release descriptors from unsubmitted draws.
        Require(device.GetFreeSrvCount() < freeBefore, "Descriptors were released before frame submission");
        device.WaitForGpu();
        Require(device.GetFreeSrvCount() < freeBefore, "Upload-only fence reclaimed an unsubmitted descriptor");
        BeginFrame();
        SubmitFrame();
        device.WaitForGpu();
        Require(device.GetFreeSrvCount() == freeBefore, "Retired SRV descriptors leaked");

        device.Resize(320, 240);
        device.Resize(0, 0);
        BeginFrame();
        SubmitFrame();
        device.WaitForGpu();
        Require(device.GetViewportWidth() == 320 && device.GetViewportHeight() == 240, "Back-buffer resize failed");
        renderer::Release();
        Resources::Release();
        device.WaitForGpu();
        if (messages)
        {
            UINT errors = 0;
            for (UINT64 i = 0; i < messages->GetNumStoredMessagesAllowedByRetrievalFilter(); ++i)
            {
                SIZE_T size = 0;
                messages->GetMessage(i, nullptr, &size);
                std::vector<BYTE> buffer(size);
                auto* message = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());
                messages->GetMessage(i, message, &size);
                if (message->Severity <= D3D12_MESSAGE_SEVERITY_ERROR)
                {
                    std::cerr << message->pDescription << '\n';
                    ++errors;
                }
            }
            Require(errors == 0, "D3D12 validation reported errors");
        }
        std::cout << "PASS: " << (warp ? "WARP" : "hardware")
            << ", 8 frames, " << frameSlots.size() << " frame slots, 303 draws/frame, two camera outputs, "
            << "texture upload, resize, deferred descriptor reuse, ImGui image composition, "
            << "Opaque/CutOut/Transparent pixels and depth, per-camera sorting, shared-shader mode changes, "
            << (messages ? "zero D3D12 errors" : "debug layer unavailable") << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        exitCode = 1;
    }
    if (hwnd) DestroyWindow(hwnd);
    CoUninitialize();
    return exitCode;
}
