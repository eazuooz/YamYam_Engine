# DX12 Scene / Game 뷰 연결

Scene과 Game은 각각 별도의 컬러·깊이 텍스처에 렌더링한다. ImGui는 표시용 GPU SRV 핸들을 받아 에디터 백버퍼에 합성한다. 표시용 SRV는 이미 합성된 RGB를 그대로 보여주도록 샘플 알파를 1로 고정하며, 원본 텍스처의 RGBA 값은 보존한다.

## 프레임 순서

1. `main.cpp`가 현재 프레임의 GPU 완료를 기다린다.
2. `Application::Render()`에서 필요하면 스왑체인을 리사이즈하고 allocator/list를 리셋한다.
3. `renderer::BeginFrame()`이 현재 프레임의 상수 버퍼 할당 위치를 초기화한다.
4. 에디터 모드에서는 `renderer::FrameBuffer`에 게임 씬을 렌더링한다. 게임 전용 모드는 백버퍼에 직접 렌더링한다.
5. `SceneWindow::Run()`이 별도 `EditorCamera`로 씬 렌더 타깃에 그린다. 숨겨진 Scene 탭은 렌더링을 생략한다.
6. 각 컬러 텍스처를 `RENDER_TARGET`에서 `PIXEL_SHADER_RESOURCE` 상태로 바꾼다.
7. Game과 Scene의 `ImGui::Image(ImTextureID(renderTarget->GetDisplaySRV().ptr), size)`가 각각의 이미지를 참조한다.
8. `ImguiEditor::End()`가 백버퍼 RTV를 복원하고 ImGui를 렌더링한 뒤 `PRESENT` 상태로 전환하고 리스트를 닫는다.
9. 메인 리스트, 분리된 ImGui 창, Present 순서로 실행하고 프레임 Fence를 기록한다.
10. 두 렌더 경로가 끝난 뒤 `Application::EndOfFrame()`에서 씬 이벤트를 처리한다.

## 텍스처와 descriptor

- `Texture`가 DX12 리소스와 SRV/RTV/DSV/UAV 핸들을 보유한다. `Create()`의 flags는 **D3D12_RESOURCE_FLAGS**이다.
- `GraphicDevice_DX12`가 shader-visible SRV heap 하나를 엔진과 ImGui에 제공한다. ImGui의 할당·반환 콜백도 엔진 allocator를 이용한다.
- SRV/UAV 4096개, 오프스크린 RTV 256개, DSV 256개를 할당할 수 있다. 고갈되면 예외를 발생시키며 슬롯을 덮어쓰지 않는다.
- 파일 텍스처는 WIC/DDS/TGA에서 2D 이미지의 mip 0을 RGBA8로 읽고, 별도 upload 리스트로 업로드한 후 완료를 기다린다.
- `SpriteDefaultPS`가 t0/s0을 샘플링한다. 스프라이트가 없으면 흰색 기본 텍스처를 사용한다.
- 각 `Shader`가 rasterizer/blend/depth 조합별 PSO를 캐시한다. 게임 렌더 PSO는 D24S8 깊이를 사용하고 ImGui 합성에서는 DSV를 바인딩하지 않는다.

## 불투명·컷아웃·반투명 렌더링

DX11 시절의 렌더 큐와 상태 조합을 DX12 PSO에 연결했다.

| 순서 | 모드 | 카메라와 오브젝트 위치 사이의 거리 정렬 | RGB 블렌딩 | 깊이 비교 / 기록 |
|---|---|---|---|---|
| 1 | Opaque | 가까운 것부터 | 끔 | LessEqual / 켬 |
| 2 | CutOut | 가까운 것부터 | 끔 | LessEqual / 켬 |
| 3 | Transparent | 먼 것부터 | SrcAlpha / InvSrcAlpha | Always / 끔 |

- 카메라마다 목록을 수집하고 다시 정렬한다. 기존 오브젝트 중심 거리 기준을 유지한다.
- Transparent의 `Always`는 이전 동작을 보존한 설정이다. 불투명 물체 뒤의 반투명 오브젝트도 그 위에 합성된다.
- 알파 채널도 기존 `ONE / ZERO` 설정을 유지하므로 마지막 통과 프래그먼트의 알파를 저장한다.
- `Material::SetRenderingMode()`는 머티리얼 자신의 모드만 바꾼다. `Bind()`와 `BindShader()`에서 모드를 전달하므로 같은 Shader를 공유해도 상태가 섞이지 않는다. 셰이더 할당 전에도 모드를 설정할 수 있다.
- 기본 PSO 세 가지는 Shader 로딩 때 생성한다. 기존 Shader 상태 setter와 직접 `Bind()`를 이용하는 추가 조합도 캐시하며, 모드를 전환할 때 이전 PSO를 파괴하지 않는다.
- SpriteDefault와 Triangle 픽셀 셰이더는 CutOut 변형에서만 `clip(alpha - 0.01f)`를 실행한다. Opaque와 Transparent는 이 임계값을 적용하지 않는다. 다른 픽셀 셰이더를 추가할 때도 CutOut을 지원하려면 `YA_ALPHA_TEST` 분기를 구현한다.
- 기존 스프라이트의 알파 잘라내기를 유지하기 위해 `Sprite-Default-Material`은 명시적으로 CutOut을 사용한다. 일반 `Material` 생성자의 기본값은 Opaque다.

반투명 스프라이트를 만들 때는 공유 기본 머티리얼을 바꾸는 대신 해당 오브젝트용 머티리얼을 등록한다.

```cpp
auto* material = new ya::Material();
material->SetShader(ya::Resources::Find<ya::graphics::Shader>(L"SpriteDefaultShader"));
material->SetRenderingMode(ya::graphics::eRenderingMode::Transparent);
ya::Resources::Insert(L"Player-Transparent-Material", material);
spriteRenderer->SetMaterial(material);
```

Scene/Game은 완성된 카메라 화면을 표시한다. `RenderTarget::GetDisplaySRV()`는 RGB를 그대로 읽고 알파만 1로 반환하는 별도 descriptor를 사용한다. ImGui가 반투명 결과를 두 번 블렌딩하지 않으며, 빈 영역은 타깃의 검정 clear 색으로 보인다. 원본 알파가 필요한 셰이더는 기존 `GetAttachmentTexture(0)->GetSRV()`를 사용한다. 표시용 descriptor도 리사이즈·삭제 시 GPU Fence 완료 후 반환한다.

## 상수 버퍼

`ConstantBuffer`는 두 프레임 슬롯마다 별도의 upload page 목록을 가진다. 한 프레임 안에서는 드로우마다 256바이트 정렬된 새 영역을 할당한다. TransformCB 기준 64KiB 페이지에 256개를 담으며, 이를 넘으면 페이지를 추가한다.

`SetData()`가 데이터와 GPU 주소를 새 영역에 기록하고 `Bind()`가 그 주소를 b0에 연결한다. 이미 기록한 드로우의 World/View/Projection 데이터는 이후 오브젝트나 카메라가 덮어쓰지 않는다. 페이지 재사용은 해당 프레임 Fence를 기다린 다음에만 가능하다.

## 리사이즈와 수명

- Game 패널은 이번 프레임에 이미 그린 이미지를 표시하고 다음 `Bind()` 때 새 크기를 적용한다.
- Scene 패널은 자신의 크기를 요청하고 해당 패스 시작 시 적용한다.
- 0 크기와 8192 초과 크기는 무시한다. 카메라 투영은 전체 Win32 창 대신 현재 렌더 타깃 크기를 사용한다.
- 교체한 텍스처와 descriptor는 즉시 반환하지 않는다. 프레임 제출 시 Fence 값을 붙이고 GPU 완료 후 회수한다.
- 텍스처 업로드를 위한 중간 `WaitForGpu()`는 아직 제출하지 않은 프레임의 descriptor를 회수하지 않는다.
- 분리된 ImGui 창도 엔진 command queue를 공유하도록 로컬 DX12 backend를 수정했다. 이 변경은 backend 업데이트 시 유지해야 한다. 창별 allocator/Fence/스왑체인은 기존 backend가 관리한다.

## 함께 수정한 연결부

- 에디터 카메라는 게임 씬의 카메라 목록에 등록하지 않는다.
- 게임 입력은 Game 패널에 포커스와 마우스가 있을 때 전달한다. 기즈모 단축키는 Scene 패널 포커스를 기준으로 처리한다.
- 로딩 중 GPU 업로드와 씬 생성·전환은 메인 스레드에서 수행한다. 기존 worker가 활성 씬과 리소스 맵을 렌더링 중에 바꾸던 경합을 피한다. 비동기 디코딩은 별도 후속 작업이다.
- 리소스 경로는 실행 파일 기준으로 해석하며, Debug/Release 빌드 모두 출력 경로 상위에 Shaders_SOURCE와 Resources를 복사한다.
- 셰이더 컴파일 HRESULT를 확인하여 파일 로드 실패 시 빈 shader blob으로 PSO를 생성하지 않는다.

## 검증

저장소 루트 PowerShell에서:

```powershell
.\Tests\Run-DX12RenderingSmoke.ps1
.\Tests\Run-DX12RenderingSmoke.ps1 -SkipBuild -Hardware
```

첫 명령은 Debug x64 엔진과 테스트를 빌드하고 WARP로 실행한다. 두 번째 명령은 같은 검증을 실제 GPU에서 실행한다. 테스트는 숨겨진 창을 사용하며 데스크톱 입력을 조작하거나 ImGui 설정을 저장하지 않는다.

검증 항목:

- 8프레임, 두 프레임 슬롯, 프레임당 303드로우의 GPU 픽셀 결과
- 파일 텍스처 로드·업로드와 여러 텍스처의 독립적인 SRV
- 서로 다른 카메라 행렬과 64KiB 상수 버퍼 페이지 경계 초과
- 실제 Scene/SpriteRenderer/Camera 경로와 서로 다른 패널 종횡비
- 모드별 GPU RGBA, CutOut 임계값/버려진 픽셀의 깊이, LessEqual/Always와 깊이 기록 여부
- 같은 Shader를 공유한 머티리얼의 프레임 내 모드 전환과 반대쪽 카메라의 반투명 정렬
- ImGui에서 반투명 RGB의 중복 블렌딩 방지와 원본 타깃 알파 보존
- 두 엔진 렌더 타깃을 실제 `ImguiEditor::End()`로 합성한 백버퍼 픽셀
- 반복 RT 리사이즈, 0 크기 요청, 스왑체인 리사이즈
- 프레임 제출 전 descriptor 보존과 완료 후 슬롯 회수
- D3D12 debug layer 및 GPU-based validation 오류 확인(사용 가능한 환경)

분리된 플랫폼 창의 드래그·도킹, 기즈모 조작, 실제 사용자 입력은 수동 확인 대상이다. MSAA, 텍스처 배열/큐브맵/전체 mip chain, 오브젝트 ID readback은 이번 구현 범위에 포함하지 않았다.
