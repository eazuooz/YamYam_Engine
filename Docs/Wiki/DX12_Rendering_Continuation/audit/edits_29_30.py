e=Edit(29,'Descriptor·Root Signature·버퍼·셰이더·PSO를 연결해 한 개의 삼각형을 그리는 학습 경로입니다. 예제는 단순한 프레임 종료 대기를 사용하고, 뒤의 Frame 동기화 글에서 여러 프레임으로 확장합니다.','상수 데이터와 CBV, PSO와 동적 명령, Fence와 Barrier의 역할을 구분합니다.')
e.para('레스터라이제이션 파이프라인은 GPU를','Rasterization은 삼각형 등의 도형을 화면 샘플로 변환하는 과정입니다. Mesh Shader도 출력한 도형을 래스터라이저로 보낼 수 있으므로 Rasterization의 반대 개념이 아닙니다. Ray Tracing과 함께 사용하는 혼합 렌더링도 가능합니다. 이 글은 IA → VS → Rasterizer → PS → OM 경로를 설명합니다.')
e.para('(Descriptor Heap은 Descriptor들이','Descriptor Heap은 리소스 내용을 저장하는 Resource Heap과 다릅니다. Descriptor라는 접근 정보를 모아 둡니다. 모든 Descriptor Heap이 shader-visible인 것은 아닙니다.')
e.para('GPU가 읽을 수 있도록 메모리에 연속적으로','CBV/SRV/UAV와 Sampler Heap은 shader-visible로 만들 수 있고, RTV/DSV Heap은 CPU handle로 명령에 지정합니다.')
e.para('힙은 여러 오브젝트들을 감싸고 있는 GPU 메모리','Resource Heap은 버퍼·텍스처가 사용할 메모리입니다. DEFAULT는 GPU 작업 중심, UPLOAD는 CPU에서 쓰기, READBACK은 GPU 복사 결과를 CPU가 읽는 용도로 사용합니다. 아래 Committed Resource는 별도 명시적 Heap 생성 없이 리소스와 메모리를 함께 확보하는 방식입니다.')
e.para('외부라이브러리를 사용하여 컴파일','HLSL 컴파일은 D3D12 Device의 기능이 아닙니다. 아래 예제는 D3DCompiler의 D3DCompileFromFile을 사용합니다. Mesh Shader 등 Shader Model 6 기능은 뒤 문서에서 DXC를 사용합니다.')
e.para('\t1. PSO는 상태가 조금이라도','\t1. 셰이더·Blend·Depth 등 **PSO에 포함된 상태**가 달라지면 다른 PSO가 필요합니다. viewport, scissor, descriptor 바인딩처럼 Command List에서 설정하는 상태는 별도입니다.')
e.para('\t3. PSO 생성은 비용이 크므로','\t3. 자주 쓰는 PSO는 미리 만들고 캐시해 재사용합니다. 모든 조합을 초기화 때 만들 필요는 없으며, 필요한 조합만 생성하거나 비동기 준비하는 정책도 가능합니다.')
e.para('ComPtr\\<T\\> 를 사용한 경우','ComPtr는 CPU 측 COM 소유권을 관리합니다. 마지막 참조를 놓기 전에 GPU 사용이 끝났는지는 Fence로 별도 확인해야 합니다. raw pointer 예제는 GPU 완료 후 각 소유 참조를 Release하고 이벤트 핸들을 CloseHandle해야 합니다.')
e.block(1,'''// 학습 예제는 Root Signature 1.0으로 구성합니다.
// ComPtr와 ThrowIfFailed는 예제 공통 헬퍼입니다.
D3D12_DESCRIPTOR_RANGE range = {};
range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
range.NumDescriptors = 1;
range.BaseShaderRegister = 0;
D3D12_ROOT_PARAMETER parameter = {};
parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
parameter.DescriptorTable = {1, &range};
D3D12_ROOT_SIGNATURE_DESC desc = {};
desc.NumParameters = 1;
desc.pParameters = &parameter;
desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
ComPtr<ID3DBlob> signature, error;
HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
    signature.GetAddressOf(), error.GetAddressOf());
if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
ThrowIfFailed(hr);
ID3D12RootSignature* rootSignature = nullptr;
ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(),
    signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
// 1.1 구조를 사용하면 지원 버전 확인과 1.0 변환/직렬화 경로도 함께 필요합니다.''')
e.block(2,'''// d3dx12.h 헬퍼를 사용하는 버퍼 업로드 예시입니다.
// sourceData는 비어 있지 않은 std::vector<uint8_t>입니다.
if (sourceData.empty()) throw std::runtime_error("Empty upload");
const auto uploadProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
const auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sourceData.size());
ComPtr<ID3D12Resource> uploadBuffer;
ThrowIfFailed(device->CreateCommittedResource(&uploadProps, D3D12_HEAP_FLAG_NONE,
    &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
    IID_PPV_ARGS(&uploadBuffer)));
void* mapped = nullptr;
const D3D12_RANGE noCpuRead{0, 0};
ThrowIfFailed(uploadBuffer->Map(0, &noCpuRead, &mapped));
std::memcpy(mapped, sourceData.data(), sourceData.size());
const D3D12_RANGE written{0, sourceData.size()};
uploadBuffer->Unmap(0, &written);
// 목적 DEFAULT 버퍼에 CopyBufferRegion을 기록한 경우 GPU 복사 완료까지 유지합니다.
// 텍스처는 GetCopyableFootprints와 RowPitch를 사용하며 이 memcpy만으로 충분하지 않습니다.

const auto readbackProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
ComPtr<ID3D12Resource> readbackBuffer;
ThrowIfFailed(device->CreateCommittedResource(&readbackProps, D3D12_HEAP_FLAG_NONE,
    &uploadDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
    IID_PPV_ARGS(&readbackBuffer)));
// GPU 복사 명령 제출 → 해당 Fence 완료 → Map으로 읽기 순서입니다.
// Map이 GPU 복사를 자동으로 기다려 주는 것은 아닙니다.''')
e.block_replace(5,'glm::mat4 projectionMatrix;\n    glm::mat4 modelMatrix;\n    glm::mat4 viewMatrix;','DirectX::XMFLOAT4X4 projectionMatrix;\n    DirectX::XMFLOAT4X4 modelMatrix;\n    DirectX::XMFLOAT4X4 viewMatrix;')
e.block_replace(5,'} cbVS;','} cbVS = {};\n// 아래 HLSL row_major + 행벡터 mul과 맞춥니다. GLM 열벡터 예제와 혼합하지 않습니다.\nDirectX::XMStoreFloat4x4(&cbVS.projectionMatrix, DirectX::XMMatrixIdentity());\nDirectX::XMStoreFloat4x4(&cbVS.modelMatrix, DirectX::XMMatrixIdentity());\nDirectX::XMStoreFloat4x4(&cbVS.viewMatrix, DirectX::XMMatrixIdentity());')
e.block_replace(5,'constantBuffer->Unmap(0, &readRange);','const D3D12_RANGE writtenRange{0, sizeof(cbVS)};\nconstantBuffer->Unmap(0, &writtenRange);')
e.block_replace(9,'psoDesc.DepthStencilState.DepthEnable = FALSE;','psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);\npsoDesc.DepthStencilState.DepthEnable = FALSE;')
code=e.blocks[9][3].rstrip(); code=code[:code.index('try\n')]+'''ThrowIfFailed(device->CreateGraphicsPipelineState(
    &psoDesc, IID_PPV_ARGS(&pipelineState)));
// 실패를 출력만 하고 Draw로 진행하지 않습니다.'''
code=code.replace('psoDesc.DepthStencilState.DepthEnable = FALSE;','psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);\npsoDesc.DepthStencilState.DepthEnable = FALSE;')
e.block(9,code)
e.block(10,'''// 앞에서 생성한 유효한 commandAllocator와 pipelineState를 사용합니다.
ID3D12GraphicsCommandList* commandList = nullptr;
ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
    commandAllocator, pipelineState, IID_PPV_ARGS(&commandList)));
// 이 예제는 다음 setupCommands에서 Reset하므로 최초 열린 목록을 먼저 닫습니다.
ThrowIfFailed(commandList->Close());''')
e.block_replace(11,'// 🚿 Reset the command list and add new commands.','// setupCommands() 함수 내부 발췌입니다.\n// 이전 GPU 실행 완료를 Fence로 확인한 후 Allocator를 재사용합니다.')
e.block_replace(11,'rtvHeap->GetCPUDescriptorHandleForHeapStart()','renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart()')
e.block(12,'''// 단순한 한 프레임 대기 예제: GPU/CPU 병렬성을 높이는 최종 구현은 아닙니다.
// fence(ID3D12Fence*), fenceEvent, fenceValue=1은 초기화 단계에서 준비합니다.
void render()
{
    static auto previous = std::chrono::steady_clock::now();
    static float angle = 0.0f;
    const auto now = std::chrono::steady_clock::now();
    angle += std::chrono::duration<float>(now - previous).count();
    previous = now;
    DirectX::XMStoreFloat4x4(&cbVS.modelMatrix, DirectX::XMMatrixRotationY(angle));
    const D3D12_RANGE noRead{0, 0};
    ThrowIfFailed(constantBuffer->Map(0, &noRead,
        reinterpret_cast<void**>(&mappedConstantBuffer)));
    std::memcpy(mappedConstantBuffer, &cbVS, sizeof(cbVS));
    const D3D12_RANGE written{0, sizeof(cbVS)};
    constantBuffer->Unmap(0, &written);

    setupCommands(); // 앞 절의 기록 코드를 이 함수에 둡니다.
    ID3D12CommandList* lists[] = {commandList};
    commandQueue->ExecuteCommandLists(1, lists);
    ThrowIfFailed(swapchain->Present(1, 0));
    const UINT64 submittedValue = fenceValue++;
    ThrowIfFailed(commandQueue->Signal(fence, submittedValue));
    if (fence->GetCompletedValue() < submittedValue)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(submittedValue, fenceEvent));
        if (WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0)
            throw std::runtime_error("Fence wait failed");
    }
    frameIndex = swapchain->GetCurrentBackBufferIndex();
}
// 매 호출 끝의 대기가 다음 호출의 CB 갱신/Allocator Reset을 보호합니다.
// 대기를 제거하려면 프레임 슬롯별 메모리와 Fence 관리가 먼저 필요합니다.''')
e.finish('### 예제와 엔진 구현의 차이\n\n위 블록은 개념별 발췌이며 동일한 변수 이름을 사용하는 생성 예제를 한 scope에 그대로 중복 선언하지 않습니다. CBV의 256바이트 주소·크기 정렬과 HLSL 16바이트 패킹은 다른 규칙입니다. 이 예제는 CBV descriptor table을 쓰지만 현재 YamYam Transform은 root CBV와 Draw별 업로드 구간을 사용합니다. <mention-page url="'+NEW1+'"/>')

e=Edit(30,'선택 학습: IA·VS 경로 대신 스레드 그룹에서 정점과 도형을 출력하는 Mesh Shader를 살펴봅니다. 현재 YamYam 렌더링에 구현한 기능은 아닙니다.','Mesh Shader도 Rasterization으로 이어지며, 지원 Tier·DXC 프로파일·출력 개수와 쓰기 범위를 확인해야 함을 이해합니다.')
e.para('- 예전의 Hull Shader 역할','- AS는 가시성·LOD 판단과 Mesh Shader 작업 분배에 활용할 수 있습니다. Hull Shader의 테셀레이션 계약을 그대로 대체하는 동일한 단계는 아닙니다.')
e.para('2. 그룹 공유 배열에 출력 기록','2. 출력 매개변수 배열에 정점과 도형 정보를 기록합니다. HLSL의 `groupshared` 저장소와 동일한 선언이 아닙니다.')
e.para('4. 래스터라이저가 출력 순서대로','4. 출력 도형이 클리핑·래스터라이제이션으로 이어지고 필요하면 Pixel Shader가 실행됩니다.')
e.para('> 주의: SetMeshOutputCounts','> SetMeshOutputCounts는 그룹 전체에 일관된 제어 흐름으로 호출하고, 선언된 출력 배열 크기와 지원 한도 안의 개수를 지정합니다. 여러 스레드가 같은 출력 요소에 동시에 쓰지 않도록 작업을 나눕니다.')
e.block(0,'''// AS를 사용할 때의 별도 개념 예시입니다.
// MS에서는 같은 MeshPayload 정의와 `in payload MeshPayload payload` 입력이 필요합니다.
struct MeshPayload { uint baseMeshlet; };
groupshared MeshPayload payload;
[numthreads(1, 1, 1)]
void ASMain(uint3 groupID : SV_GroupID)
{
    payload.baseMeshlet = groupID.x * 4;
    DispatchMesh(4, 1, 1, payload);
}''','hlsl')
e.block(1,'''// AS를 생략한 최소 MS 예시입니다. CPU에서 DispatchMesh(1, 1, 1).
struct VertexAttributes { float4 position : SV_Position; };
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MSMain(out vertices VertexAttributes verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
    verts[0].position = float4(-0.5, -0.5, 0, 1);
    verts[1].position = float4(0.0, 0.5, 0, 1);
    verts[2].position = float4(0.5, -0.5, 0, 1);
    tris[0] = uint3(0, 1, 2);
}
// 하나의 스레드만 출력하므로 중복 쓰기가 없습니다.
// 실제 메시 처리에서는 여러 스레드가 서로 다른 요소를 담당하게 확장합니다.''','hlsl')
e.block(2,'''// d3dx12.h와 Mesh Shader를 지원하는 SDK가 필요합니다.
// 완전한 PSO 구성 시 출력 포맷·Rasterizer·Blend·Depth 상태도 맞춥니다.
struct PSO_STREAM
{
    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSig;
    CD3DX12_PIPELINE_STATE_STREAM_MS MS;
    CD3DX12_PIPELINE_STATE_STREAM_PS PS;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS formats;
    CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER raster;
    CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC blend;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth;
};
// stream의 각 필드는 컴파일한 바이트코드, root signature, RT 구성으로 채웁니다.
// 순서는 크기 → 포인터입니다. ID3D12Device2 인터페이스로 호출합니다.
D3D12_PIPELINE_STATE_STREAM_DESC desc{sizeof(stream), &stream};
ThrowIfFailed(device2->CreatePipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
// AS 사용 시 AS subobject를 추가하며, 전통 VS/GS/HS/DS와 혼합하지 않습니다.''')
e.block(3,e.blocks[3][3],lang='hlsl',caption='출력 속성 확장 예시 — 앞의 최소 VertexAttributes를 대체할 때 사용')
e.block(4,'''// 선언만 해서는 통계가 수집되지 않습니다.
D3D12_QUERY_DATA_PIPELINE_STATISTICS1 stats = {};
// 지원 확인 → QueryHeap 생성 → BeginQuery/EndQuery → ResolveQueryData
// → Fence 완료 → readback 결과를 stats로 읽은 뒤 MSInvocations/MSPrimitives 사용.
// 이 절은 API 순서를 설명하며 실제 수집 구현은 포함하지 않습니다.''')
e.block(5,'''D3D12_FEATURE_DATA_D3D12_OPTIONS7 options = {};
const HRESULT hr = device->CheckFeatureSupport(
    D3D12_FEATURE_D3D12_OPTIONS7, &options, sizeof(options));
const bool supported = SUCCEEDED(hr) &&
    options.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
// 지원하지 않으면 기존 IA/VS 렌더링 경로를 사용합니다.''')
e.finish('### 컴파일과 실행 조건\n\nAS/MS는 DXC의 as_6_5 / ms_6_5 이상 프로파일과 해당 기능 지원이 필요합니다. CPU의 DispatchMesh는 ID3D12GraphicsCommandList6 인터페이스에서 호출합니다. PSO 코드의 stream 채우기와 QueryHeap 통계 수집은 별도 구현 단계이며, 예제 일부만 붙여 넣어 전체 파이프라인이 실행되는 것은 아닙니다. [Microsoft Mesh Shader 명세](https://microsoft.github.io/DirectX-Specs/d3d/MeshShader.html)')
