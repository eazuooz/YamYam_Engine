from edit import Edit,H,MEDIA,FENCE
import json,re

META={
46:('3dc0b1ffa61e814e9e8bd403ca145ebe','DX12 Texture·RenderTarget 구현과 Scene / Game 뷰 연결'),
47:('3dc0b1ffa61e8153b02fd321cc48e655','DX12 렌더링 모드 복구와 ImGui 화면 합성 완성'),
'hub11':('80d76736a5d348f4ac4a77a983fd7050','DIRECTX 11 강의 목차'),
'hub12':('1b90b1ffa61e80f49175e7028cd6c881','DIRECTX 12 강의 목차')}
class NewEdit(Edit):
    def __init__(self,i):
        assert i in META
        self.i=i;self.meta=dict(id=META[i][0],title=META[i][1],group='course')
        self.data=json.loads((H/'current'/f'{i}.json').read_text(encoding='utf-8'))
        self.original=self.data['text'].split('<content>\n')[1].split('\n</content>')[0]
        self.body=self.original;self.updates=[]
        self.codes=[m.group() for m in FENCE.finditer(self.body)]
        self.media=[m.group() for m in MEDIA.finditer(self.body)]

if __name__=='__main__':
    e=NewEdit(46)
    e.replace('이전 단계에서는 CPU가 기록하는 프레임과 GPU가 실행하는 프레임의 순서를 맞췄습니다. 이번에는 그 위에 **텍스처 → 렌더 타깃 → Scene / Game 패널**을 연결합니다. DX11에서 만들었던 편집 화면의 역할은 유지하고, 그림을 저장하고 표시하는 경로를 DX12 방식으로 바꾸는 작업입니다.', '''이전 강의에서 프레임 동기화를 마쳤다면 ImGui 창과 삼각형은 그릴 수 있습니다. 이제 삼각형이 있던 자리에 게임 장면을 넣고, 그 옆에는 같은 장면을 자유롭게 살펴볼 Scene 화면을 만들고 싶습니다. 여기서 첫 번째 질문이 생깁니다. **“게임을 그린 결과를 어떻게 ImGui 창 안으로 가져올까?”**

답을 찾으려면 이미지가 저장되는 곳부터 따라가야 합니다. 파일에서 읽은 픽셀을 GPU 텍스처에 올리고, 카메라가 그리는 목적지도 텍스처로 만든 다음, 그 결과를 ImGui가 읽게 연결합니다. 두 카메라가 서로 다른 화면을 만들도록 Draw마다 행렬을 저장할 공간도 나눕니다. 이 글에서는 이 연결을 **Texture → descriptor → 상수 버퍼 → RenderTarget → ImGui::Image** 순서로 직접 따라가겠습니다.''')
    e.replace('렌더 타깃(Render Target, RT)은 GPU가 그림을 써 넣을 목적지입니다. 윈도우의 백버퍼에 바로 그리면 프로그램 전체 화면에만 출력됩니다. Scene과 Game을 ImGui 패널 안에 넣으려면 각 뷰의 그림을 별도 텍스처에 먼저 저장한 뒤, 그 텍스처를 패널에 표시해야 합니다.', '''렌더 타깃(Render Target, RT)은 GPU가 그림을 써 넣을 목적지입니다. 백버퍼도 렌더 타깃이며 viewport를 나누면 그 일부에만 그릴 수 있습니다. 다만 편집기에서는 패널을 옮기거나 크기를 바꾸고, 같은 결과를 다른 UI와 겹쳐 표시해야 합니다. 그래서 카메라의 결과를 별도 텍스처에 먼저 저장하고, ImGui에게 그 텍스처를 하나의 이미지처럼 배치하도록 맡깁니다.

예를 들어 게임 카메라는 플레이어 앞을 보고, 편집용 카메라는 장면 전체를 멀리서 봅니다. 물체 데이터는 하나지만 **바라보는 카메라와 그림을 저장할 목적지는 둘**입니다. 아래 그림에서 두 경로가 마지막 에디터 백버퍼에서 만나는 이유도 여기에 있습니다.''')
    e.replace('### 셰이더에 전달할 자리를 정한다', '''### 파일의 픽셀을 GPU가 읽는 텍스처로 옮기기

DirectXTex가 읽은 `image->pixels`는 CPU 메모리입니다. 이 주소를 `ImGui::Image`에 넣어도 GPU 텍스처가 되지 않습니다. 현재 엔진은 Default 힙에 실제 텍스처를 만들고, CPU가 쓸 수 있는 Upload 버퍼를 경유해 픽셀을 복사합니다.

```mermaid
flowchart LR
    F[PNG · DDS · TGA 파일] --> C[DirectXTex: CPU의 RGBA8 픽셀]
    C --> U[Upload 버퍼: 복사에 맞는 행 간격]
    U -->|GPU 복사 명령| T[Default 힙 텍스처]
    T --> V[SRV: 이 텍스처를 읽는 방법]
    V --> S[픽셀 셰이더에서 샘플링]
```

다음은 포맷 변환을 마친 뒤 실행하는 실제 `Texture::Load()`의 끝부분입니다. 여기서 `image`는 mip 0의 RGBA8 이미지입니다. `Create()`는 텍스처와 SRV를 준비하고, `UploadTexture()`는 그 텍스처에 넣을 데이터를 전달받습니다.

```c++
if (!Create(UINT(image->width), UINT(image->height), DXGI_FORMAT_R8G8B8A8_UNORM)) return E_FAIL;
D3D12_SUBRESOURCE_DATA data = { image->pixels, LONG_PTR(image->rowPitch), LONG_PTR(image->slicePitch) };
GetDevice()->UploadTexture(mTexture.Get(), data, mState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
mState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
return S_OK;
```

`rowPitch`는 원본 이미지에서 한 행을 건너가는 바이트 수입니다. `slicePitch`는 이미지 한 면의 데이터 크기입니다. 원본 행 간격과 GPU 복사용 버퍼의 행 간격은 다를 수 있으므로 둘 다 무조건 `너비 × 4`라고 취급하면 안 됩니다. 엔진의 `UploadTexture()`는 `GetRequiredIntermediateSize()`와 `UpdateSubresources()`를 사용해 복사 공간과 배치를 처리합니다. 이 정렬과 복사 방식은 [Microsoft의 텍스처 업로드 설명](https://learn.microsoft.com/en-us/windows/win32/direct3d12/upload-and-readback-of-texture-data)에서도 확인할 수 있습니다.

현재 컬러 텍스처는 생성 시 `PIXEL_SHADER_RESOURCE` 상태입니다. 업로드 함수는 이를 `COPY_DEST`로 바꿔 복사하고, 다시 셰이더 읽기 상태로 되돌립니다. 별도 allocator와 command list로 업로드를 제출하고 완료를 기다리므로, 현재 기록 중인 프레임 명령을 Reset하지도 않고 임시 Upload 버퍼가 복사보다 먼저 사라지지도 않습니다.

### 픽셀 저장소에 읽기용 descriptor 만들기

다음 실제 함수는 새 픽셀을 복사하지 않습니다. 이미 만든 `mTexture`에 대해 **“2D RGBA 텍스처의 mip 하나를 이렇게 읽어라”**라는 SRV를 힙의 한 슬롯에 써 넣습니다.

```c++
bool Texture::CreateSRV()
{
    if (!mTexture || (mTexture->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) return false;
    if (!mSrv) mSrv = GetDevice()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = mFormat;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipLevels = 1;
    GetDevice()->GetID3D12Device()->CreateShaderResourceView(mTexture.Get(), &desc, mSrv.Cpu);
    return true;
}
```

마지막 줄이 `mSrv.Cpu`를 받는 이유는 CPU가 descriptor를 작성하기 때문입니다. 반면 Draw 명령에는 같은 슬롯의 `mSrv.Gpu`를 연결합니다. 두 핸들은 이미지가 두 벌이라는 뜻이 아니라, 같은 descriptor 슬롯을 CPU와 GPU에서 각각 참조하는 값입니다. 힙의 시작 핸들에 `슬롯 인덱스 × 디바이스가 알려 준 간격`을 더해 위치를 계산합니다. [Microsoft의 descriptor heap 설명](https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-descriptor-heaps)

### 셰이더에 전달할 자리를 정한다''')
    e.code(2,after='''`SetDescriptorHeaps()`는 사용할 힙을 선택하고, `SetGraphicsRootDescriptorTable(1, …)`은 그 안에서 이 Draw가 읽을 SRV 슬롯을 지정합니다. 힙만 선택하고 테이블 위치를 설정하지 않으면 어떤 텍스처를 읽을지 연결이 끝나지 않습니다. 현재 함수의 stage·slot 인자는 사용하지 않으며, 스프라이트 경로인 픽셀 셰이더의 `t0` 한 자리에 고정되어 있습니다.''')
    e.code(4,before='''오브젝트 A와 B를 Game에서 한 번씩, Scene에서 한 번씩 그린다고 해 봅시다. 아래 주소는 페이지 시작점으로부터의 **설명용 오프셋**입니다.

| Draw | 대상 | 저장할 행렬 | 바이트 오프셋 |
| --- | --- | --- | --- |
| 0 | Game의 A | A의 World + Game의 View / Projection | 0 |
| 1 | Game의 B | B의 World + Game의 View / Projection | 256 |
| 2 | Scene의 A | A의 World + Scene의 View / Projection | 512 |
| 3 | Scene의 B | B의 World + Scene의 View / Projection | 768 |

A의 World는 같아도 카메라 행렬이 다르므로 Draw 0과 Draw 2가 같은 주소를 덮어써서는 안 됩니다. 아래 코드의 `pageIndex`가 사용할 페이지를 고르고, `offset`이 그 안의 Draw 위치를 고릅니다.''',after='''`NextDraw = 255`일 때는 첫 페이지의 마지막 256바이트 구간을 씁니다. `NextDraw = 256`이면 `pageIndex = 1`, `offset = 0`이 되어 다음 페이지를 사용합니다. 첫 페이지의 내용을 덮어쓰는 것이 아닙니다. `memcpy`가 복사하는 데이터는 192바이트이며, 다음 Draw의 시작점을 256바이트에 맞추기 위해 남는 간격을 둡니다.

현재 HLSL은 세 행렬을 `row_major matrix`로 선언하고, CPU의 World / View / Projection을 전치하지 않고 복사합니다. 정점 셰이더는 위치에 World → View → Projection 순으로 곱합니다. 예전 강의의 다른 행렬 저장 관례를 섞어 CPU에서 다시 전치하면 Scene과 Game의 화면이 잘못될 수 있습니다.''')
    e.replace('`BeginFrame()`은 재사용할 프레임의 GPU 완료를 기다린 뒤 `NextDraw`를 0으로 돌립니다.', '`BeginFrame()` 자체가 GPU를 기다리지는 않습니다. 주 루프가 재사용할 프레임 슬롯의 GPU 완료를 먼저 기다리고, 그 뒤 호출한 `BeginFrame()`이 해당 슬롯의 `NextDraw`를 0으로 돌립니다. 완료를 확인했으므로 그 슬롯의 페이지를 다시 써도 되는 것입니다.')
    e.code(5,after='''`OMSetRenderTargets(0, …)`는 그리기 대상으로 연결했던 RTV를 해제하는 호출입니다. 이것만으로 텍스처의 상태가 바뀌지는 않으므로 각 컬러 attachment에 별도의 transition barrier도 기록합니다. 그래야 뒤에 오는 ImGui 픽셀 셰이더가 완성된 이미지를 읽는 순서가 성립합니다. 이 과정은 CPU가 GPU 전체를 기다리는 작업이 아닙니다.''')
    e.code(6,after='''`RenderSceneFromCamera()`는 같은 씬의 물체를 `mEditorCamera`의 View / Projection으로 다시 그립니다. 별도의 게임 오브젝트 복사본을 만드는 것이 아닙니다. `GetDisplaySRV()`가 돌려주는 GPU 핸들의 `ptr` 값을 현재 ImGui DX12 백엔드가 texture ID로 사용합니다. `ImGui::Image` 호출 시점에는 UI 그리기 정보가 쌓이며, 실제 이미지를 읽는 GPU 명령은 뒤의 ImGui 렌더링 단계에서 기록됩니다. 따라서 Image를 호출했다고 텍스처나 descriptor를 바로 해제하면 안 됩니다.''')
    e.replace('## 7. 여기까지 완성된 연결', '''## 7. 직접 움직여 보며 연결 확인하기

두 패널이 보이면 먼저 Scene의 편집용 카메라만 움직여 봅니다. Scene의 구도만 변하고 Game은 그대로라면 카메라가 분리된 것입니다. 이어서 같은 오브젝트의 Transform을 바꾸면 두 화면에서 그 물체의 위치가 함께 바뀌어야 합니다. 두 카메라가 같은 장면 데이터를 사용하기 때문입니다.

Game 패널의 너비를 늘리면 크기 요청은 다음 Game 렌더 패스에 반영됩니다. Scene은 자기 렌더 직전에 크기를 반영합니다. 이때 RT 해상도뿐 아니라 Projection의 화면 비율도 함께 바뀌어야 원이 타원처럼 늘어나지 않습니다. 두 패널이 항상 같은 구도로 보이면 카메라 행렬과 CB 주소를, 한 패널에 다른 패널의 그림이 나오면 RT와 GPU SRV 핸들을 차례로 확인합니다.

### 여기까지 완성된 연결''')
    e.save()

    e=NewEdit(47)
    e.replace('현재 Material은 자신의 모드만 기억합니다. 그릴 때 그 모드를 Shader에 전달해 맞는 PSO를 선택합니다.', '''예를 들어 벽 Material과 유리 Material이 같은 Sprite Shader를 사용한다고 해 봅시다. 벽은 Opaque, 유리는 Transparent여야 합니다. 유리의 모드를 설정하면서 공유 Shader의 기본 blend 상태를 바꾸면 벽도 그 설정의 영향을 받습니다. 반대로 Shader의 멤버 값만 바꾸고 기존 PSO를 계속 바인딩하면 화면에는 변경이 반영되지 않습니다.

그래서 역할을 나눕니다. **Material은 “나는 어떻게 그릴 것인가”를 기억하고, Shader는 “그 모드로 그릴 PSO가 무엇인가”를 찾아 줍니다.** 벽 Draw는 Opaque PSO를, 유리 Draw는 Transparent PSO를 고르되 셰이더 소스와 캐시는 함께 사용합니다.''')
    e.code(1,after='''여기서는 유효한 모드인지 검사하고 `mMode`만 바꿉니다. blend나 depth를 공유 Shader에 써 넣는 호출이 없다는 점을 봅시다. 실제 GPU 상태의 선택은 아래 `BindShader()`가 실행되는 Draw 준비 시점으로 미룹니다.''')
    e.code(3,after='''`switch`의 결과는 rasterizer·blend·depth 세 상태의 조합입니다. 이어지는 오버로드는 그 조합의 PSO를 캐시에서 찾아 command list에 설정합니다. 이미 생성한 PSO의 내부 설정을 수정하는 방식이 아니라, **설정이 다른 PSO를 선택**하는 방식입니다. 따라서 유리 Material의 모드를 Opaque로 바꿨다가 되돌려도 벽 Material의 `mMode`나 기존 PSO를 훼손할 이유가 없습니다.''')
    e.code(4,replacement=e.codes[4].replace('```javascript\n','```c++\n// HLSL\n',1),after='''`clip(x)`는 `x`가 음수인 픽셀을 버립니다. 따라서 경계가 알파 0.01일 때 결과를 직접 계산할 수 있습니다. 아래 알파는 텍스처 색과 정점 색을 곱한 뒤의 `color.a`입니다.

| 알파 | CutOut의 clip 인자 | CutOut 결과 | Transparent의 RGB 결과 |
| --- | --- | --- | --- |
| 0 | -0.01 | 픽셀을 버림 | 배경 RGB 유지 |
| 0.005 | -0.005 | 픽셀을 버림 | 전경 0.5% + 배경 99.5% |
| 0.5 | 0.49 | 픽셀을 그리고 깊이 기록 | 전경 50% + 배경 50% |

CutOut에서 알파 0.5가 통과했다고 반투명하게 보이는 것은 아닙니다. 이 모드는 RGB 블렌딩이 꺼져 있으므로 통과한 픽셀의 색을 그대로 씁니다. 풀잎의 빈 부분처럼 **있거나 없는 경계**에는 CutOut을, 유리처럼 **배경과 섞여 보여야 하는 색**에는 Transparent를 쓰는 이유입니다. Opaque에는 이 clip 분기가 없으므로 알파 0도 자동으로 구멍이 되지 않습니다.''')
    e.code(0,after='''불투명 A가 카메라에서 2만큼, B가 8만큼 떨어져 있으면 A → B 순입니다. 깊이를 먼저 기록한 A 뒤의 픽셀을 나중에 버릴 수 있습니다. 반투명은 같은 거리에서 B → A 순으로 그립니다. 예를 들어 검정 위에 반투명 빨강을 먼저, 반투명 파랑을 나중에 각각 알파 0.5로 그리면 RGB는 `(0.25, 0, 0.5)`입니다. 순서를 뒤집으면 `(0.5, 0, 0.25)`가 됩니다. 합성 순서가 색을 바꾸기 때문에 카메라마다 목록을 다시 정렬합니다.''')
    e.replace('**Resource barrier**는 GPU가 리소스를 어떤 상태로 접근할지 전환합니다.', '''숫자를 넣어 수명을 따라가 봅시다. 아래 Fence 값과 슬롯 번호는 설명용입니다. `ImGui::Image`가 이전 RT의 SRV 슬롯 17을 참조하는 UI 정보를 이미 만들었지만, 그 정보를 읽을 GPU 명령은 아직 모두 제출되지 않은 상황입니다.

```mermaid
sequenceDiagram
    participant C as CPU
    participant Q as 같은 GPU Queue
    C->>C: 이전 RT와 슬롯 17을 폐기 대기열에 보관
    C->>Q: 중간 텍스처 업로드 제출 + Signal 11
    Q-->>C: 업로드 완료 11
    C->>C: 슬롯 17은 아직 반납하지 않음
    C->>Q: 메인 화면 및 분리된 ImGui 창 명령 제출
    C->>Q: 최종 Signal 12
    C->>C: 폐기 항목의 기준을 12로 확정
    Q-->>C: 완료 값이 12에 도달
    C->>C: 이전 RT 해제 + 슬롯 17 재사용 가능
```

업로드 11이 끝났다는 사실은 그 뒤에 제출할 ImGui 작업까지 끝났다는 뜻이 아닙니다. 그래서 `UINT64_MAX`인 항목은 `CollectRetiredResources()`가 건너뜁니다. 최종 Signal 12를 붙인 뒤에도 즉시 지우지 않고 `GetCompletedValue() >= 12`가 될 때 반납합니다. 텍스처만 보관하고 슬롯 17을 새 텍스처로 덮어써도 기존 UI가 다른 이미지를 읽을 수 있으므로, 리소스와 descriptor 양쪽의 수명을 함께 관리합니다.

**Resource barrier**는 GPU가 리소스를 어떤 상태로 접근할지 전환합니다.''')
    e.replace('## 7. 무엇을 검증했는가?', '''## 7. 화면이 맞는지 직접 확인하는 방법

우선 겹치는 스프라이트 둘을 놓고 알파와 모드를 바꿔 봅니다. Opaque는 알파 0이어도 색을 쓰고, CutOut은 경계 아래의 픽셀을 버리며, Transparent는 배경과 색을 섞어야 합니다. 현재 Transparent는 `Always`이므로 불투명 물체 뒤에서도 합성된다는 점까지 앞의 표와 일치해야 합니다. Game과 Scene의 카메라 위치를 반대로 두면 거리 정렬도 각각의 카메라를 따라야 합니다.

눈으로 보는 것만으로 찾기 어려운 문제도 있습니다. 한 페이지에 들어가는 256 Draw까지만 확인하면 다음 페이지로 넘어가는 오류를 놓칠 수 있습니다. 두 카메라에 서로 다른 행렬을 넣지 않으면 공유된 CB 주소 문제도 드러나지 않을 수 있습니다. 아래 테스트가 **303 Draw와 두 카메라**를 사용하는 이유입니다.

### 현재 소스에서 실행한 검증''')
    e.save()

    e=NewEdit('hub11')
    e.pattern(r'<callout[^>]*>.*?</callout>', '''<callout icon="💡" color="blue_bg">
	이 과정에서는 삼각형 하나를 화면에 띄우는 것부터 시작해, 텍스처가 있는 물체를 그리고 Scene 뷰에서 편집하는 게임엔진의 기반을 만듭니다. 각 강의에서는 먼저 화면에 어떤 변화가 필요한지 살펴보고, 그림으로 데이터의 흐름을 따라간 뒤, 그 일을 맡는 코드와 실행 결과를 연결합니다.
</callout>

## 강의를 따라가는 방법

처음에는 GPU가 정점을 픽셀로 바꾸는 과정을 배웁니다. 그다음 정점·인덱스·상수 버퍼를 직접 연결해 삼각형을 그리고, 반복되는 코드를 Shader와 Mesh 클래스로 옮깁니다. 클래스 이름을 먼저 외우기보다 **“앞 강의에서 직접 호출한 일을 이 클래스의 어느 함수가 맡았는가”**를 찾아보면 설계가 이해됩니다.

텍스처 단계에서는 파일의 픽셀, GPU 텍스처, SRV, UV와 Sampler를 차례로 연결합니다. 이어서 Material이 셰이더와 텍스처를 묶고, SpriteRenderer가 오브젝트의 Transform과 함께 실제 Draw를 요청합니다. 깊이와 블렌드를 배우면 물체가 겹쳤을 때 어떤 색이 남는지도 설명할 수 있습니다.

마지막에는 ImGui 창을 만들고 Scene 화면을 별도 렌더 타깃에 그립니다. 카메라와 기즈모, 입력과 이벤트를 연결하면서 화면을 보여 주는 프로그램을 장면을 편집하는 도구로 확장합니다. 코드를 읽은 뒤에는 강의의 작은 실험에서 값 하나를 바꾸고, 예상한 화면과 실제 결과가 같은지 확인해 보세요.

아래 본문은 DX11 단계의 구현을 중심으로 읽습니다. 현재 저장소가 DX12로 옮겨진 부분은 각 강의에 표시한 당시 소스와 비교하며 따라가면 됩니다.

## 강의 목록''')
    # Exact math/native reference segment is deliberately never an edit target.
    math_segment=e.original[e.original.index('<unknown'):e.original.index('<page',e.original.index('<unknown'))]
    assert math_segment in e.body
    e.save()

    e=NewEdit('hub12')
    e.replace('### DirectX 12 개론', '''## DX11에서 만든 엔진을 DX12로 옮기기

DX11에서 `Draw`를 호출할 때 드라이버가 대신 관리하던 일 가운데 일부를 이제 엔진이 직접 맡습니다. CPU가 명령을 기록하는 시점과 GPU가 실행하는 시점이 다르므로, 버퍼에 새 값을 쓸 때와 리소스를 지울 때도 그 차이를 생각해야 합니다. 이 과정에서는 이미 만든 Scene / Game 화면을 유지하면서 그 아래의 명령 제출·메모리·상태 관리를 DX12로 바꿉니다.

### 먼저 알아둘 개념

소개와 API 초기화에서 Device, Queue, Command List, Swapchain의 역할을 익힌 뒤 Raster Graphics Pipeline에서 Root Signature와 PSO를 연결합니다. Mesh Shader·Compute·Ray Tracing은 각기 다른 작업을 GPU에 맡기는 별도 주제입니다. 첫 삼각형과 에디터 연결을 따라가는 데 세 파이프라인을 먼저 구현할 필요는 없습니다.

### DirectX 12 개론''')
    e.replace('### 게임엔진 제작','''### 게임엔진 제작

먼저 Scene과 Game이 각각 어떤 카메라와 출력 화면을 가져야 하는지 정합니다. 라이브러리를 준비하고 첫 삼각형을 그린 다음 ImGui를 붙입니다. 버퍼와 파이프라인을 엔진 클래스에 연결한 뒤, 프레임 동기화에서 **GPU가 아직 읽는 메모리를 CPU가 덮어쓰지 않는 순서**를 완성합니다.

이어지는 두 강의는 그 기반으로 실제 에디터 화면을 만듭니다. 첫 글에서는 텍스처의 픽셀을 GPU에 올리고, Draw마다 상수 버퍼 주소를 나누고, Scene / Game RT를 `ImGui::Image`로 표시합니다. 두 번째 글에서는 Opaque·CutOut·Transparent를 PSO로 되살리고, ImGui의 알파 합성과 RT를 교체할 때의 수명까지 해결합니다. 함수 이름뿐 아니라 **왜 이 호출이 앞이나 뒤에 와야 하는지**를 그림과 계산 예제로 따라가 보세요.''')
    # All page links including the hidden 11On12 appendix retain their order.
    links=lambda x:re.findall(r'<(?:page|mention-page)\b[^>]*>(?:[^<]*</page>)?',x)
    assert links(e.body)==links(e.original)
    e.save()
