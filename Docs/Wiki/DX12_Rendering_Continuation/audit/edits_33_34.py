e=Edit(33,'이 글은 DX11 시절 Scene/Game을 분리했던 구조를 설명하는 연결 문서입니다. 기존의 SRV 객체 포인터 방식과 현재 DX12 GPU handle 방식을 구분해 읽습니다.','같은 Scene을 서로 다른 카메라로 RT에 그린 뒤 ImGui 패널에 표시하는 이유를 이해합니다.','flowchart LR\n    S[같은 Scene] --> E[EditorCamera → Scene RT]\n    S --> G[게임 카메라 → Game FrameBuffer]\n    E --> I[각 ImGui::Image]\n    G --> I\n    I --> B[에디터 백버퍼]')
e.para('<td>현재 마우스 포커스 여부','<td>Focused는 키보드 포커스, Hovered는 마우스 위치에 따른 영역 포함 여부입니다.</td>')
e.para('이 과정을 `SceneWindow`와 `GameWindow`에서','과거 분리의 핵심은 출력 타깃과 View/Projection을 구분하는 것입니다. 현재 DX12에서는 Game 렌더링을 UI 구성 전에 수행하고, Scene 렌더링은 SceneWindow 내부에서 수행합니다. 각 창이 동일한 전체 프레임 루프를 독립적으로 실행하는 것은 아닙니다.')
e.para('씬뷰와 게임뷰는 각자 다른 카메라,','Scene/Game은 다른 카메라와 RT를 사용하고, 리소스·명령 큐·셰이더·오브젝트 수집/정렬 코드는 공유합니다. 현재 DX12 연결은 다음 두 글에서 이어집니다: <mention-page url="'+NEW1+'"/> <mention-page url="'+NEW2+'"/>')
e.block(0,'''// DX11 개발 당시 흐름을 정리한 예시입니다.
if (ImGui::Begin("Game"))
{
    const ImVec2 size = ImGui::GetContentRegionAvail();
    if (FrameBuffer && size.x > 0 && size.y > 0)
    {
        auto* texture = FrameBuffer->GetAttachmentTexture(0);
        if (texture && texture->GetSRV())
            ImGui::Image((ImTextureID)texture->GetSRV().Get(), size);
    }
    // PROJECT_ITEM 드롭을 받는 코드는 payload 크기·문자열 형식을 검증합니다.
    // 현재 OpenScene 함수는 구현 전 상태입니다.
}
ImGui::End();
// 다른 editor window들은 이 Begin/End 바깥에서 각자의 창을 실행합니다.
for (auto& entry : EditorWindows) entry.second->Run();''')
e.block(1,'''using namespace ya::graphics;
RenderTargetSpecification spec{};
spec.Width = width;   // 0보다 큰 실제 패널 렌더링 크기
spec.Height = height;
spec.Attachments = {eRenderTragetFormat::RGBA8, eRenderTragetFormat::Depth};
mRenderTarget = RenderTarget::Create(spec);
// 소유 클래스에서 기존 RT 교체와 최종 해제를 관리합니다.''')
e.block(4,'''// 함수 호출 형태: 생략부를 실행 코드처럼 복사하지 않도록 목록을 명시합니다.
renderer::RenderRenderables(opaqueList, view, proj);
renderer::RenderRenderables(cutoutList, view, proj);
renderer::RenderRenderables(transparentList, view, proj);''')
e.block(5,'''// DX11 SceneWindow 내부의 RT 바인딩 발췌입니다.
// 패널 Begin/End, 카메라 업데이트, 가시성·크기 검사는 바깥에서 처리합니다.
auto* rt = mEditorCamera->GetRenderTarget();
auto rtv = rt->GetAttachmentTexture(0)->GetRTV();
auto dsv = rt->GetDepthAttachment()->GetDSV();
ya::graphics::GetDevice()->ClearRenderTargetView(rtv);
ya::graphics::GetDevice()->ClearDepthStencilView(dsv);
ya::graphics::GetDevice()->BindRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
// 이어서 같은 EditorCamera의 View/Projection으로 렌더링합니다.
// ImGui를 그리기 전에는 최종 출력 백버퍼를 다시 바인딩합니다.''')
e.block(6,'''// DX11 backend: 컬러 Texture의 ID3D11ShaderResourceView*를 전달합니다.
ImGui::Image((ImTextureID)sceneColorTexture->GetSRV().Get(), sceneSize);
ImGui::Image((ImTextureID)gameColorTexture->GetSRV().Get(), gameSize);

// 현재 DX12 backend: RT 표시용 GPU SRV descriptor handle을 전달합니다.
ImGui::Image((ImTextureID)sceneRT->GetDisplaySRV().ptr, sceneSize);
ImGui::Image((ImTextureID)gameRT->GetDisplaySRV().ptr, gameSize);
// 서로 다른 backend용 예시입니다. 같은 실행 경로에서 모두 호출하지 않습니다.''')
e.finish('### 영상과 코드의 시점\n\n기존 영상·구조 그림은 Scene/Game 분리 당시의 기록입니다. 최신 실행 캡처와 descriptor 수명·상수 버퍼 분리는 후속 DX12 문서에서 확인합니다. RT를 읽는 SRV와 쓰는 RTV는 같은 컬러 리소스를 가리키며, 그리기 상태에서 샘플링 상태로 전환한 뒤 ImGui가 사용해야 합니다.')

e=Edit(34,'프로젝트에 고정한 NuGet 패키지 버전과 헤더 경로를 확인하고, Agility SDK의 런타임 선택·배포는 별도 단계임을 이해합니다.','헤더가 보인다는 사실과 app-local D3D12Core 런타임이 실제 선택됐다는 사실을 구분합니다.')
e.para('Windows SDK는 운영체제 업데이트 주기에','Agility SDK는 앱이 사용할 D3D12 런타임을 OS 기본 런타임과 분리해 배포할 수 있게 합니다. 다만 지원 OS 기반 버전·업데이트와 드라이버·GPU 기능 조건은 여전히 필요합니다.')
e.para('**크로스 플랫폼 빌드 지원**','**팀의 Windows 빌드 환경 공유**')
e.para('NuGet 패키지는 여러 플랫폼과','NuGet은 패키지 버전과 네이티브 빌드 설정을 프로젝트에서 관리하도록 돕습니다. NuGet을 사용한다고 Direct3D 12 자체가 크로스 플랫폼 API가 되는 것은 아닙니다.')
a=e.body.index('<table>',e.body.index('## DirectX 관련 주요'));b=e.body.index('</table>',a)+len('</table>')
e.replace(e.body[a:b],'''| 패키지·도구 | 역할 |
| --- | --- |
| `Microsoft.Direct3D.D3D12` | Agility SDK의 실제 패키지 이름. 현재 CORE의 packages.config는 **1.616.1**로 고정 |
| WinPixEventRuntime | PIX 이벤트 마커 런타임. PIX 전체 앱 설치와는 별개 |
| DirectXTex / DirectXTK12 / DirectXMath | 이미지 처리·런타임 유틸리티·수학 라이브러리. 각각 독립적인 의존성 |
| D3D12MemoryAllocator | 리소스 메모리 할당을 돕는 별도 라이브러리. descriptor와 GPU 동기화까지 자동으로 해결하지 않음 |

패키지마다 실제 배포 ID와 지원 플랫폼을 확인합니다. `Microsoft.Direct3D.D3D12.Agility`라는 별도 필수 패키지를 설치하는 과정으로 설명하지 않습니다.''')
a=e.body.index('<table>',e.body.index('## NuGet과 vcpkg'));b=e.body.index('</table>',a)+len('</table>')
e.replace(e.body[a:b],'''| 항목 | NuGet | vcpkg |
| --- | --- | --- |
| 이 프로젝트에서의 사용 | packages.config와 vcxproj import로 SDK 버전 참조 | 필요할 때 C/C++ 라이브러리 의존성 구성 |
| 버전 고정 | 프로젝트 패키지 버전 고정 | manifest와 baseline 등으로 프로젝트별 버전 관리 가능 |
| Visual Studio 연결 | 패키지의 props/targets에 따라 설정 | MSBuild 통합 또는 CMake toolchain 사용 가능 |
| 설치 위치 | 이 저장소는 솔루션 packages 폴더 사용 | classic/manifest 모드와 설정에 따라 달라짐 |

한쪽이 항상 더 낫거나 vcpkg가 전역 버전만 지원하는 것은 아닙니다. 같은 라이브러리를 두 경로로 중복 링크하지 않도록 관리합니다.''')
e.para('찾아보기 탭에서 "D3D12"를','찾아보기 탭에서 `Microsoft.Direct3D.D3D12`를 찾습니다. 이 저장소를 재현할 때는 현재 고정된 1.616.1을 복원하고, 최신 버전으로 올리는 작업은 별도 변경·검증으로 진행합니다. 아래 이미지는 설치 당시 화면입니다.')
e.block(0,'$(SolutionDir)packages\\Microsoft.Direct3D.D3D12.1.616.1\\build\\native\\','plain text',caption='현재 vcxproj의 추가 포함 디렉터리 — 아래 include 경로와 한 쌍')
e.para('모든 설정이 완료되었으면 프로젝트를 빌드하여','패키지 복원 후 x64 Debug/Release에서 include·link 경로를 확인합니다. 헤더가 컴파일된 것만으로 Agility SDK 런타임 선택과 배포까지 검증된 것은 아닙니다.')
rewrite_tail(e,'## Agility SDK란 무엇인가',r'''## Agility SDK는 어떤 DLL을 선택하는가?

시스템의 `d3d12.dll`은 로더 역할을 하며, 설정에 따라 앱이 배포한 `D3D12Core.dll`을 사용합니다. 앱 폴더의 임의의 d3d12.dll로 운영체제 파일을 대체하는 방식이 아닙니다. [Microsoft: Agility SDK 시작하기](https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/)

현재 소스에서 확인한 것은 CORE의 패키지 1.616.1 참조와 CORE/Window/Editor 프로젝트의 헤더 경로입니다. 소스 검색에서는 `D3D12SDKVersion`·`D3D12SDKPath` export 정의를 찾지 못했습니다. 따라서 패키지 설치만으로 app-local 런타임이 활성화됐다고 단정하지 않습니다. 최종 EXE의 export와 로드한 DLL도 확인해야 합니다.

### 전통적인 EXE export 방식의 예시

```c++
// 선택한 패키지와 함께 제공된 D3D12Core.dll의 SDK 버전에 맞춰야 합니다.
// 1.616.x 계열의 예시. 실제 배포 파일과 대응 여부를 확인합니다.
extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 616;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}
// EXE에 정확히 한 번 정의합니다. 정적 라이브러리에 두고 누락되지 않도록 합니다.
```

위 내용은 **배포 구성 예시**이며 이 검수에서 엔진 소스를 변경한 것은 아닙니다. 대응하는 `D3D12Core.dll`을 EXE 기준 `D3D12` 하위 폴더에 배치합니다. 디버깅할 때는 같은 SDK 버전의 `d3d12SDKLayers.dll`도 맞춰 사용합니다. 최신 SDK가 제공하는 다른 선택 API를 쓸 경우 해당 SDK의 지침을 따릅니다.

### 지원 기능과 실행 환경

Mesh Shader·Sampler Feedback·Enhanced Barriers·Work Graphs 같은 기능은 선택한 SDK 버전과 OS·드라이버·GPU 지원을 함께 확인합니다. Agility SDK를 포함했다고 모든 장치에서 사용할 수 있는 것은 아닙니다. DirectStorage는 별도 API/SDK입니다.

NuGet 패키지의 props/targets가 include와 DLL 복사를 어디까지 처리하는지 확인하고, CORE 정적 라이브러리뿐 아니라 실행 EXE 프로젝트의 산출물도 검사합니다. Debug/Release·x64 등의 구성마다 동일한 버전이 참조되는지 확인합니다. 지원 기준과 다운로드는 [공식 Agility SDK 페이지](https://devblogs.microsoft.com/directx/directx12agility/)에서 확인합니다.
''')
e.finish()
