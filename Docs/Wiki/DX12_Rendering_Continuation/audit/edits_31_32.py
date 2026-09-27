e=Edit(31,'선택 학습: Compute Shader가 UAV 텍스처에 값을 쓰고 그 결과를 백버퍼로 복사하는 흐름입니다. 현재 YamYam의 Scene/Game 표시 경로에 구현한 기능은 아닙니다.','Dispatch의 그룹 수와 실제 스레드 좌표를 구분하고, 화면 경계 검사·상태 전환·Fence를 연결합니다.','flowchart LR\n    D[Dispatch: 그룹 수] --> C[CS: numthreads]\n    C --> U[UAV 텍스처 쓰기]\n    U --> B[UAV → COPY_SOURCE]\n    B --> P[같은 크기·포맷의 백버퍼에 복사]\n    P --> S[COPY_DEST → PRESENT]')
e.para('컴퓨트 파이프라인은 그래픽스 파이프라인보다','Compute Pipeline은 정점·래스터라이저 단계 없이 Compute Shader를 실행합니다. 단계는 적지만 병렬 알고리즘과 메모리 동기화까지 항상 쉬운 것은 아닙니다.')
e.para('리소스와 연산이  분리되어','리소스를 Root Signature로 연결하고, Dispatch로 스레드 그룹을 실행합니다.')
e.para('레스터라이제이션, 레이트레이싱 파이프라인과 마찬가지로','연산에 필요한 SRV·UAV·상수 버퍼를 준비합니다. 삼각형 데이터가 필수는 아니며, UAV는 Compute 전용 리소스도 아닙니다. 이 예제는 UAV 텍스처 한 장을 출력으로 사용합니다.')
e.para('컴퓨트 쉐이더는 간단합니다.','Compute Pipeline의 셰이더 단계는 CS 하나입니다. 이 예제의 한 그룹은 16×16×1 스레드이며 각 스레드가 출력 픽셀 하나를 담당합니다.')
e.para('컴퓨트 파이프라인은 셰이더와 루트 시그니처만','Compute PSO의 핵심은 CS 바이트코드와 Root Signature입니다. 실제 실행에는 descriptor·리소스 상태·명령 목록·동기화도 필요합니다.')
e.block(0,'''// Root Signature 1.0 학습 예시: u0 UAV table 하나.
D3D12_DESCRIPTOR_RANGE range = {};
range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
range.NumDescriptors = 1;
range.BaseShaderRegister = 0;
D3D12_ROOT_PARAMETER param = {};
param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
param.DescriptorTable = {1, &range};
D3D12_ROOT_SIGNATURE_DESC desc = {};
desc.NumParameters = 1;
desc.pParameters = &param;
ComPtr<ID3DBlob> signature, error;
HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1,
    signature.GetAddressOf(), error.GetAddressOf());
if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
ThrowIfFailed(hr);
ComPtr<ID3D12RootSignature> rootSignature;
ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(),
    signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));''')
e.block(1,'''RWTexture2D<float4> outputTexture : register(u0);
[numthreads(16, 16, 1)]
void main(uint3 localID : SV_GroupThreadID, uint3 globalID : SV_DispatchThreadID)
{
    uint width, height;
    outputTexture.GetDimensions(width, height);
    if (globalID.x >= width || globalID.y >= height) return;
    outputTexture[globalID.xy] = float4(
        float(localID.x) / 16.0, float(localID.y) / 16.0,
        float(globalID.x) / float(width), 1.0);
}
// SV_GroupID = グループ番号、SV_GroupThreadID = グループ内の座標。
// SV_GroupIndexはグループ内の線形スレッド番号です。'''.replace('// SV_GroupID = グループ番号、SV_GroupThreadID = グループ内の座標。','// SV_GroupID는 그룹 번호, SV_GroupThreadID는 그룹 안의 좌표입니다.').replace('// SV_GroupIndexはグループ内の線形スレッド番号です。','// SV_GroupIndex는 그룹 안의 스레드를 일렬로 센 번호입니다.'),'hlsl')
e.block(2,'''// compShader는 성공적으로 컴파일한 CS 바이트코드입니다.
D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
psoDesc.pRootSignature = rootSignature.Get();
psoDesc.CS = {compShader->GetBufferPointer(), compShader->GetBufferSize()};
ComPtr<ID3D12PipelineState> pipelineState;
ThrowIfFailed(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
// 실패를 출력만 하고 Dispatch로 진행하지 않습니다.''')
e.block(3,'''// d3dx12.h 사용. width/height > 0, 백버퍼와 같은 크기·포맷·샘플 수를 전제합니다.
D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
heapDesc.NumDescriptors = 1;
heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
ComPtr<ID3D12DescriptorHeap> uavHeap;
ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&uavHeap)));
const auto props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
const auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
    DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1, 1, 0,
    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
ComPtr<ID3D12Resource> outputTexture;
ThrowIfFailed(device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE,
    &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
    IID_PPV_ARGS(&outputTexture)));
D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
uavDesc.Format = texDesc.Format;
uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
const auto uavCPU = uavHeap->GetCPUDescriptorHandleForHeapStart();
const auto uavGPU = uavHeap->GetGPUDescriptorHandleForHeapStart();
device->CreateUnorderedAccessView(outputTexture.Get(), nullptr, &uavDesc, uavCPU);
// この例はdescriptor 1個。動的割当てと範囲外index処理は不要です。'''.replace('// この例はdescriptor 1個。動的割当てと範囲外index処理は不要です。','// 이 예제는 descriptor 1개를 사용합니다. CPU/GPU handle은 같은 슬롯을 가리킵니다.'))
e.block(4,'''// DIRECT Queue/List에서 Compute와 백버퍼 복사를 함께 기록하는 함수 내부입니다.
// 이전 GPU 사용 완료 뒤 allocator Reset. backBuffer는 현재 백버퍼 raw pointer.
ThrowIfFailed(commandAllocator->Reset());
ThrowIfFailed(commandList->Reset(commandAllocator, pipelineState.Get()));
commandList->SetComputeRootSignature(rootSignature.Get());
ID3D12DescriptorHeap* heaps[] = {uavHeap.Get()};
commandList->SetDescriptorHeaps(1, heaps);
commandList->SetComputeRootDescriptorTable(0, uavGPU);
const auto groups = [](UINT value) { return value / 16 + (value % 16 != 0); };
commandList->Dispatch(groups(width), groups(height), 1);

D3D12_RESOURCE_BARRIER beforeCopy[] = {
    CD3DX12_RESOURCE_BARRIER::Transition(outputTexture.Get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
    CD3DX12_RESOURCE_BARRIER::Transition(backBuffer,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST)
};
commandList->ResourceBarrier(2, beforeCopy);
commandList->CopyResource(backBuffer, outputTexture.Get());
D3D12_RESOURCE_BARRIER afterCopy[] = {
    CD3DX12_RESOURCE_BARRIER::Transition(outputTexture.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
    CD3DX12_RESOURCE_BARRIER::Transition(backBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT)
};
commandList->ResourceBarrier(2, afterCopy);
ThrowIfFailed(commandList->Close());
// 続いてExecuteCommandLists、Present、Fenceを実行します。'''.replace('// 続いてExecuteCommandLists、Present、Fenceを実行します。','// 이어서 ExecuteCommandLists, Present, Fence 처리를 수행합니다.'))
e.finish('### 상태와 동기화\n\n16의 배수가 아닌 해상도도 처리하려고 그룹 수를 올림하므로 CS 내부의 경계 검사가 필요합니다. UAV 쓰기 뒤 COPY_SOURCE 전환은 다음 복사와 순서를 연결합니다. UAV 상태를 유지한 채 의존하는 다음 Dispatch를 수행하는 경우에는 필요한 UAV Barrier를 검토합니다. 별도 Compute Queue를 쓰면 Queue 간 Fence 동기화를 추가해야 합니다. [Microsoft: Resource Barrier](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12)')

e=Edit(32,'선택 학습: DXR의 Shader Library·State Object·BLAS/TLAS·Shader Table을 구분합니다. 아래는 API 구조를 설명하는 발췌이며 현재 YamYam에 Ray Tracing을 구현한 기록은 아닙니다.','가속 구조 빌드와 광선 실행을 구분하고, record/table의 바이트 정렬 및 GPU 사용 수명을 설명합니다.')
e.para('Directx 12의 레이트레이싱 하드웨어','DXR은 가속 구조를 탐색해 광선과 도형의 교차를 처리하는 기능을 제공합니다. 반사·그림자·전역조명에 활용할 수 있지만, Path Tracing의 재질 평가·샘플링·누적·노이즈 제거까지 자동으로 구현하지는 않습니다.')
e.para('다이렉트x 12의 레이트레이싱 쉐이더 스테이지는 총 5종류','이 글은 Ray Generation·Intersection·Any Hit·Closest Hit·Miss를 중심으로 설명합니다. DXR에는 Callable Shader도 있으며, 이 예제에서는 사용하지 않습니다.')
e.para('- 🥃 **Any Hit**','- **Any Hit**: 탐색 중 후보 교차를 허용하거나 무시할 때 사용합니다. 거리 순으로 모든 교차점이 한 번씩 보고된다고 가정하면 안 됩니다. 알파 테스트 등에 활용합니다.')
e.para('- 🏏 **Closest Hit**','- **Closest Hit**: 허용된 교차 중 광선의 원점에 가장 가까운 교차를 처리합니다. 내부 탐색에서 가장 먼저 발견한 교차와 같은 뜻은 아닙니다.')
e.para('레이트레이싱 파이프라인은 쉐이더가 읽을','Global Root Signature는 DispatchRays 전에 바인딩할 공통 리소스를 설명합니다. Local Root Signature는 필요한 shader record에 인자를 붙이는 선택적 구성입니다. 모든 셰이더에 local 인자가 필수인 것은 아닙니다.')
e.para('*글로벌 루트 시그니처*는','Global 인자는 이번 Dispatch가 사용할 TLAS·카메라 상수·출력 UAV 같은 공통 바인딩입니다. Dispatch 간 데이터 공유는 리소스와 명령의 수명·갱신 정책으로 결정됩니다.')
e.para('쉐이더 레코드 테이블은 64비트','Shader Table의 시작 GPU 주소는 **64바이트**, record stride는 **32바이트** 배수로 맞춥니다. 64비트 정렬이 아닙니다. Record는 32바이트 shader identifier 뒤에 local root 인자를 순서대로 배치합니다. Root constants는 지정한 값 자체, root descriptor는 GPU 주소, descriptor table은 GPU descriptor handle을 담습니다. [DXR 명세: Shader Tables](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#shader-tables)')
e.para('**스크래치 버퍼**는','Scratch는 BLAS/TLAS 빌드에 사용하는 임시 작업 메모리입니다. 결과 AS 버퍼와 다르며 UNORDERED_ACCESS 상태로 사용합니다. 크기는 PrebuildInfo로 구하고, 재사용 전에 이전 빌드의 사용 완료·순서를 보장합니다.')
e.para('- 🇺🇸 로컬 리소스를 위한','- 선택적 Local Root Signature와 필요한 export/hit group association')
e.block(0,'''// DXC lib_6_3 이상으로 컴파일하는 HLSL 예시.
RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> tOutput : register(u0);
cbuffer CameraCB : register(b0)
{
    row_major float4x4 inverseViewProjection;
    float3 cameraOrigin;
    float maxRayDistance;
};
struct RayPayload { float4 color; };
struct LocalCB { float time; };
ConstantBuffer<LocalCB> localCB : register(b1);
Texture2D<float4> localTex : register(t1);
SamplerState localSampler : register(s0);

[shader("raygeneration")]
void raygen()
{
    float2 uv = (float2(DispatchRaysIndex().xy) + 0.5) / float2(DispatchRaysDimensions().xy);
    float2 ndc = float2(uv.x * 2 - 1, 1 - uv.y * 2);
    // 일반 Direct3D 깊이·유한 far plane의 원근 카메라 예시.
    float4 world = mul(float4(ndc, 1, 1), inverseViewProjection);
    world.xyz /= world.w;
    RayDesc ray;
    ray.Origin = cameraOrigin;
    ray.Direction = normalize(world.xyz - cameraOrigin);
    ray.TMin = 0.001;
    ray.TMax = maxRayDistance;
    RayPayload payload = {float4(0, 0, 0, 0)};
    TraceRay(Scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);
    tOutput[DispatchRaysIndex().xy] = payload.color;
}
[shader("closesthit")]
void closesthit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    // 重心座標を仮のUVとして使うデモ。実際の頂点UV補間ではありません。
    payload.color = localTex.SampleLevel(localSampler,
        attr.barycentrics + float2(sin(localCB.time) * 0.1, 0), 0);
}
[shader("miss")]
void miss(inout RayPayload payload) { payload.color = float4(0.5, 0.5, 0.5, 1); }
'''.replace('// 重心座標を仮のUVとして使うデモ。実際の頂点UV補間ではありません。','// 무게중심 좌표를 임시 UV로 쓰는 데모입니다. 실제 정점 UV 보간은 별도입니다.'),'hlsl')
for n in (1,2):
    code=e.blocks[n][3].rstrip()
    code=code.replace('ID3DBlob* signature;\nID3DBlob* error;','ComPtr<ID3DBlob> signature;\nComPtr<ID3DBlob> error;')
    begin=code.index('try\n')
    which='globalRootSignature' if n==1 else 'localRootSignature'
    code=code[:begin]+f'''HRESULT hr = D3D12SerializeVersionedRootSignature(
    &rootSignatureDesc, signature.GetAddressOf(), error.GetAddressOf());
if (error) OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
ThrowIfFailed(hr);
ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(),
    signature->GetBufferSize(), IID_PPV_ARGS(&{which})));
// Root Signature 1.1を前提。対応しない環境は1.0用desc/serializeへ切り替えます。'''.replace('// Root Signature 1.1を前提。対応しない環境は1.0用desc/serializeへ切り替えます。','// Root Signature 1.1 지원을 전제합니다. 미지원이면 1.0 구조/직렬화를 사용합니다.')
    if n==2:
        code=code.replace('((sizeof(mRayGenCB) - 1) / sizeof(UINT32) + 1)','4')
        code=code.replace('sampler.MaxAnisotropy = 0;','sampler.MaxAnisotropy = 1;')
    e.block(n,code)
e.block(3,'''// Local Root: b1 root constants 4개(16 bytes) → t1 table handle(8 bytes).
// shared heap의 [0]=u0, [1]=b0, [2]=t1이며 local table offset=2입니다.
struct alignas(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) HitRecord
{
    unsigned char identifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES]; // 32
    float localConstants[4]; // time + padding
    D3D12_GPU_DESCRIPTOR_HANDLE heapBase; // table의 시작; offset 2 적용
};
static_assert(sizeof(HitRecord) == 64);
// GetShaderIdentifier(L"MyHitGroup")의 32바이트를 identifier에 복사합니다.
// RayGen/Miss에는 local root를 연결하지 않아 identifier만 담은 record를 사용합니다.
// 각 table은 시작 GPU 주소를 64바이트 정렬해 할당하고 올바른 상태로 유지합니다.''')
e.block_replace(4,'ID3D12PipelineState* pipelineState;','ID3D12StateObject* pipelineState = nullptr;')
e.block_replace(4,'ID3D12StateObjectProperties* stateObjectProperties;','ID3D12StateObjectProperties* stateObjectProperties = nullptr;')
e.block_replace(4,'static LPCWSTR shaderNames[3] = {\n    L"raygen", L"closesthit", L"miss"};','static LPCWSTR shaderNames[] = { L"MyHitGroup" };')
e.block_replace(4,'subObjects + 4, 3u, shaderNames','subObjects + 4, 1u, shaderNames')
code=e.code_changes[4][0]
code=code[:code.index('HRESULT result = device->CreateStateObject')]+'''ThrowIfFailed(device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&pipelineState)));
ThrowIfFailed(pipelineState->QueryInterface(IID_PPV_ARGS(&stateObjectProperties))));'''
code=code.replace('IID_PPV_ARGS(&stateObjectProperties))));','IID_PPV_ARGS(&stateObjectProperties)));')
e.block(4,code)
e.block(5,'''// API 구조 발췌: device=ID3D12Device5*, commandList=ID3D12GraphicsCommandList4*.
// VB/IBは完成済み、indexはR16_UINT、positionは頂点先頭のfloat3を前提。
D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
geometry.Triangles.VertexBuffer = {vertexBuffer->GetGPUVirtualAddress(), sizeof(Vertex)};
geometry.Triangles.VertexCount = vertexCount;
geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
geometry.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
geometry.Triangles.IndexCount = indexCount;
geometry.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs = {};
blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
blasInputs.NumDescs = 1;
blasInputs.pGeometryDescs = &geometry;
blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs = {};
tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
tlasInputs.NumDescs = 1;
tlasInputs.Flags = blasInputs.Flags;
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasInfo = {}, tlasInfo = {};
device->GetRaytracingAccelerationStructurePrebuildInfo(&blasInputs, &blasInfo);
device->GetRaytracingAccelerationStructurePrebuildInfo(&tlasInputs, &tlasInfo);
// ここでresultとscratchを確保。下の文章にサイズ・状態を記載します。

D3D12_RAYTRACING_INSTANCE_DESC instance = {};
instance.Transform[0][0] = instance.Transform[1][1] = instance.Transform[2][2] = 1.0f;
instance.InstanceMask = 0xff;
instance.AccelerationStructure = blasBuffer->GetGPUVirtualAddress();
// instanceをUPLOAD bufferへコピーし、ビルド完了まで保持します。
tlasInputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();
D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blasBuild = {}, tlasBuild = {};
blasBuild.Inputs = blasInputs;
blasBuild.DestAccelerationStructureData = blasBuffer->GetGPUVirtualAddress();
blasBuild.ScratchAccelerationStructureData = blasScratch->GetGPUVirtualAddress();
tlasBuild.Inputs = tlasInputs;
tlasBuild.DestAccelerationStructureData = tlasBuffer->GetGPUVirtualAddress();
tlasBuild.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
commandList->BuildRaytracingAccelerationStructure(&blasBuild, 0, nullptr);
auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(blasBuffer);
commandList->ResourceBarrier(1, &barrier);
commandList->BuildRaytracingAccelerationStructure(&tlasBuild, 0, nullptr);
barrier = CD3DX12_RESOURCE_BARRIER::UAV(tlasBuffer);
commandList->ResourceBarrier(1, &barrier);
// BLAS/TLAS scratchを別々に用意した例。共有する場合は再利用のBarrierも必要。
'''.replace('// VB/IBは完成済み、indexはR16_UINT、positionは頂点先頭のfloat3を前提。','// 업로드 완료된 VB/IB, R16_UINT 인덱스, 정점 맨 앞 float3 위치를 전제합니다.').replace('// ここでresultとscratchを確保。下の文章にサイズ・状態を記載します。','// 이 지점에서 result/scratch 버퍼를 확보합니다. 아래 설명의 크기·상태를 맞춥니다.').replace('// instanceをUPLOAD bufferへコピーし、ビルド完了まで保持します。','// instance를 UPLOAD 버퍼에 복사하고 빌드 완료까지 유지합니다.').replace('// BLAS/TLAS scratchを別々に用意した例。共有する場合は再利用のBarrierも必要。','// BLAS/TLAS scratch를 따로 준비한 예입니다. 공유하면 재사용 Barrier도 필요합니다.'))
e.block_replace(6,'commandList->SetComputeRootSignature(globalRootSignature);','// 한 개 RayGen, 한 개 Miss, 한 개 HitGroup record를 담은 예제입니다.\n// 각 buffer는 사용한 record 크기만 따로 관리하며 전체 할당 크기로 stride를 정하지 않습니다.\ncommandList->SetComputeRootSignature(globalRootSignature);')
e.block_replace(6,'tlas->GetGPUVirtualAddress()','tlasBuffer->GetGPUVirtualAddress()')
e.block_replace(6,'rayGenShaderTable->GetDesc().Width','D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES')
e.block_replace(6,'missShaderTable->GetDesc().Width','D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES')
e.block_replace(6,'hitShaderTable->GetDesc().Width','sizeof(HitRecord)')
e.finish('### 이 예제를 완성할 때 필요한 연결\n\nDXR Tier는 D3D12_FEATURE_D3D12_OPTIONS5로 확인합니다. BLAS/TLAS 결과는 PrebuildInfo.ResultDataMaxSizeInBytes 이상의 DEFAULT 버퍼, ALLOW_UNORDERED_ACCESS flag, RAYTRACING_ACCELERATION_STRUCTURE 상태로 만듭니다. Scratch는 ScratchDataSizeInBytes 이상의 DEFAULT UAV 버퍼이며 UNORDERED_ACCESS 상태입니다. 주소 정렬과 0 크기 실패를 확인하고 입력 VB/IB·instance 데이터는 빌드에서 읽을 수 있는 상태로 유지합니다. Shader Table 메모리 할당·identifier 복사, CPU 카메라 상수와 local 인자 채우기, UAV 결과 표시, 제출·Fence는 위 발췌 밖의 필수 단계입니다. 엔진에 완성된 기능으로 표시하지 않습니다. [Microsoft DXR 명세](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html)')
