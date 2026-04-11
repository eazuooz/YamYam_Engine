# DX12 렌더링 파이프라인 구축 - Phase 1

> ConstantBuffer / IndexBuffer DX12 구현 및 SpriteRenderer 파이프라인 연결

---

## 시작하기 전에

이 문서는 YamYam Engine의 렌더링 시스템을 DirectX 11에서 12로 전환하는 과정 중 첫 번째 단계를 기록한다. 단순한 변경 사항 나열이 아니라, **왜 이런 결정을 내렸는지**, **DX12에서는 어떤 방식으로 동작하는지**를 이해할 수 있도록 작성했다.

---

## 1. 무엇이 문제였는가

전환 작업이 중단된 시점에서 엔진의 상태를 보면, 겉으로는 렌더링이 되는 것처럼 보였다. 삼각형이 화면에 나왔고 크래시도 없었다. 하지만 실제로는 많은 부분이 빈 껍데기였다.

가장 심각한 문제는 **ConstantBuffer**였다. DX11 시절에는 `SetData()`가 GPU 버퍼에 데이터를 올리고, `Bind()`가 셰이더 슬롯에 연결해줬다. 그런데 DX12로 전환하면서 이 두 함수의 내부가 전부 주석 처리되어 빈 함수가 되어버렸다. 즉, 매 프레임마다 `Transform::Bind()`를 호출해서 World / View / Projection 행렬을 열심히 계산하고 구조체에 담아도, 그 데이터는 GPU에 단 한 바이트도 전달되지 않고 있었다.

결과적으로 셰이더는 초기화되지 않은 메모리를 읽고 있었고, 오브젝트는 카메라나 트랜스폼 설정과 무관하게 NDC 원점 근처에 고정된 채로 렌더링되고 있었다.

**IndexBuffer**도 마찬가지였다. `Create()`는 인덱스 개수만 저장할 뿐 GPU 버퍼를 만들지 않았고, `Bind()`는 아무것도 하지 않았다. 그 상태에서 `DrawInstanced(6, 1, 0, 0)`을 호출했는데, 이 함수는 인덱스 버퍼 없이 버텍스 버퍼에서 순서대로 6개를 읽는다. 그런데 직전에 RectMesh를 4개 버텍스로 변경했기 때문에, 존재하지 않는 버텍스 4번, 5번을 읽으려 했고 그 결과 절반만 제대로 된 삼각형이 나오고 나머지는 깨진 쓰레기 픽셀이 출력됐다.

정리하면, 화면에 무언가 그려지고 있었지만 **올바른 이유로 그려지는 것이 아무것도 없는 상태**였다.

---

## 2. DX12에서 상수 버퍼는 어떻게 동작하는가

DX11에서는 상수 버퍼가 비교적 단순했다. `D3D11_USAGE_DYNAMIC`으로 버퍼를 만들고, `Map()`으로 열어서 데이터를 쓰고, `Unmap()`으로 닫은 뒤, `VSSetConstantBuffers()`로 슬롯에 연결하면 됐다. API가 내부적으로 많은 것을 추상화해줬다.

DX12는 다르다. DX12는 CPU와 GPU가 메모리를 어떻게 공유하는지를 **개발자가 직접 선택**해야 한다. 이를 위해 힙(Heap) 타입이라는 개념이 있다.

힙에는 크게 세 가지가 있다. `DEFAULT` 힙은 GPU만 접근할 수 있는 고속 VRAM 영역이다. `UPLOAD` 힙은 CPU가 쓰고 GPU가 읽는 영역으로, CPU에서 데이터를 올리기에 적합하지만 GPU 접근 속도는 DEFAULT보다 느리다. `READBACK` 힙은 GPU가 쓰고 CPU가 읽는 경우에 사용한다.

상수 버퍼는 매 프레임 CPU에서 행렬 데이터를 올려야 하므로 **UPLOAD 힙**이 적합하다. 정적인 메시 데이터라면 DEFAULT 힙에 올리는 게 맞지만, 상수 버퍼처럼 동적으로 갱신되는 데이터는 UPLOAD 힙을 사용한다.

```
CPU 메모리                       GPU 메모리
┌─────────────────┐             ┌─────────────────────────┐
│  C++ 코드        │             │  DEFAULT Heap (VRAM)    │
│  TransformCB    │             │  Mesh VB / IB 등        │
│  {World, View,  │             │  (GPU 전용, 고속)        │
│   Projection}   │             └─────────────────────────┘
└────────┬────────┘
         │ memcpy                ┌─────────────────────────┐
         └──────────────────────▶│  UPLOAD Heap            │
                                 │  ConstantBuffer         │
                                 │  (CPU ↔ GPU 공유 영역)  │
                                 └────────────┬────────────┘
                                              │ GPU가 읽음
                                              ▼
                                    Vertex Shader b0 슬롯
```

UPLOAD 힙의 가장 큰 특징은 `Map()`을 한 번 호출하면 버퍼가 살아있는 동안 포인터를 계속 유지할 수 있다는 것이다. DX11처럼 매번 Map/Unmap을 반복할 필요 없이, 포인터를 멤버 변수(`mMappedData`)로 저장해두고 `memcpy`로 데이터만 밀어 넣으면 된다. 이를 **Persistent Map** 방식이라고 한다.

```cpp
// 초기화 시 한 번만
buffer->Map(0, &readRange, &mMappedData);

// 매 프레임 SetData() 호출 시
memcpy(mMappedData, data, mSize);
```

또 한 가지 중요한 제약이 있다. DX12의 CBV는 반드시 **256바이트 단위로 정렬**되어 있어야 한다. 하드웨어 캐시 라인 구조에서 비롯된 제약이다. `TransformCB`의 실제 크기는 행렬 3개, 즉 192바이트다. 그런데 이 값을 그대로 사용하면 안 되고, 256바이트로 올림해서 버퍼를 만들어야 한다.

```
TransformCB 실제 데이터: 192 bytes
                         World    (64B)
                         View     (64B)
                         Projection (64B)

GPU 버퍼 실제 크기: 256 bytes
                         World    (64B)
                         View     (64B)
                         Projection (64B)
                         패딩      (64B) ← 하드웨어 정렬을 위한 여유 공간
```

정렬 계산은 비트 연산으로 깔끔하게 처리할 수 있다.

```cpp
UINT alignedSize = (size + 255) & ~255;
// 예: (192 + 255) = 447
//     447를 2진수로: 0001 1011 1111
//     ~255        : 1111 1111 0000 0000
//     AND 결과    : 0000 0001 0000 0000 = 256
```

---

## 3. RootSignature — 셰이더와 리소스를 연결하는 계약서

DX12에서 셰이더가 어떤 리소스를 어떤 슬롯에서 받을 것인지를 미리 선언하는 것이 **RootSignature**다. DX11에서는 `VSSetConstantBuffers(slot, ...)` 한 줄이면 됐지만, DX12에서는 그 전에 RootSignature를 통해 "이 파이프라인은 b0 슬롯에 CBV 하나를 받는다"고 계약을 맺어야 한다.

기존 코드에서는 RootSignature를 만들 때 파라미터를 0개로 초기화하고 있었다.

```cpp
// Before: 파라미터 없음
rootSignatureDesc.Init(0, nullptr, 0, nullptr, ...);
```

이 상태에서 `SetGraphicsRootConstantBufferView(0, gpuVA)`를 호출하면 root parameter 0번이 존재하지 않으므로 오류가 발생한다. CBV 파라미터를 추가해야 한다.

```cpp
// After: b0 레지스터에 CBV 1개 선언
CD3DX12_ROOT_PARAMETER rootParams[1] = {};
rootParams[0].InitAsConstantBufferView(0); // register(b0)
rootSignatureDesc.Init(1, rootParams, 0, nullptr, ...);
```

RootSignature는 PSO(Pipeline State Object) 생성 시 함께 사용되기 때문에, **RootSignature를 변경하면 PSO도 새로 만들어야** 한다. 다행히 현재 구조에서는 `GraphicDevice_DX12::Initialize()` → `renderer::LoadShaders()` → `Shader::Load()` 순서로 호출되기 때문에, RootSignature가 먼저 만들어지고 PSO가 그것을 참조하는 순서가 보장된다.

---

## 4. IndexBuffer — 왜 인덱스가 필요한가

사각형 하나를 그리려면 삼각형 두 개가 필요하고, 각 삼각형은 꼭짓점 3개를 사용하므로 총 6개의 꼭짓점 데이터가 필요하다. 그런데 사각형의 꼭짓점은 실제로 4개뿐이고, 두 삼각형이 모서리 2개를 공유한다.

인덱스 버퍼 없이 이 문제를 해결하려면 공유되는 꼭짓점을 중복으로 저장해야 한다. 기존 코드가 바로 그 방식이었다.

```
중복 버텍스 방식 (Before)          인덱스 방식 (After)

v0 (-0.5,  0.5)                    v0 (-0.5,  0.5)  ← 한 번만 저장
v1 ( 0.5,  0.5)                    v1 ( 0.5,  0.5)
v2 ( 0.5, -0.5)                    v2 ( 0.5, -0.5)
v3 (-0.5, -0.5)                    v3 (-0.5, -0.5)
v4 (-0.5,  0.5)  ← v0와 동일      
v5 ( 0.5, -0.5)  ← v2와 동일      Index: [0, 1, 2, 0, 2, 3]

DrawInstanced(6)                   DrawIndexedInstanced(6)
→ v0~v5 순서대로 읽기               → 인덱스 순서로 v0,v1,v2 / v0,v2,v3 읽기
```

지금이야 꼭짓점이 4개 vs 6개의 차이지만, 복잡한 3D 메시에서는 이 차이가 극적으로 커진다. 캐릭터 메시처럼 수만 개의 폴리곤이 있는 경우 인덱스 버퍼 없이는 수십 MB의 중복 데이터가 생긴다.

IndexBuffer도 ConstantBuffer와 마찬가지로 UPLOAD 힙에 생성한다. 다만 인덱스 데이터는 매 프레임 바뀌지 않으므로 Persistent Map 없이 생성 시 한 번만 데이터를 올리고 Unmap한다.

```
생성 시 (한 번만)
CPU: [0][1][2][0][2][3]  → Map → memcpy → Unmap
GPU UPLOAD Heap: [0][1][2][0][2][3]  (고정)

매 프레임
IB.Bind() → IASetIndexBuffer(&view)
DrawIndexedInstanced(6, ...)
  → GPU가 index 0,1,2 읽어서 v0,v1,v2로 삼각형
  → GPU가 index 0,2,3 읽어서 v0,v2,v3로 삼각형
```

---

## 5. Vertex 구조체와 UV

DX11 시절 주석 처리되어 있던 `uv` 필드를 살렸다. 이유는 SpriteRenderer가 텍스처를 샘플링할 때 UV 좌표가 필요하기 때문이다.

```cpp
struct Vertex {
    Vector3 pos;    // 12 bytes, offset  0
    Vector4 color;  // 16 bytes, offset 12
    Vector2 uv;     //  8 bytes, offset 28
};                  // total: 36 bytes
```

셰이더 입력 레이아웃도 함께 업데이트했다. 입력 레이아웃은 GPU가 버텍스 버퍼의 바이트 덩어리를 어떻게 해석할지를 정의한다. offset 값이 `Vertex` 구조체의 각 필드 위치와 정확히 일치해야 한다.

```
버텍스 버퍼 메모리 레이아웃 (36 bytes per vertex)

[ pos.x ][ pos.y ][ pos.z ][ col.r ][ col.g ][ col.b ][ col.a ][ uv.x ][ uv.y ]
    0         4         8       12       16       20       24       28       32
└──────────────────────┘└──────────────────────────────────────┘└────────────┘
       POSITION (12B)                  COLOR (16B)                TEXCOORD (8B)
```

UV 좌표는 텍스처의 어느 위치를 샘플링할지를 나타낸다. (0,0)이 텍스처의 좌상단, (1,1)이 우하단이다.

```
v0 (0,0) ──────── v1 (1,0)
    │                  │
    │   텍스처 공간     │
    │                  │
v3 (0,1) ──────── v2 (1,1)
```

---

## 6. SpriteRenderer 파이프라인 연결

ConstantBuffer와 IndexBuffer가 동작하게 됐으니, 이제 SpriteRenderer가 올바른 셰이더를 사용하도록 연결할 차례였다.

기존 SpriteRenderer는 `TriangleMaterial`을 사용하고 있었다. TriangleShader는 원래 학습용으로 만든 단순한 셰이더로, WVP 변환 없이 NDC 좌표를 그대로 출력한다. SpriteRenderer에 이것을 붙여두면 아무리 ConstantBuffer를 구현해도 WVP 변환이 적용되지 않는다.

`SpriteDefaultShader`는 이미 `ConstantBuffers.hlsli`를 포함하고 WVP 변환을 적용하도록 작성되어 있었다. 로드만 활성화하고 `Sprite-Default-Material`을 만들어 SpriteRenderer에 연결하면 된다.

```
연결 구조

SpriteRenderer
  ├─ Mesh: RectMesh
  │    ├─ VB: 4 vertices (pos + color + uv)
  │    └─ IB: 6 indices [0,1,2, 0,2,3]
  │
  └─ Material: Sprite-Default-Material
       └─ Shader: SpriteDefaultShader
            ├─ VS (SpriteDefaultVS.hlsl)
            │    └─ ConstantBuffers.hlsli (b0)
            │         World × View × Projection 적용
            │
            └─ PS (SpriteDefaultPS.hlsl)
                 └─ vertex color 출력 (텍스처 구현 전 임시)
                    TODO: sprite.Sample(anisotropicSampler, uv)
```

PS에서 텍스처 샘플링을 하려면 Sampler와 Texture SRV가 RootSignature에 등록되어 있어야 하고, 실제 Texture DX12 리소스도 생성되어 있어야 한다. 이는 다음 단계의 작업이므로, 현재는 vertex color를 그대로 출력하는 것으로 대체했다. TriangleVS는 학습용이므로 WVP 없이 원본 그대로 유지했다.

---

## 7. 완성된 프레임 흐름

지금까지의 변경을 합치면 한 프레임이 다음과 같이 흘러간다.

```
[프레임 시작]
    │
    ▼
ResetCommandAllocator()
ResetCommandList()              ← PSO가 커맨드 리스트에 자동 바인딩
SetBaseGraphicsRootSignature()  ← b0 CBV 파라미터를 가진 RootSignature 설정
BindViewportAndScissor()
TransitionBarrier(PRESENT → RT)
BindFrameBuffer()               ← swap chain RTV 바인딩 + Clear
    │
    ▼
SceneManager::Render()
  └─ Scene::Render()
       └─ for each Camera (SceneCamera)
              └─ RenderSceneFromCamera(scene, camera)
                   └─ Opaque 오브젝트 수집 → 거리순 정렬 → 렌더
                        └─ obj->Render(viewMatrix, projMatrix)
                             │
                             ├─ Transform::Bind(view, proj)
                             │    ├─ TransformCB.World = worldMatrix
                             │    ├─ TransformCB.View  = viewMatrix
                             │    ├─ TransformCB.Proj  = projMatrix
                             │    ├─ CB.SetData() → memcpy to UPLOAD heap ✅
                             │    └─ CB.Bind()   → SetGraphicsRootCBV(0) ✅
                             │
                             ├─ Mesh::Bind()
                             │    ├─ VB.Bind() → IASetVertexBuffers     ✅
                             │    └─ IB.Bind() → IASetIndexBuffer       ✅
                             │
                             ├─ Material::BindShader()
                             │    └─ (PSO는 이미 바인딩된 상태)
                             │
                             └─ BaseRenderer::Draw()
                                  └─ DrawIndexedInstanced(6,1,0,0,0)   ✅
    │
    ▼
CloseCommandList()    ← RT → PRESENT barrier
ExcuteCommandList()   ← GPU에 제출
Present()
MoveToNextFrame()     ← fence signal, frameIndex 전진
```

---

## 8. 남은 과제

이번 작업으로 렌더링 파이프라인의 핵심 골격이 동작하게 됐다. 하지만 아직 미완성인 부분이 있다.

**Texture** 구현이 빠져 있다. SpriteDefaultPS가 텍스처를 샘플링하지 못하고 있고, 이미지가 아닌 단색으로 렌더링된다. Texture는 DEFAULT 힙과 UPLOAD 힙을 함께 사용하는 방식으로 구현해야 하며, SRV(Shader Resource View) descriptor 할당도 필요하다.

**RenderTarget** 구현도 빠져 있다. 지금은 모든 카메라가 swap chain 백버퍼에 직접 그린다. 카메라별로 독립적인 오프스크린 RenderTarget을 만들어야 에디터의 Scene 뷰와 Game 뷰를 분리할 수 있다. RenderTarget은 Texture 구현이 선행되어야 한다.

**PSO 관리** 방식도 정리가 필요하다. 현재 `GraphicDevice_DX12`가 `mPipelineState`를 하나만 보유하고 있어서, 셰이더를 여러 개 로드하면 PSO가 덮어씌워진다. 각 `Shader`가 자신의 PSO를 직접 소유하고, `Material::BindShader()`에서 `commandList->SetPipelineState(shader->GetPSO())`를 호출하는 방식으로 바꿔야 한다.

| 작업 | 우선순위 | 선행 조건 |
|------|----------|-----------|
| Texture DX12 구현 | ★★★ | - |
| RenderTarget DX12 구현 | ★★★ | Texture |
| Camera별 RT 바인딩 분리 | ★★ | RenderTarget |
| Scene / Game 뷰 ImGui 연결 | ★★ | RenderTarget |
| Shader가 PSO 직접 소유하도록 정리 | ★★ | - |
