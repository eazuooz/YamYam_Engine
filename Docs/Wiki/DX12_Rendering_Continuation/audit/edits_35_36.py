e=Edit(35,'초기화 객체를 엔진 클래스에 연결하고 백버퍼에 삼각형을 그리는 단계입니다. 코드 블록은 역할별 예시이며 뒤의 프레임 동기화·상수 버퍼 글에서 현재 구조로 확장합니다.','실패한 초기화에서 Draw로 진행하지 않고, 최초 Fence 값·PSO·Root Signature·백버퍼 상태를 일관되게 맞춥니다.')
e.para('DirectX 12 디바이스는 GPU와의 모든 상호작용','아래 표는 관련 구성 요소의 역할을 나열한 것입니다. D3D12CreateDevice가 표의 순서대로 DXGI와 모든 드라이버 계층을 통과한다는 호출 스택은 아닙니다.')
e.para('- DirectX 12 Feature Level 12.0 이상','- 필요한 최소 Feature Level을 지원하는 GPU. 아래 삼각형 예제는 11_0을 요청합니다.')
e.para('WARP(Windows Advanced Rasterization Platform)는','WARP는 CPU 기반 소프트웨어 렌더러입니다. Direct3D 12에서도 사용할 수 있으며 지원 Feature Level은 WARP·런타임 버전에 따라 확인해야 합니다.')
e.para('WARP는 정확성은 보장하지만','WARP는 하드웨어 GPU 없이 테스트할 때 유용하지만 실행 속도와 지원 기능은 실제 환경에 따라 다릅니다. 고정된 10~100배 성능 차이나 모든 오류가 없다는 보장은 하지 않습니다.')
a=e.body.index('<table>',e.body.index('**Feature Level의 의미와 선택**'));b=e.body.index('</table>',a)+8
e.replace(e.body[a:b],'''| 확인할 조건 | 의미 |
| --- | --- |
| D3D12 API + 최소 Feature Level 11_0 | 이 삼각형 예제의 디바이스 생성 조건 |
| 더 높은 Feature Level | 정해진 기능 집합의 지원 조건. GPU 출시 연도나 특정 제조사 이름만으로 판단하지 않음 |
| 개별 기능 쿼리 | MeshShaderTier, RaytracingTier, ResourceBindingTier 등 사용할 기능을 CheckFeatureSupport로 확인 |

[Microsoft: Direct3D 하드웨어 Feature Level](https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-feature-levels)''')
e.para('- **동기화 객체**: Fence, Event','- **동기화 객체**: Device로 Fence를 만들고, CPU 대기용 Event는 Win32 CreateEvent로 만듭니다.')
e.para('- **파이프라인 객체**: Root Signature, PSO, 셰이더','- **파이프라인 객체**: Root Signature와 PSO. HLSL 컴파일은 별도 컴파일러를 사용합니다.')
e.para('<td>화면 출력을 위한 2개 이상의 백버퍼를','<td>Flip 모델에 필요한 백버퍼를 준비합니다. 버퍼 수만으로 화면 찢어짐 방지나 최소 입력 지연이 보장되지는 않습니다.</td>')
e.para('GPU가 새 프레임을 렌더링하는 도중 모니터의','화면을 스캔하는 동안 표시할 이미지가 바뀌면 한 화면에 서로 다른 프레임의 일부가 보일 수 있습니다. 이것이 tearing이며, Present 동기화·표시 모드·VRR 정책을 함께 고려합니다.')
e.para('렌더링이 완료되면 모니터의 수직 귀선','V-Sync를 사용하는 경우 표시 시점을 수직 동기화에 맞춥니다. Flip 모델의 실제 표시·합성은 OS와 DXGI가 관리하므로 모든 Present가 즉시 두 raw pointer를 교환하는 동작이라고 설명하지 않습니다.')
e.para('- **입력 지연 감소**: 다음 V-Sync','- **작업 공간 추가**: 준비할 버퍼가 늘어 CPU/GPU 대기를 줄일 여지가 있습니다. 큐에 더 많은 프레임이 쌓이면 입력 지연이 늘 수도 있습니다.')
e.para('<td>백버퍼 개수를 지정합니다. 2는','<td>Flip 모델의 백버퍼 개수입니다. 2 또는 3을 선택할 때 메모리·프레임 지연·표시 정책을 측정합니다. 장르만으로 특정 값을 정하지 않습니다.</td>')
e.para('<td>`RENDER_TARGET_OUTPUT`은','<td>RENDER_TARGET_OUTPUT은 렌더링 출력 용도입니다. FLIP_DISCARD 백버퍼의 Present 이후 내용을 TAA용 이전 프레임으로 보존한다고 가정하지 않습니다. 히스토리는 별도 텍스처로 관리합니다.</td>')
e.para('<td>멀티샘플 안티앨리어싱(MSAA) 설정입니다.','<td>Flip 모델 SwapChain은 Count=1, Quality=0을 사용합니다. MSAA는 별도 멀티샘플 RT에 그린 뒤 single-sample 결과로 Resolve합니다.</td>')
e.para('- `ResizeBuffers1()`:','- `ResizeBuffers1()`: 버퍼별 생성 노드와 Present Queue 등의 추가 인자를 지정하는 크기 변경 함수입니다.')
e.para('Descriptor Heap의 각 슬롯은 고정 크기를','Descriptor handle의 증가량은 Device와 Heap 타입에 따라 GetDescriptorHandleIncrementSize로 조회합니다. 제조사 이름을 보고 32바이트처럼 하드코딩하지 않습니다.')
e.para('Command List는 생성 시 "열린(Recording)"','CreateCommandList는 열린 기록 상태로 생성됩니다. 바로 명령을 기록해도 됩니다. 이 예제는 다음 프레임에서 Reset하는 흐름이므로 최초 목록을 Close해 둡니다.')
e.para('<td>32비트 상수 값을 Root Signature에','<td>Root parameter를 통해 32비트 값을 명령에 기록합니다. 일반 graphics/compute Root Signature의 전체 비용 한도는 64 DWORD이며 다른 parameter와 합산합니다. 값 자체가 Root Signature 객체에 고정 저장되는 것은 아닙니다.</td>')
e.para('<td>CBV, SRV, UAV 하나를 직접','<td>지원되는 CBV·버퍼 SRV/UAV를 GPU 가상 주소로 연결합니다. Texture SRV를 임의로 root descriptor에 넣을 수는 없습니다. 성능은 사용 패턴을 측정해 판단합니다.</td>')
e.para('<td>Descriptor Heap의 특정 범위를','<td>shader-visible Heap의 descriptor 범위를 연결합니다. Sampler 범위와 CBV/SRV/UAV 범위는 같은 descriptor table에 섞을 수 없습니다.</td>')
e.para('- **셰이더**: Vertex, Hull, Domain, Geometry, Pixel, Compute','- **셰이더**: Graphics PSO는 VS/HS/DS/GS/PS 등을, Compute PSO는 CS를 사용합니다. 둘을 한 Graphics PSO에 함께 넣지 않습니다.')
e.para('- 1: V-Sync 활성화, 60Hz','- 1: 일반적으로 한 refresh 간격에 동기화합니다. 60Hz에서 목표 상한이 60fps인 예이며 실제 fps 보장은 아닙니다.')
e.para('- 2: 30 FPS로 제한','- 2: 두 refresh 간격을 사용합니다. 60Hz 환경에서는 30fps에 해당하며 모든 모니터에서 고정 30fps라는 뜻은 아닙니다.')
e.para('DirectX 12 초기화는 복잡하고 많은 코드를','DirectX 12 초기화에서는 애플리케이션이 명령·메모리·동기화 정책을 명확하게 정해야 합니다. API를 바꾼 것만으로 DX11보다 항상 높은 성능을 얻는 것은 아닙니다.')
for n,b in enumerate(e.blocks):
    if 'assert(NULL &&' in b[3]:
        code=re.sub(r'assert\(NULL && ("[^"]*")\);',r'throw std::runtime_error(\1);',b[3])
        e.block(n,code,caption='초기화 실패 처리 포함 — stdexcept 필요' if n==0 else None)
e.block(2,'''// VRAM이 큰 지원 어댑터를 고르는 학습 정책. VRAM은 속도의 절대 기준이 아닙니다.
void GetHardwareAdapter(IDXGIFactory4* factory, IDXGIAdapter1** adapter)
{
    *adapter = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> selected;
    SIZE_T largest = 0;
    for (UINT i = 0; ; ++i)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> candidate;
        HRESULT hr = factory->EnumAdapters1(i, candidate.GetAddressOf());
        if (hr == DXGI_ERROR_NOT_FOUND) break;
        ThrowIfFailed(hr);
        DXGI_ADAPTER_DESC1 desc = {};
        ThrowIfFailed(candidate->GetDesc1(&desc));
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (FAILED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_11_0,
            __uuidof(ID3D12Device), nullptr))) continue;
        if (!selected || desc.DedicatedVideoMemory > largest)
        {
            selected = candidate;
            largest = desc.DedicatedVideoMemory;
        }
    }
    if (!selected) throw std::runtime_error("No supported adapter");
    *adapter = selected.Detach(); // 호출자가 COM 참조를 소유합니다.
}''')
e.block(3,'''if (mbUseWarpDevice)
{
    Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
    ThrowIfFailed(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
    ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&mDevice)));
}''')
e.block_replace(1,'// debugController->SetEnableGPUBasedValidation(true);','// ComPtr<ID3D12Debug1> debug1;\n        // if (SUCCEEDED(debugController.As(&debug1))) debug1->SetEnableGPUBasedValidation(true);')
e.block_replace(9,'swapChain.As(&mSwapChain);','ThrowIfFailed(swapChain.As(&mSwapChain));')
e.block_replace(12,'UINT64 FenceValue;','UINT64 FenceValue = 0;')
e.block_replace(15,'UINT64 mFenceValue = 0;','UINT64 mFenceValue = 1; // 다음에 Signal할 값. Fence의 초기 완료 값 0과 구분합니다.')
e.block_replace(16,'mCommandQueue->Signal(mFence.Get(), currentFenceValue);','ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), currentFenceValue));')
e.block_replace(18,'// Descriptor Table: 텍스처와 샘플러','// Descriptor Table: 텍스처만. Sampler는 별도 static sampler로 선언합니다.')
code=e.code_changes[18][0]
start=code.index('CD3DX12_DESCRIPTOR_RANGE ranges[2];');stop=code.index('// Root Signature를 바이너리',start)
code=code[:start]+'''CD3DX12_DESCRIPTOR_RANGE range;
range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);
rootParameters[2].InitAsDescriptorTable(1, &range);
CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
rootSigDesc.Init(3, rootParameters, 1, &sampler,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

'''+code[stop:]
code=code.replace('D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);','ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));').replace('mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));','ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));')
e.block(18,code)
e.block_replace(19,'psoDesc.DepthStencilState.DepthEnable = TRUE;','psoDesc.DepthStencilState.DepthEnable = FALSE; // 이 삼각형 예제는 DSV를 바인딩하지 않습니다.')
e.block_replace(19,'D3D12_COMPARISON_FUNC_LESS','D3D12_COMPARISON_LESS')
e.block_replace(19,'psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;','psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;')
e.block_replace(19,'mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO));','ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));')
e.block_replace(24,'mCommandList->SetGraphicsRootSignature(mRootSignature.Get());','// textureHeap의 t0~t3 descriptor가 유효하고 참조 리소스가 읽기 상태라고 가정합니다.\nID3D12DescriptorHeap* heaps[] = {textureHeap.Get()};\nmCommandList->SetDescriptorHeaps(1, heaps);\nmCommandList->SetGraphicsRootSignature(mRootSignature.Get());')
for n in (13,17,20,26):
    code=e.code_changes[n][0] if n in e.code_changes else e.blocks[n][3]
    code=re.sub(r'(?m)^(\s*)(mCommandList->(?:Close|Reset)\([^\n]*\)|mFrameContext\[[^\]]+\]\.CommandAllocator->Reset\(\)|mCommandQueue->Signal\([^\n]*\)|mSwapChain->Present\([^\n]*\));',r'\1ThrowIfFailed(\2);',code)
    e.block(n,code)
e.finish('### 예제 연결과 현재 소스\n\nWaitForPreviousFrame은 전체 제출 뒤 기다리는 단순 패턴이고, BeginFrame/EndFrame과 RenderFrame/PresentFrame은 대안적인 구조입니다. 전부 한 프레임에서 중복 호출하지 않습니다. RenderFrame의 WaitForFrame은 프레임 슬롯 Fence 완료를 확인하는 헬퍼를 뜻하며 앞의 대기 패턴으로 구현해야 합니다. Root/PSO 예제는 position·normal·UV 정점과 대응 셰이더를 전제로 하므로 PART2 Raster 글의 position·color 삼각형과 섞지 않습니다. 완전한 첫 삼각형 흐름은 <mention-page url="https://app.notion.com/p/1bd0b1ffa61e8097b9d6dda7ad3704a0"/>를, 현재 엔진 프레임은 <mention-page url="https://app.notion.com/p/3400b1ffa61e8054a80bc1d4b575b515"/>를 확인합니다.')

e=Edit(36,'ImGui를 DX12의 엔진 프레임 안에 연결합니다. 초기 통합 당시 중복되던 프레임 대기·제출 예제는 정리하고 현재 ImguiEditor의 책임을 기준으로 읽습니다.','엔진이 프레임 슬롯을 기다린 뒤 UI를 기록하고, Scene RT에서 백버퍼로 돌아와 합성하는 순서를 설명합니다.')
init=source_read('Editor_Window/guiImguiEditor.cpp');a=init.index('ImGui_ImplDX12_InitInfo init_info');b=init.index('ImGui_ImplDX12_Init(&init_info);',a)+len('ImGui_ImplDX12_Init(&init_info);')
init_code=textwrap.dedent(init[a:b]);line=init.count('\n',0,a)+1
rewrite_tail(e,'ImGui(Immediate Mode GUI)는',f'''ImGui는 매 프레임 UI를 함수 호출로 구성합니다. 내부에서는 창·입력·도킹 상태를 유지하고 DX12 백엔드는 Vertex/Index 버퍼와 텍스처를 관리합니다. 즉시 모드라고 상태가 없거나 GPU 동기화가 필요 없는 것은 아닙니다.

## 1. 초기화: 플랫폼과 렌더러를 나눈다

Win32 백엔드는 창 입력과 플랫폼 정보를, DX12 백엔드는 UI의 GPU 렌더링을 담당합니다. 저장소에 포함된 ImGui는 1.92.0 WIP 표기의 버전이며, 아래 코드는 이 저장소의 `ImGui_ImplDX12_InitInfo` 인터페이스를 기준으로 합니다. 다른 버전의 구형 6개 인자 초기화 예제와 섞지 않습니다.

[{Path('guiImguiEditor.cpp').name} · 실제 Initialize 발췌]({SOURCE_BASE}Editor_Window/guiImguiEditor.cpp#L{line})

```c++
{init_code}
```

`device`는 이 함수 앞에서 얻은 GraphicDevice_DX12 포인터입니다. 컨텍스트 생성, ConfigFlags, 스타일, Win32 초기화도 앞부분에서 수행합니다. `NumFramesInFlight=2`로 백엔드의 순환 버퍼 수를 정하고 엔진의 같은 shader-visible SRV Heap을 전달합니다. 할당 callback은 새로운 슬롯을, 해제 callback은 Fence에 따른 지연 반환을 연결합니다. 폰트 descriptor 하나만 놓고 Scene/Game 이미지까지 같은 슬롯에 덮어쓰면 안 됩니다.

현재 초기화 함수는 backend Init의 bool 반환을 검사하지 않습니다. 견고한 실패 처리에는 각 Init 결과를 확인하고 이미 생성한 컨텍스트·백엔드를 역순으로 정리하는 경로가 필요합니다. 위는 실제 코드 발췌이며 이 검수에서 엔진 코드를 변경한 것은 아닙니다.

## 2. FrameContext와 백버퍼의 관계

프레임 슬롯에는 Command Allocator와 마지막 사용 Fence 값을 둡니다. 해당 GPU 작업이 끝나기 전에는 Allocator와 같은 업로드 구간을 재사용하지 않습니다. 이번 엔진은 2개 슬롯을 사용합니다.

SwapChain의 BufferCount와 CPU가 선행할 FrameContext 수는 개념적으로 다릅니다. 서로 다른 개수로도 설계할 수 있으므로 2와 3이 다르다는 이유만으로 항상 오류인 것은 아닙니다. 다만 이 엔진처럼 같은 인덱스를 사용하는 코드에서는 배열·매핑과 ImGui 순환 리소스의 재사용 조건을 함께 맞춰야 합니다. 여러 프레임이 항상 2배 성능을 만드는 것도 아닙니다.

## 3. Begin: 이번 프레임의 UI 구성 시작

{actual('Editor_Window/guiImguiEditor.cpp','void ImguiEditor::Begin(')}

DX12 NewFrame은 필요한 backend 객체를 준비합니다. 실제 UI Vertex/Index 데이터 업로드와 Draw 기록은 RenderDrawData에서 수행합니다. Win32 NewFrame은 플랫폼 정보를 갱신하며 키 메시지 자체는 Win32 WndProc의 backend handler로 전달합니다.

```c++
// Begin과 End 사이에 매 프레임 호출하는 UI 예시.
if (ImGui::Begin("Debug Info"))
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
ImGui::End();
```

## 4. End: Scene RT에서 백버퍼로 돌아와 UI 합성

{actual('Editor_Window/guiImguiEditor.cpp','void ImguiEditor::End(')}

SceneWindow가 마지막으로 오프스크린 RT에 그렸으므로 `BindFrameBuffer(false, false)`로 **백버퍼를 다시 바인딩**합니다. 여기서 clear를 반복하면 앞선 결과가 사라지고, Scene RT를 그대로 두면 ImGui가 잘못된 텍스처에 그려집니다. ImGui PSO에 맞게 DSV는 연결하지 않습니다.

`ImGui::Render`는 UI draw data를 마무리하고 `ImGui_ImplDX12_RenderDrawData`가 열린 Command List에 기록합니다. End에서 다시 Allocator를 Reset하거나 WaitForNextFrameResources를 호출하지 않습니다. 마지막 RT→PRESENT barrier와 Close 뒤에는 같은 목록에 더 기록하지 않습니다. 현재 Close의 HRESULT도 검사하도록 개선할 여지가 있습니다.

## 5. 실제 프레임 전체 순서

```mermaid
flowchart TD
    W[재사용할 슬롯 Fence 대기] --> A[Application Run: Game 렌더링]
    A --> U[Editor Run: Begin · UI 구성 · Scene 렌더링]
    U --> E[ImguiEditor End: 백버퍼 복구 · UI Draw · Close]
    E --> X[메인 Command List 제출]
    X --> P[추가 플랫폼 창 제출]
    P --> S[메인 Present]
    S --> F[최종 Fence Signal]
    F --> O[EndOfFrame: Scene 이벤트 적용]
```

플랫폼 창은 도킹 탭을 메인 창 밖으로 꺼냈을 때 만들어지는 OS 창입니다. Docking과 Multi-Viewport는 관련은 있지만 서로 다른 옵션입니다. 추가 창도 엔진 Queue를 사용하므로 최종 Fence는 그 제출까지 포함해야 합니다. 메인 목록 Close와 Queue 제출을 UI 함수와 엔진 함수 양쪽에서 중복 실행하지 않습니다.

종료는 GPU 사용 완료 대기 → ImGui DX12/Win32 backend 종료 → 컨텍스트 종료 → 엔진 리소스 정리 순서입니다. ComPtr만으로 GPU 사용 완료가 보장되지는 않습니다.

## 6. 화면이 나오지 않을 때

UI가 Scene 안에 반복해서 그려지면 출력 RTV를, 이미지가 엉뚱하면 GPU handle·현재 Heap·descriptor 수명을 확인합니다. 리사이즈에서만 깨지면 이전 RT와 descriptor의 지연 해제를 확인합니다. `io.DisplaySize`는 플랫폼 backend가 제공하는 클라이언트 크기와 일관돼야 하며, 고 DPI 처리는 끝에서 값 하나만 덮어쓴다고 모두 해결되지 않습니다.

성능 최적화는 실제 병목을 측정한 뒤 합니다. 비싼 값의 계산은 시간 기준으로 캐시할 수 있지만 UI 선언은 매 프레임 필요하며, 60프레임마다 실행하는 것은 항상 1초마다 실행한다는 뜻이 아닙니다. 폰트 압축이나 backend 버퍼 수정은 기본 통합의 필수 과정이 아닙니다.

동기화 원리: <mention-page url="https://app.notion.com/p/3400b1ffa61e8054a80bc1d4b575b515"/>
RT·descriptor와 현재 화면: <mention-page url="{NEW1}"/>
최종 합성·PSO·수명 관리: <mention-page url="{NEW2}"/>
''')
e.finish()
