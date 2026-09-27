e=Edit(27,'DX11에서 배운 렌더링 개념을 DX12의 명시적 리소스·명령·동기화 관리로 확장합니다. API 선택 이유와 이번 엔진의 구현 범위를 먼저 파악합니다.','DX12를 쓰면 자동으로 빨라진다는 오해 없이, 엔진이 직접 관리해야 할 책임을 설명합니다.')
e.para('예전과는 다르게 지금은','PART1에서 정점·텍스처·상수 버퍼·카메라·에디터를 만들었습니다. PART2에서는 같은 화면을 DX12로 옮기면서 GPU 명령의 기록과 제출, 리소스 상태, descriptor와 수명을 직접 관리합니다. 그래픽스 수학과 파이프라인 이해는 여전히 기반이며, AI나 API 버전이 이를 대체하지 않습니다.')
e.para('DirectX 12 는 윈도우와 엑스박스','Direct3D 12는 Windows에서 GPU 렌더링과 연산을 제어하는 API입니다. Vulkan·Metal과도 명시적 자원 관리라는 공통점이 있지만 세부 구조와 지원 플랫폼은 다릅니다.')
e.para('그래서 더 빠르게 실행할수 있습니다.','DX12에서는 여러 스레드에서 각자의 Command List를 기록하고 Queue에 제출하는 구조를 만들 수 있습니다. 드라이버가 하던 일을 엔진이 담당하므로 CPU 오버헤드를 줄일 여지가 있지만, 같은 장면이 자동으로 빨라지지는 않습니다. 동기화·메모리·상태 관리가 잘못되면 성능이나 안정성이 오히려 나빠질 수 있습니다.')
a=e.body.index('directx는 플랫폼 독점성');b=e.body.index('## 강의 목차',a)
e.replace(e.body[a:b],'''## DX11에서 무엇이 바뀌는가?

| 관점 | DX11에서 배운 방식 | DX12에서 직접 다룰 일 |
| --- | --- | --- |
| 명령 기록 | Context에 상태·Draw 설정 | Command List 기록 후 Queue 제출 |
| 파이프라인 | 셰이더·Blend·Depth 등의 개별 상태 | 자주 쓰는 조합을 PSO로 생성·재사용 |
| 리소스 연결 | SRV·RTV 객체와 슬롯 | Descriptor Heap, CPU/GPU Handle, Root Signature |
| 사용 시점 | 드라이버의 많은 자동 관리 | Barrier와 Fence를 구분해 관리 |
| 데이터 수명 | CPU 측 객체 관리 중심 | GPU 사용 완료 뒤 메모리·descriptor 재사용 |

DX11에도 Deferred Context를 통한 명령 기록이 있습니다. DX12의 차이는 멀티스레드가 처음 가능해졌다는 것이 아니라 더 많은 명령·자원 제어를 애플리케이션에 맡긴다는 점입니다. DX11 지원 중단이나 채용 시장에 관한 단정은 이 엔진의 전환 근거로 삼지 않습니다.

## 이번 PART2의 목표

우선 Raster 파이프라인으로 기존 SpriteRenderer와 Opaque·CutOut·Transparent 순서를 복구하고, Scene/Game을 별도 RT에 그려 ImGui에서 표시합니다. Mesh Shader·Compute·DXR 문서는 GPU 기능을 이해하는 개론이며, 현재 YamYam에 모두 구현했다는 뜻은 아닙니다. DXR이나 Mesh Shader는 디바이스의 해당 기능 지원 여부를 따로 확인해야 합니다.

학습 순서는 **초기화 → Raster 파이프라인 → 프레임 동기화 → Texture·RT·상수 버퍼 → 에디터 화면 연결 → 렌더 상태와 수명 관리**입니다. 뒤의 개론 그림은 각 파이프라인을 비교하는 참고 자료입니다.

[Microsoft: Direct3D 12 프로그래밍 가이드](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)

''','검증되지 않은 취업·지원 중단·성능 단정 대신 전환 목표와 책임으로 정리')
e.finish()

e=Edit(28,'Factory → Adapter → Device → Queue → Allocator/List → Fence → SwapChain의 관계를 살펴봅니다. 아래는 Win32 초기화 학습 예시로, 엔진 클래스 전체 구현은 뒤의 삼각형 글에서 이어집니다.','유효한 어댑터를 선택하고, 백버퍼 RTV와 0~1 깊이 범위의 viewport를 생성합니다.')
e.para('Device를 사용하여 Command Queue','Device는 Queue, Allocator, Resource, PSO, Heap, Fence 등을 생성합니다. HLSL 바이트코드 컴파일은 D3DCompile/DXC 같은 별도 컴파일러가 담당합니다.')
e.para('Command 큐는 커맨드(명령)들을 제출','Command Queue에는 Close된 Command List를 제출합니다. CPU 제출과 GPU 실행은 비동기입니다. 여러 Queue가 있다고 하드웨어에서 무조건 동시에 실행되는 것은 아니며, 서로 결과를 주고받을 때 Fence로 순서를 연결해야 합니다.')
e.para('Command allocator 를 사용하여','Command Allocator는 기록된 GPU 명령에 필요한 메모리를 관리합니다. Command List를 Reset할 때 사용할 Allocator를 지정합니다. Allocator가 저장한 명령을 GPU가 사용 중이면 Allocator::Reset을 호출하면 안 됩니다.')
e.para('Dx12 에서는 여러개의 커맨드 큐를','Fence는 GPU 작업 진행 지점을 나타내는 값입니다. Queue가 Signal한 값의 완료를 확인한 뒤 Allocator·업로드 메모리·descriptor를 재사용합니다. 단일 Queue여도 CPU가 GPU보다 앞서 진행하므로 동기화가 필요합니다.')
e.para('Barrier 는 리소스를 어떻게 사용해야','ResourceBarrier는 리소스 접근 순서와 상태를 GPU 명령 안에서 연결합니다. Transition 외에 UAV·Aliasing Barrier도 있습니다. CPU가 GPU 완료를 기다리는 Fence와 역할이 다릅니다. 암시적 promotion/decay가 허용되는 경우도 있으므로 모든 접근에 무조건 Transition을 넣는 것은 아닙니다.')
e.replace('<callout icon="💡" color="gray_bg">\n\tDirect3D 12에서는 리소스가 "동시에 여러 상태로 사용되는 것"은 허용되지 않아.\n\t즉, **GPU 파이프라인에서 동일한 리소스를 다른 상태로 동시에 접근하면 안 돼.**\n\t**→ 반드시 상태 전환(ResourceBarrier)이 필요해.**\n</callout>','''<callout icon="💡" color="gray_bg">
	읽기 전용 상태는 허용된 조합으로 함께 사용할 수 있습니다. 예를 들어 PIXEL_SHADER_RESOURCE와 NON_PIXEL_SHADER_RESOURCE를 조합할 수 있습니다. 쓰기 상태와 읽기 상태를 임의로 섞거나, 같은 subresource에 충돌하는 접근을 동시에 수행하면 안 됩니다. [Microsoft Resource States](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_states)
</callout>''')
e.block(0,'''// 예제 공통: <windows.h>, <wrl/client.h>, <dxgi1_6.h>,
// <d3d12.h>, <stdexcept>와 d3d12.lib, dxgi.lib가 필요합니다.
using Microsoft::WRL::ComPtr;
auto Check = [](HRESULT hr) {
    if (FAILED(hr)) throw std::runtime_error("Direct3D initialization failed");
};
UINT factoryFlags = 0;
#if defined(_DEBUG)
ComPtr<ID3D12Debug> debug;
Check(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
debug->EnableDebugLayer(); // Device 생성 전에 호출
factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
ComPtr<IDXGIFactory4> factory;
Check(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)));''')
e.block(1,'''ComPtr<IDXGIAdapter1> adapter;
for (UINT index = 0; ; ++index)
{
    ComPtr<IDXGIAdapter1> candidate;
    HRESULT hr = factory->EnumAdapters1(index, candidate.GetAddressOf());
    if (hr == DXGI_ERROR_NOT_FOUND) break;
    Check(hr);
    DXGI_ADAPTER_DESC1 desc = {};
    Check(candidate->GetDesc1(&desc));
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
    if (SUCCEEDED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_11_0,
                                   __uuidof(ID3D12Device), nullptr)))
    {
        adapter = candidate;
        break;
    }
}
if (!adapter) throw std::runtime_error("No supported hardware adapter");
// WARP를 사용하려면 별도로 EnumWarpAdapter를 호출하는 정책을 추가합니다.
// D3D12 API 사용과 Feature Level 12_0 요구는 서로 다른 조건입니다.''')
e.block(2,'''ComPtr<ID3D12Device> device;
Check(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                       IID_PPV_ARGS(&device)));''')
e.block(3,'''#if defined(_DEBUG)
ComPtr<ID3D12DebugDevice> debugDevice;
Check(device.As(&debugDevice));
// 종료 시 ReportLiveDeviceObjects로 남은 객체를 조사할 수 있습니다.
#endif''')
e.block(4,'''D3D12_COMMAND_QUEUE_DESC queueDesc = {};
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
ComPtr<ID3D12CommandQueue> commandQueue;
Check(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));''')
e.block(5,'''ComPtr<ID3D12CommandAllocator> commandAllocator;
Check(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     IID_PPV_ARGS(&commandAllocator)));
ComPtr<ID3D12GraphicsCommandList> commandList;
Check(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
    commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
// CreateCommandList는 열린 기록 상태로 생성합니다. 실제 제출 전에 Close합니다.''')
e.block(6,'''ComPtr<ID3D12Fence> fence;
UINT64 nextFenceValue = 1;
Check(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
// CPU 대기 이벤트가 필요하면 CreateEvent 실패 여부를 검사하고,
// 종료 때 GPU 완료를 기다린 후 CloseHandle합니다.
// 생성만으로 GPU 완료를 기다린 것은 아닙니다.''')
e.block(7,'''// 현재 상태가 PRESENT인 백버퍼를 그리기 상태로 전환하는 예시입니다.
// backBuffer는 뒤 절에서 얻은 ID3D12Resource*입니다.
D3D12_RESOURCE_BARRIER barrier = {};
barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
barrier.Transition.pResource = backBuffer;
barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
commandList->ResourceBarrier(1, &barrier);
// 렌더링을 마치면 반대 방향으로 전환한 후 Present합니다.''')
e.block(8,'''// 최초 생성 예시. hwnd는 앞에서 생성한 유효한 Win32 창 핸들입니다.
constexpr UINT backbufferCount = 2;
const UINT width = 640, height = 640;
DXGI_SWAP_CHAIN_DESC1 desc = {};
desc.Width = width;
desc.Height = height;
desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
desc.SampleDesc.Count = 1;
desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
desc.BufferCount = backbufferCount;
desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
ComPtr<IDXGISwapChain1> temporary;
Check(factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd,
    &desc, nullptr, nullptr, temporary.GetAddressOf()));
ComPtr<IDXGISwapChain3> swapchain;
Check(temporary.As(&swapchain));
UINT frameIndex = swapchain->GetCurrentBackBufferIndex();

D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
heapDesc.NumDescriptors = backbufferCount;
heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
ComPtr<ID3D12DescriptorHeap> rtvHeap;
Check(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap)));
const UINT stride = device->GetDescriptorHandleIncrementSize(heapDesc.Type);
ComPtr<ID3D12Resource> renderTargets[backbufferCount];
auto handle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
for (UINT i = 0; i < backbufferCount; ++i)
{
    Check(swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));
    device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, handle);
    handle.ptr += stride;
}
D3D12_VIEWPORT viewport{0.0f, 0.0f, float(width), float(height), 0.0f, 1.0f};
D3D12_RECT scissor{0, 0, LONG(width), LONG(height)};
// viewport.MinDepth/MaxDepth는 투영의 near/far 거리가 아닙니다.
// Resize는 GPU 사용 완료 → 기존 백버퍼 참조 해제 → ResizeBuffers 성공 확인
// → GetBuffer/RTV 재생성 순서로 별도 처리합니다. 크기 0이면 보류합니다.''')
e.finish('### 초기화 다음 단계\n\n위 코드는 개별 개념을 연결한 학습 예시입니다. Error 처리를 추가하면서 ComPtr 소유권, 어댑터 실패, 잘못된 QueryInterface, viewport의 깊이 범위를 정리했습니다. Debug Layer는 Windows Graphics Tools가 필요하며 GPU Based Validation은 별도 선택입니다. [Microsoft: 명령 기록과 Allocator 재사용](https://learn.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles)')
