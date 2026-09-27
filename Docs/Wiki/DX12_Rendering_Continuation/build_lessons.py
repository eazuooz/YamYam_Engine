from pathlib import Path
import json, textwrap

ROOT = Path(__file__).resolve().parents[3]
OUT = Path(__file__).resolve().parent
REV = 'ce56f16dd7a40fd2b4bf03c680790e5b637c7530'
BASE = f'https://github.com/eazuooz/YamYam_Engine/blob/{REV}/'
SYNC = 'https://app.notion.com/p/3400b1ffa61e8054a80bc1d4b575b515'
GODOT = 'https://docs.godotengine.org/en/stable/getting_started/introduction/first_look_at_the_editor.html'

def read(path):
    data = (ROOT/path).read_bytes()
    try: return data.decode('utf-8-sig').replace('\r\n', '\n')
    except UnicodeDecodeError: return data.decode('cp949').replace('\r\n', '\n')

def code(path, start, stop=None, language='c++'):
    source = read(path)
    begin = source.index(start)
    end = source.index(stop, begin) if stop else len(source)
    line = source.count('\n', 0, begin) + 1
    snippet = textwrap.dedent(source[begin:end].rstrip())
    return f'[{Path(path).name} · 실제 코드 발췌]({BASE}{path}#L{line})\n\n```{language}\n{snippet}\n```'

def table(headers, rows):
    return '<table header-row="true" fit-page-width="true">\n' + '\n'.join('<tr>' + ''.join(f'<td>{v}</td>' for v in row) + '</tr>' for row in [headers]+rows) + '\n</table>'

p1 = f'''이전 글: <mention-page url="{SYNC}"/>

이전 단계에서는 CPU가 기록하는 프레임과 GPU가 실행하는 프레임의 순서를 맞췄습니다. 이번에는 그 위에 **텍스처 → 렌더 타깃 → Scene / Game 패널**을 연결합니다. DX11에서 만들었던 편집 화면의 역할은 유지하고, 그림을 저장하고 표시하는 경로를 DX12 방식으로 바꾸는 작업입니다.

기준 코드는 [49e04e5 — 프레임 동기화·에디터 렌더링](https://github.com/eazuooz/YamYam_Engine/commit/49e04e5e875f04cfb792ea8a71e10667be78ad41)와 [ce56f16 — PSO·표시용 SRV](https://github.com/eazuooz/YamYam_Engine/commit/ce56f16dd7a40fd2b4bf03c680790e5b637c7530)입니다. 아래 코드 블록은 두 번째 커밋의 실제 소스에서 필요한 부분을 발췌했습니다. 생략된 주변 코드는 파일 링크에서 확인할 수 있습니다.

## 1. 우리가 만들려는 에디터

게임을 제작할 때는 물체를 배치하는 화면과 플레이어가 보는 화면이 필요합니다. Scene에서는 편집용 카메라로 장면을 살펴보고, Game에서는 게임 카메라의 결과를 확인합니다. 두 화면을 같은 프로그램 안에서 다루는 것이 이번 작업의 목표입니다.

### 참고 화면: Godot의 편집기

![Godot 공식 문서의 편집기 화면](https://docs.godotengine.org/en/stable/_images/editor_intro_editor_empty.webp)

출처: [Godot 공식 문서 — First look at Godot’s interface]({GODOT}), 2026-09-15 열람. 외부 엔진의 참고 화면입니다. 중앙 작업 화면, Scene 트리, Inspector, FileSystem과 하단 도구의 역할을 비교하기 위해 사용했습니다. Godot의 Scene 도크는 노드 목록이며, 우리 엔진의 Scene 뷰와 이름만 보고 같은 기능으로 보면 안 됩니다.

### 현재 화면: YamYam Engine

<image src="file-upload://3dc0b1ff-a61e-812f-a38e-00b2708c728b"></image>

2026-09-15 실제 Debug 실행 화면. Game 패널을 띄워 Scene과 함께 확인했습니다. 두 검은 영역 안의 스프라이트는 각각의 카메라 렌더 결과입니다. 작은 스프라이트는 현재 테스트 씬의 배치이며, 완성된 게임 장면을 뜻하지 않습니다.

{table(['사용 목적', 'Godot에서 참고할 부분', 'YamYam의 현재 상태와 목표'], [
['장면 편집', '중앙 2D / 3D 작업 화면', 'Scene 패널 + 독립 EditorCamera + 전용 렌더 타깃 연결'],
['실행 결과 확인', 'Game 화면', '게임 카메라 결과를 Game 패널에 표시'],
['오브젝트 선택·속성 편집', 'Scene 도크 + Inspector', 'Hierarchy·Inspector 패널은 있으나 전체 편집 기능의 완성을 뜻하지 않음'],
['파일·로그 관리', 'FileSystem + 하단 패널', 'Project·Console을 확장해 제작 흐름을 갖추는 것이 목표']])}

비교 기준은 화면의 역할입니다. Godot의 내부 렌더러가 아래 DX12 코드와 같은 구조라고 가정하지 않습니다.

## 2. 화면을 먼저 텍스처에 그린다

렌더 타깃(Render Target, RT)은 GPU가 그림을 써 넣을 목적지입니다. 윈도우의 백버퍼에 바로 그리면 프로그램 전체 화면에만 출력됩니다. Scene과 Game을 ImGui 패널 안에 넣으려면 각 뷰의 그림을 별도 텍스처에 먼저 저장한 뒤, 그 텍스처를 패널에 표시해야 합니다.

```mermaid
flowchart LR
    S[같은 게임 장면] --> GC[게임 카메라]
    S --> EC[편집용 카메라]
    GC --> G[Game 컬러 RT + 깊이 버퍼]
    EC --> E[Scene 컬러 RT + 깊이 버퍼]
    G --> GI[Game ImGui::Image]
    E --> EI[Scene ImGui::Image]
    GI --> B[에디터 백버퍼]
    EI --> B
    UI[메뉴와 편집 도구] --> B
    B --> P[Present]
```

현재 코드의 분리 단위는 **Scene 뷰와 Game 뷰**입니다. 등록된 게임 카메라 각각에 RT를 하나씩 만드는 구조는 아닙니다. 여러 게임 카메라가 있다면 Game용 타깃에 순서대로 렌더링합니다.

## 3. Texture와 descriptor는 무엇이 다른가?

`ID3D12Resource`에는 실제 픽셀 데이터가 있습니다. Descriptor는 GPU에게 그 리소스를 어떤 용도로 사용할지 설명하는 정보입니다. 같은 컬러 텍스처를 그릴 때는 RTV로, 읽을 때는 SRV로 참조합니다.

{table(['이름', '하는 일', '이번 코드에서 사용'], [
['RTV', '컬러 렌더링 출력 대상으로 연결', 'Scene / Game 컬러 텍스처에 그리기'],
['DSV', '깊이·스텐실 출력 대상으로 연결', '앞뒤 가림을 판단하는 깊이 버퍼'],
['SRV', '셰이더에서 리소스 읽기', '스프라이트 샘플링, ImGui 이미지 표시'],
['UAV', '셰이더의 일반적인 읽기·쓰기', '생성 인터페이스는 있으나 이 화면 표시 경로에서는 사용하지 않음']])}

`GraphicDevice_DX12`는 엔진과 ImGui가 함께 쓰는 shader-visible SRV/UAV 힙을 관리합니다. 슬롯 수는 4096개이며, 오프스크린 RTV와 DSV는 각각 256개입니다. `DescriptorAllocator`가 빈 인덱스를 배정하고 반납된 인덱스를 재사용합니다. 핸들 간격은 하드코딩하지 않고 디바이스에서 얻습니다.

**CPU 핸들**은 descriptor를 만들거나 RTV·DSV를 바인딩할 때 사용합니다. **GPU 핸들**은 셰이더가 사용할 descriptor 위치를 가리킵니다. `ImGui::Image`에 넘겨야 하는 것은 GPU SRV 핸들이며, 텍스처 포인터나 CPU RTV 핸들이 아닙니다.

파일 텍스처는 DirectXTex로 WIC/DDS/TGA를 읽고, 현재 지원 범위인 2D 이미지의 mip 0을 RGBA8로 준비합니다. Upload 버퍼에서 Default 힙 텍스처로 복사한 뒤 `PIXEL_SHADER_RESOURCE` 상태로 전환합니다. 이 로더는 업로드 완료를 기다리므로 임시 Upload 버퍼가 먼저 해제되지 않습니다. 비동기 스트리밍 로더를 완성한 단계는 아닙니다.

### 셰이더에 전달할 자리를 정한다

{code('YamYamEngine_CORE/yaGraphicDevice_DX12.cpp', '        CD3DX12_DESCRIPTOR_RANGE textureRange;', '\n\t\tCD3DX12_ROOT_SIGNATURE_DESC')}

`b0`에는 변환 행렬, `t0`에는 텍스처, `s0`에는 샘플러를 연결합니다. Root parameter 번호와 HLSL 레지스터 번호는 별도 개념입니다. 이 코드에서는 root parameter 0이 b0, root parameter 1이 t0 테이블을 담당합니다. 텍스처를 실제로 연결하는 부분은 다음과 같습니다.

{code('YamYamEngine_CORE/yaTexture.cpp', '    void Texture::Bind(', '\n}')}

## 4. 상수 버퍼는 프레임별·Draw별로 공간을 나눈다

World / View / Projection 행렬은 오브젝트와 카메라마다 다릅니다. 하나의 GPU 주소에 값을 계속 덮어쓰면, CPU가 여러 Draw를 기록한 후 GPU가 읽을 때 마지막 값만 남을 수 있습니다. Fence로 이전 프레임을 기다리는 것만으로는 **같은 프레임 안에서의 덮어쓰기**가 해결되지 않습니다.

현재 `TransformCB` 데이터는 행렬 3개로 192바이트입니다. 각 Draw의 시작 주소는 256바이트 간격으로 배치합니다. 두 프레임 슬롯이 각각 Upload 페이지를 소유하고, 한 페이지를 채우면 다음 페이지를 추가합니다. 이 데이터 크기에서는 64 KiB 페이지 하나에 256번의 Draw가 들어갑니다.

```mermaid
flowchart TB
    F0[프레임 슬롯 0] --> A[Draw 0: Game 오브젝트 A]
    F0 --> B[Draw 1: Game 오브젝트 B]
    F0 --> C[Draw 2: Scene 오브젝트 A]
    F1[프레임 슬롯 1] --> D[다음 프레임의 별도 저장 공간]
    A --> X[b0에 서로 다른 GPU 주소 바인딩]
    B --> X
    C --> X
```

{code('YamYamEngine_CORE/yaConstantBuffer.cpp', '    void ConstantBuffer::SetData(', '\n    void ConstantBuffer::Bind(')}

`BeginFrame()`은 재사용할 프레임의 GPU 완료를 기다린 뒤 `NextDraw`를 0으로 돌립니다. `Bind()`는 방금 할당한 `mCurrentAddress`를 root CBV에 기록합니다. 여기서 말하는 상수 버퍼 분리는 **메모리 영역과 수명의 분리**입니다. World·카메라 데이터가 서로 다른 HLSL cbuffer로 나뉜 것은 아니며, 현재는 하나의 TransformCB에 함께 들어갑니다.

## 5. Scene 카메라로 별도 RT에 그리기

`RenderTarget`은 컬러 텍스처와 깊이 텍스처를 묶습니다. `Bind()`에서 컬러를 `RENDER_TARGET`으로 전환하고, RTV·DSV를 설정하고, 화면을 지우고, 타깃 크기에 맞는 viewport와 scissor를 지정합니다. 렌더링이 끝나면 `Unbind()`에서 컬러를 `PIXEL_SHADER_RESOURCE`로 바꿉니다.

{code('YamYamEngine_CORE/yaRenderTarget.cpp', '    void RenderTarget::Unbind()', '\n    void RenderTarget::RequestResize(')}

Scene 패널은 독립적인 `EditorCamera`를 소유합니다. 편집용 카메라를 게임 씬의 카메라 목록에 등록하지 않으므로, Scene을 움직였다고 게임 카메라 구성이 바뀌지 않습니다. 카메라 투영 행렬의 화면 비율도 현재 RT의 크기를 사용합니다.

{code('Editor_Window/guiSceneWindow.cpp', '        auto* frameBuffer = mEditorCamera->GetRenderTarget();', '\n\t\t// To do : guizmo')}

이 짧은 구간에 연결 순서가 모두 있습니다. 패널 크기를 요청하고 → RT에 그린 뒤 → 읽기 상태로 바꾸고 → GPU SRV 핸들을 `ImGui::Image`에 넘깁니다. 화면에 보이지 않는 Scene 패널은 앞부분의 검사에서 렌더링을 생략합니다.

## 6. Game 뷰와 크기 변경의 시점

게임은 `Application::Render()`에서 먼저 `renderer::FrameBuffer`에 그립니다. 이후 에디터 UI가 그 결과를 표시합니다. 따라서 Game 패널을 그리는 순간 RT를 즉시 교체하면 방금 그린 텍스처를 잃을 수 있습니다. `RequestResize()`로 크기만 저장하고 다음 `Bind()`에서 반영하도록 바꿨습니다.

{code('Editor_Window/guiEditorApplication.cpp', '            // Game was rendered earlier in this frame:', '\n            if (ImGui::BeginDragDropTarget())')}

Game은 보통 다음 프레임의 Bind에서, Scene은 자기 렌더 패스 직전의 Bind에서 크기를 적용합니다. 너비·높이가 0이거나 8192를 넘는 요청은 무시합니다. 교체된 이전 텍스처를 GPU가 읽고 있을 수 있으므로 해제 시점도 Fence와 연결해야 합니다. 이 부분은 다음 글에서 이어집니다.

Game 입력은 패널이 보이고, 포커스가 있고, 마우스가 올라왔을 때 활성화합니다. 에디터에서 메뉴나 Scene을 조작하는 동안 게임 오브젝트까지 동시에 움직이는 일을 줄이기 위한 연결입니다.

## 7. 여기까지 완성된 연결

Scene / Game의 독립된 렌더 결과, 텍스처 샘플링, Draw별 상수 버퍼, ImGui GPU SRV 연결이 동작합니다. 게임 전용 실행 경로는 백버퍼에 직접 렌더링하며, 에디터 모드에서 오프스크린 Game RT를 사용합니다.

다음 글에서는 화면이 보이는 것에 더해 **불투명·컷아웃·반투명의 결과가 올바른지**, **ImGui가 완성된 색을 다시 섞지 않는지**, **창 크기를 바꿔도 GPU 리소스가 안전한지**를 확인합니다.
'''

p2 = f'''이전 글의 Texture·RenderTarget·상수 버퍼 연결을 바탕으로, DX11에서 구현했던 렌더링 순서를 DX12의 PSO에 연결합니다. 기준은 [ce56f16 커밋](https://github.com/eazuooz/YamYam_Engine/commit/ce56f16dd7a40fd2b4bf03c680790e5b637c7530)입니다. 코드 블록은 해당 커밋의 실제 소스 발췌이며, 설명용 계산은 별도로 표시했습니다.

<image src="file-upload://3dc0b1ff-a61e-8187-a7a6-00b283ddf60d"></image>

2026-09-15 실제 에디터의 Scene 패널. 이 화면은 RT와 ImGui 연결을 확인하는 캡처입니다. 알파·깊이·정렬의 정확성은 화면을 눈으로 보는 것과 별도로 GPU 픽셀 읽기 테스트로 검증했습니다.

## 1. DX11에서 만들었던 순서를 되살린다

그리는 순서는 그대로 유지합니다. **Opaque → CutOut → Transparent** 순으로 목록을 처리하며, 매번 현재 카메라 위치를 기준으로 정렬합니다. 같은 장면을 반대쪽에서 보는 카메라는 반투명 오브젝트의 앞뒤 순서도 달라질 수 있기 때문입니다.

{table(['모드', '정렬', 'RGB 블렌딩', '깊이 비교 / 기록', '알파 처리'], [
['Opaque: 불투명', '가까운 것부터', '끔', 'LessEqual / 켬', '알파가 0이어도 셰이더에서 버리지 않음'],
['CutOut: 잘라내기', '가까운 것부터', '끔', 'LessEqual / 켬', '알파가 0.01보다 작은 픽셀 버림'],
['Transparent: 반투명', '먼 것부터', 'SrcAlpha / InvSrcAlpha', 'Always / 끔', '낮은 알파도 블렌딩에 사용']])}

현재 Transparent의 `Always`는 예전 엔진의 동작을 복구한 설정입니다. 불투명 물체 뒤에 있는 반투명 오브젝트도 그 위에 합성됩니다. 일반적인 3D 반투명 렌더러가 흔히 사용하는 깊이 검사와 차이가 있으므로, 이후 정책을 바꿀 때는 이 동작을 의도적으로 변경해야 합니다.

{code('YamYamEngine_CORE/yaRenderer.cpp', '\t\tCollectRenderables(scene, opaqueList, cutoutList, transparentList);', '\n\t}\n\n\tvoid CollectRenderables')}

정렬 기준은 오브젝트 위치와 카메라 위치 사이의 거리입니다. 교차하는 삼각형이나 큰 반투명 메시까지 완벽하게 해결하는 픽셀 단위 정렬은 아닙니다. 또한 현재 정렬은 `RenderSceneFromCamera()`에 전달된 씬의 목록 안에서 이루어집니다.

## 2. Material이 공유 Shader의 상태를 바꾸면 안 된다

이전 연결은 Material의 모드를 바꾸면서 Shader의 blend·depth 설정도 변경하는 방식이었습니다. 두 Material이 같은 Shader를 공유하면 한쪽 변경이 다른 쪽에도 영향을 줄 수 있습니다. DX12에서는 이미 생성한 PSO에 그 상태가 들어 있으므로, 멤버 값만 바꾸는 것으로 실제 GPU 설정이 바뀌지도 않습니다.

현재 Material은 자신의 모드만 기억합니다. 그릴 때 그 모드를 Shader에 전달해 맞는 PSO를 선택합니다.

{code('YamYamEngine_CORE/yaMaterial.cpp', '\tvoid Material::SetRenderingMode(', '\n}')}

{code('YamYamEngine_CORE/yaMaterial.cpp', '\tvoid Material::BindShader()', '\n\tvoid Material::BindTextures()')}

`Shader`는 rasterizer·blend·depth 조합을 키로 PSO를 보관합니다. 일반적인 세 모드는 로딩할 때 준비하고, 추가 조합은 첫 사용 때 생성해 캐시합니다. 모드를 바꿀 때 기존 PSO를 지우지 않으므로 이미 기록된 GPU 명령이 참조하던 PSO도 유지됩니다.

{code('YamYamEngine_CORE/yaShader.cpp', '    void Shader::Bind(eRenderingMode mode)', '\n}')}

기본 rasterizer는 양면을 그리는 `SolidNone`입니다. Wireframe이나 다른 culling 상태도 캐시 키에 포함합니다. 엔진이 하나의 PSO만 전역으로 바꿔 가며 쓰는 구조에서, Shader가 필요한 PSO를 소유하는 구조로 바뀌었습니다.

## 3. CutOut과 Transparent를 셰이더에서도 구분한다

CutOut은 알파가 작은 픽셀을 아예 버립니다. Transparent는 픽셀을 버리지 않고 배경과 섞습니다. 모든 모드에 무조건 `clip()`을 적용하면 아주 옅은 반투명 픽셀까지 사라집니다.

{code('Shaders_SOURCE/SpriteDefaultPS.hlsl', '', language='hlsl')}

같은 소스를 `YA_ALPHA_TEST` 정의 유무에 따라 컴파일합니다. CutOut PSO는 정의가 있는 픽셀 셰이더를 사용하고 Opaque·Transparent는 일반 셰이더를 사용합니다. 사용자 정의 픽셀 셰이더도 CutOut을 지원하려면 이 분기를 구현해야 합니다.

일반 Material의 생성 기본값은 Opaque입니다. 기존 스프라이트의 투명 영역 잘라내기를 유지하기 위해 `Sprite-Default-Material`은 명시적으로 CutOut으로 설정합니다. 반투명 스프라이트는 해당 오브젝트용 Material에 Transparent를 지정하는 방식으로 사용합니다.

## 4. ImGui에서 알파를 두 번 적용하지 않기

반투명 오브젝트를 RT에 그릴 때 이미 색이 섞였습니다. 이 RT의 알파를 그대로 ImGui에 넘기면 ImGui가 그 알파로 다시 합성할 수 있습니다.

**설명용 계산:** 빨강을 알파 0.5로 초록 배경 위에 그리면 RGB는 `(0.5, 0.5, 0)`이 됩니다. 현재 엔진의 알파 블렌드 식은 `ONE / ZERO`이므로 RT 알파에는 마지막 통과 픽셀의 0.5가 저장됩니다. 이를 ImGui가 검정 위에 다시 0.5로 섞으면 RGB가 `(0.25, 0.25, 0)`으로 어두워집니다. 이 알파 채널은 누적된 화면 커버리지가 아닙니다.

해결 방법은 **표시할 때만 알파가 1로 읽히는 SRV**입니다. 원본 컬러 텍스처의 픽셀을 수정하거나 복사하지 않습니다. 같은 리소스에 별도의 descriptor를 만들고, component mapping에서 알파만 1로 고정합니다.

{code('YamYamEngine_CORE/yaRenderTarget.cpp', '    D3D12_GPU_DESCRIPTOR_HANDLE RenderTarget::GetDisplaySRV()', '\n    void RenderTarget::RetireDisplaySRV()')}

`GetDisplaySRV()`는 에디터 화면 표시용입니다. 원래 RGBA가 필요한 후속 패스는 attachment의 일반 `GetSRV()`를 사용할 수 있습니다. 이 구분 덕분에 Opaque 모드의 알파 0 픽셀도 에디터에서 사라지지 않고, 이미 합성한 색은 그대로 보입니다.

```mermaid
flowchart LR
    R[하나의 컬러 텍스처] --> S[일반 SRV: 원본 RGBA]
    R --> D[표시용 SRV: 원본 RGB + 알파 1]
    S --> O[원본 알파가 필요한 셰이더]
    D --> I[Scene / Game ImGui::Image]
```

## 5. 크기가 바뀌어도 이전 텍스처를 바로 지우지 않는다

CPU가 창 크기를 변경할 때 GPU는 이전 프레임의 이미지를 읽고 있을 수 있습니다. 텍스처뿐 아니라 SRV 슬롯까지 즉시 재사용하면 이전 명령이 다른 리소스를 읽는 문제가 생깁니다.

현재는 Texture 또는 표시용 SRV를 폐기할 때 `RetireResource()`에 넘겨 보관합니다. 처음의 `UINT64_MAX`는 아직 어떤 완료 Fence가 이 자원의 사용을 덮는지 정해지지 않았다는 뜻입니다. 프레임의 명령 제출이 끝난 뒤 Fence 값을 붙이고, GPU가 그 값에 도달하면 리소스와 descriptor 슬롯을 반납합니다.

{code('YamYamEngine_CORE/yaGraphicDevice_DX12.cpp', '    void GraphicDevice_DX12::RetireResource(', '\n    void GraphicDevice_DX12::UploadTexture(')}

프레임 중간의 동기식 텍스처 업로드도 Fence를 Signal할 수 있습니다. 하지만 그 Signal 뒤에 아직 제출하지 않은 Scene·ImGui 명령이 있을 수 있으므로, 업로드 완료 값을 프레임 전체의 폐기 기준으로 사용하지 않습니다. `SealRetiredResources()`는 최종 프레임 완료 Signal에서만 호출합니다.

**Resource barrier**는 GPU가 리소스를 어떤 상태로 접근할지 전환합니다. **Fence**는 CPU가 메모리·descriptor·command allocator를 언제 재사용해도 되는지 판단하는 완료 기준입니다. 둘은 서로 다른 문제를 해결합니다.

## 6. 프레임 동기화와 에디터 전체 흐름

```mermaid
flowchart TB
    A[재사용할 프레임의 Fence 대기] --> B[allocator / list 초기화, CB 할당 위치 초기화]
    B --> C[Game RT 렌더링]
    C --> D[보이는 Scene RT 렌더링]
    D --> E[백버퍼 복원, ImGui 합성, 리스트 Close]
    E --> F[메인 리스트와 분리된 ImGui 창 제출]
    F --> G[Present, 최종 Fence Signal]
    G --> H[EndOfFrame: 씬 이벤트 처리]
```

ImGui의 별도 운영체제 창도 엔진과 같은 command queue를 사용하도록 로컬 DX12 백엔드를 수정했습니다. RT 생성 명령과 이미지 소비 명령의 제출 순서가 같은 큐에서 이어지고, 최종 Fence가 둘을 포함하도록 하기 위해서입니다. 별도 창의 allocator·swapchain 등은 ImGui 백엔드가 계속 관리합니다. 패널을 붙이는 Docking과 별도 OS 창을 만드는 Multi-Viewports는 구분해서 이해해야 합니다.

{table(['함께 바뀐 부분', '변경 목적'], [
['Application::SetEditorMode', '에디터의 오프스크린 Game RT 경로와 게임 전용 백버퍼 경로 선택'],
['ImguiEditor::End', '백버퍼를 다시 바인딩한 뒤 ImGui를 그리고 PRESENT 전환·Close'],
['EndOfFrame 씬 이벤트 처리', 'Game과 Scene이 같은 프레임의 장면을 다룬 뒤 생성·삭제·전환 처리'],
['로딩 경로 정리', '공유 씬·리소스 컨테이너를 작업 스레드가 동시에 수정하던 경로를 주 렌더 스레드 중심으로 정리'],
['리소스 경로·빌드 후 복사', '실행 파일 기준으로 셰이더·텍스처를 찾고 Debug / Release에서 필요한 자산 제공'],
['셰이더 컴파일 실패 검사', '실패한 blob으로 PSO 생성을 진행하지 않고 원인 확인 가능']])}

## 7. 무엇을 검증했는가?

2026-09-14에 Debug / Release x64 빌드와 WARP·하드웨어 smoke test를 통과했습니다. 2026-09-15에는 해당 코드의 Debug 실행 화면을 캡처하고 하드웨어 smoke test를 다시 실행했습니다. 문서 정리 중에 엔진 코드를 변경하거나 전체 빌드를 다시 수행한 것은 아닙니다.

```plain text
PASS: hardware, 8 frames, 2 frame slots, 303 draws/frame, two camera outputs,
texture upload, resize, deferred descriptor reuse, ImGui image composition,
Opaque/CutOut/Transparent pixels and depth, per-camera sorting,
shared-shader mode changes, zero D3D12 errors
```

위 출력은 줄바꿈만 정리했습니다. [DX12RenderingSmoke.cpp]({BASE}Tests/DX12RenderingSmoke.cpp)의 테스트는 GPU 결과를 읽어 색과 깊이를 검사합니다. 프레임당 303 Draw로 상수 버퍼 한 페이지를 넘는 경우, 두 카메라의 다른 정렬 결과, CutOut 경계 알파, 공유 Shader의 모드 변경, RT 리사이즈 후 descriptor 재사용, 실제 ImGui 합성까지 확인합니다.

```powershell
.\u005cTests\u005cRun-DX12RenderingSmoke.ps1
.\u005cTests\u005cRun-DX12RenderingSmoke.ps1 -SkipBuild -Hardware
```

첫 명령은 빌드 후 WARP로 실행하고, 두 번째는 기존 실행 파일로 하드웨어 검증을 수행합니다. 스크립트: [Run-DX12RenderingSmoke.ps1]({BASE}Tests/Run-DX12RenderingSmoke.ps1).

## 8. 다음 구현의 출발점

현재 연결이 완성됐다고 해서 에디터 전체가 완성된 것은 아닙니다. `RenderTarget::ReadPixel()`은 아직 0을 반환하므로 오브젝트 ID 기반 picking은 별도의 readback 구현이 필요합니다. MSAA RT, 전체 mip·배열·큐브 텍스처 처리, 비동기 스트리밍, Hierarchy·Inspector의 편집 완성도는 후속 작업입니다.

이제 화면에 오브젝트를 올바르게 그리는 기반을 갖췄으므로, 다음에는 **선택 → 속성 편집 → 저장·복원** 흐름을 연결해 Godot과 비교했던 제작 도구의 역할을 채워 갈 수 있습니다.
'''

pages = [
    {'title':'DX12 Texture·RenderTarget 구현과 Scene / Game 뷰 연결', 'filename':'01_Texture_RenderTarget_Scene_Game.md', 'content':p1},
    {'title':'DX12 렌더링 모드 복구와 ImGui 화면 합성 완성', 'filename':'02_PSO_ImGui_Lifetime.md', 'content':p2}
]
for p in pages:
    (OUT/p['filename']).write_text(p['content'], encoding='utf-8', newline='\n')
(OUT/'lessons.json').write_text(json.dumps(pages, ensure_ascii=False, indent=2), encoding='utf-8', newline='\n')
print(json.dumps([{'title':p['title'], 'chars':len(p['content']), 'code_blocks':p['content'].count('```')//2} for p in pages],ensure_ascii=False))
