e=Edit(39,'선택 부록: D3D12 백버퍼 위에 기존 D3D11·Direct2D 콘텐츠를 합성하는 11On12 상호 운용 방식을 살펴봅니다. 현재 YamYam의 네이티브 DX12 경로와 구분합니다.','CreateWrappedResource의 In/Out 상태와 Acquire → Draw → Release → Flush 순서를 설명합니다.')
media=list(re.finditer(r'(?m)^!\[[^\n]*$',e.body))
assert len(media)==2
intro='''## 1. 11On12를 사용하는 목적

11On12는 기존 D3D11 API 호출을 D3D12 위에서 실행하도록 돕는 상호 운용 계층입니다. DX12가 만든 백버퍼에 Direct2D 텍스트를 덧그리거나, 기존 DX11 기능을 점진적으로 옮길 때 사용할 수 있습니다. DX12 입문의 필수 선행 단계는 아니며, 현재 YamYam은 네이티브 DX12와 ImGui 백엔드를 사용합니다.

DX11 쪽 상태 설정을 번역해 준다고 D3D12 Device·Queue·SwapChain 생성과 공유 리소스 동기화가 사라지지는 않습니다. 성능 향상이나 DX11 API만으로 Async Compute 사용이 자동으로 보장되는 것도 아닙니다.

이 부록은 [Microsoft D3D1211On12 샘플](https://github.com/microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D1211On12)을 설명합니다. `D3D1211on12`, `Win32Application`, `ThrowIfFailed`, `NAME_D3D12_OBJECT`는 샘플의 클래스·헬퍼입니다. 마지막 매크로는 디버깅용 객체 이름을 붙이는 용도입니다.

## 2. 초기화와 명령 큐

초기화는 D3D12 Debug Layer → DXGI Factory → Adapter/Device → DIRECT Queue → SwapChain 순서로 준비합니다. 자세한 일반 DX12 초기화는 <mention-page url="https://app.notion.com/p/1bd0b1ffa61e80978742dd1613e0efc6"/>에서 확인합니다. 어댑터 선택 후 모든 HRESULT를 확인하고, 실패한 초기화 상태로 렌더링하지 않습니다.

```c++
D3D12_COMMAND_QUEUE_DESC queueDesc = {};
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
ThrowIfFailed(m_d3d12Device->CreateCommandQueue(&queueDesc,
    IID_PPV_ARGS(&m_commandQueue)));
```

그림: CPU가 기록한 Command List를 Queue에 제출하는 구조입니다.
'''
middle='''
DIRECT는 그래픽·Compute·복사 작업을 지원합니다. COMPUTE도 일부 복사 명령을 지원하므로 “Compute Shader만 실행”하는 큐로 설명하지 않습니다. 여러 큐를 사용할 때 실행 중첩과 이득은 GPU에 따라 달라지고 공유 데이터에는 별도 동기화가 필요합니다.

## 3. D3D11On12와 Direct2D 연결

```c++
// D2D와 함께 사용할 D3D11 장치에는 BGRA 지원 플래그가 필요합니다.
UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
IUnknown* queues[] = {m_commandQueue.Get()};
ComPtr<ID3D11Device> d3d11Device;
ThrowIfFailed(D3D11On12CreateDevice(m_d3d12Device.Get(), flags,
    nullptr, 0, queues, 1, 0, d3d11Device.GetAddressOf(),
    m_d3d11DeviceContext.GetAddressOf(), nullptr));
ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));
ComPtr<IDXGIDevice> dxgiDevice;
ThrowIfFailed(d3d11Device.As(&dxgiDevice));
// 준비한 D2D Factory에서 이 DXGI Device를 바탕으로 D2D Device/Context 생성.
```

잘못된 descriptor handle 경고를 일반적으로 무시하도록 필터링하지 않습니다. 과거 샘플의 특정 런타임 문제 회피 설정이 필요하면 그 조건을 확인한 뒤 제한적으로 적용합니다. 메시지를 숨겨도 잘못된 descriptor가 유효해지는 것은 아닙니다.

## 4. 백버퍼 RTV와 래핑 리소스

RTV Heap에는 백버퍼 수만큼의 슬롯을 만들고 `GetDescriptorHandleIncrementSize`로 간격을 구합니다. RTV/DSV Heap은 shader-visible이 아니며 CPU handle을 통해 명령에 지정합니다. CBV/SRV/UAV와 Sampler는 각각 맞는 Heap 종류를 사용합니다.

그림: 리소스 본체와 리소스에 접근하는 descriptor의 관계입니다.
'''
loop=e.blocks[18][3].strip()
tail='''
### 백버퍼별 초기화 예제

아래는 Microsoft 샘플의 초기화 부분입니다. `bitmapProperties`는 대상 D2D 비트맵 옵션·알파 모드·DPI를 설정한 구조체입니다. 과거 GetDesktopDpi 예제를 사용할 때는 시스템 DPI와 per-monitor DPI를 혼동하지 말고 창의 DPI 변경에 맞춰 비트맵·텍스트 배율을 갱신합니다.

```c++
'''+loop+'''
```

`CreateWrappedResource`의 **InState=RENDER_TARGET**은 DX11에 넘길 때 기대하는 상태이고, **OutState=PRESENT**는 DX11 사용 후 반환할 상태입니다. D3D11이 PRESENT 상태에서 그린다는 뜻이 아닙니다. 래핑과 D2D surface/bitmap 생성은 같은 백버퍼를 연결하며, 픽셀 전체를 새 이미지로 복사하는 단계가 아닙니다.

## 5. 한 프레임의 공유 순서

```mermaid
flowchart LR
    D[D3D12 3D Draw 제출] --> A[AcquireWrappedResources]
    A --> U[D2D BeginDraw · UI · EndDraw]
    U --> R[ReleaseWrappedResources]
    R --> F[D3D11 Context Flush]
    F --> P[SwapChain Present]
    P --> S[Fence Signal과 재사용 대기]
```

```c++
// D3D12 목록은 먼저 제출하고 백버퍼를 InState인 RENDER_TARGET에 둡니다.
ID3D11Resource* wrapped[] = {m_wrappedBackBuffers[m_frameIndex].Get()};
m_d3d11On12Device->AcquireWrappedResources(wrapped, 1);
m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_frameIndex].Get());
m_d2dDeviceContext->BeginDraw();
// 이 위치에서 D2D DrawText/DrawBitmap 등의 UI 명령을 기록합니다.
const HRESULT drawResult = m_d2dDeviceContext->EndDraw();
m_d3d11On12Device->ReleaseWrappedResources(wrapped, 1);
m_d3d11DeviceContext->Flush();
ThrowIfFailed(drawResult); // 실제 앱에서는 D2DERR_RECREATE_TARGET 복구 경로도 준비
ThrowIfFailed(m_swapChain->Present(1, 0));
// 이어서 이 Queue의 작업을 포함하는 Fence를 Signal합니다.
```

Release는 OutState로 전환할 명령을 준비하고 Flush가 D3D11 측 명령을 공유 Queue에 제출합니다. Flush가 GPU 완료까지 기다리는 것은 아닙니다. 다음 프레임에 Allocator·리소스를 재사용하거나 리사이즈·종료할 때는 해당 제출을 포함한 Fence 완료를 확인합니다. [Microsoft: ReleaseWrappedResources](https://learn.microsoft.com/en-us/windows/win32/api/d3d11on12/nf-d3d11on12-id3d11on12device-releasewrappedresources)

프레임별 Allocator가 있어도 같은 Allocator를 GPU 사용 중 Reset하면 안 됩니다. 리사이즈 때는 D2D target, bitmap, wrapped resource와 D3D12 백버퍼 참조를 함께 정리하는 수명 관리가 필요합니다. [Microsoft: Direct3D 11 on 12](https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-11-on-12)

## 6. 현재 엔진으로 이어가기

YamYam의 현재 UI 경로는 11On12/D2D 대신 DX12 ImGui backend에 명령을 기록합니다. 공통점은 게임 렌더 뒤 UI를 합성하고 최종 제출을 기준으로 리소스 수명을 관리한다는 점입니다. <mention-page url="'''+NEW2+'''"/>
'''
e.replace(e.body[media[1].end():],tail)
e.replace(e.body[media[0].end():media[1].start()],middle)
e.replace(e.body[:media[0].start()],intro)
e.finish()
