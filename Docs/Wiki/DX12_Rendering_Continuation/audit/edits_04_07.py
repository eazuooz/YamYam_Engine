e=Edit(4,'큰 파일을 읽는 작업과 장면을 갱신하는 작업을 분리하는 이유를 배웁니다. 스레드 완료 통지와 공유 데이터 보호를 함께 읽습니다.','로딩 결과를 읽는 시점이 정해지고, 스레드가 사용하는 객체가 먼저 파괴되지 않습니다.','flowchart LR\n    W[작업 스레드: 파일 읽기·디코딩] --> Q[결과 전달과 완료 동기화]\n    Q --> M[메인 스레드: 리소스 등록·씬 전환]\n    M --> R[렌더링]')
e.para('모두\u00a0`"<mutex>"`헤더에','`std::mutex`, `recursive_mutex`, `timed_mutex` 계열은 `<mutex>`에, `shared_mutex`·`shared_timed_mutex`는 `<shared_mutex>`에 정의되어 있습니다. 실제 공유 데이터를 읽고 쓰는 모든 경로가 같은 잠금 규칙을 따라야 합니다.')
e.para('`std::mutex`는 같은 mutex 객체로','같은 스레드가 이미 소유한 `std::mutex`를 다시 `lock()`하는 것은 허용되지 않으며 정의되지 않은 동작입니다. 항상 특정 예외가 발생한다고 기대하면 안 됩니다.')
e.para('그래서\u00a0`std::system_error`','`std::recursive_mutex`는 같은 스레드가 반복해서 잠글 수 있으며, 잠근 횟수만큼 해제해야 합니다. 보통은 `std::lock_guard` 또는 `std::unique_lock`으로 범위가 끝날 때 잠금이 해제되게 구성합니다.')
e.para('C++17에서 추가된\u00a0`std::shared_mutex`','`std::shared_mutex`는 여러 읽기 작업의 공유 잠금과 하나의 쓰기 작업의 배타적 잠금을 제공합니다. 내부 구현과 성능은 표준 라이브러리·운영체제·경합 패턴에 따라 달라지며, 항상 특정 OS lock을 사용하거나 항상 더 빠른 것은 아닙니다.')
e.para('로딩씬에서는 원래 뮤텍스를 이용해서','잠금이 필요한지는 로딩 씬이라는 이름이 아니라 공유 데이터의 접근 방식으로 결정됩니다. 일반 bool을 한 스레드가 쓰고 다른 스레드가 동기화 없이 읽으면 데이터 경합입니다. 완료 플래그에는 atomic이나 mutex를 사용하고, 로딩 결과의 공개 시점도 함께 정해야 합니다.')
e.para('여러분들의 학습목적으로 넣어둔 코드이다.','아래 기존 캡처는 당시 DX11 로딩 실습의 코드입니다. 현재 DX12 경로는 공유 씬·리소스 컨테이너의 경쟁을 피하도록 메인 렌더 스레드에서 로딩을 처리하는 단계이며, 이 캡처를 현재 구현으로 읽지 않습니다.')
e.para('임계영역이 설정되면','mutex를 얻으려는 다른 스레드만 잠금 해제까지 대기합니다. 모든 다른 스레드가 멈추는 것은 아니며, mutex를 사용하지 않고 공유 데이터에 접근하는 코드는 보호되지 않습니다.')
e.para('로딩이 완료된후 main쓰레드에서','`join()`은 호출한 스레드가 대상 스레드의 종료를 기다리는 함수입니다. 자식 스레드를 부모 스레드 안으로 합치는 동작이 아닙니다.')
e.para('또는 아예 독립적인 쓰레드로','joinable한 `std::thread` 객체가 소멸하면 `std::terminate()`가 호출됩니다. `join()` 또는 `detach()`로 처리해야 하지만, detach는 공유 객체의 수명 문제를 해결하지 않습니다. 이 로딩 흐름에서는 종료 시점을 관리하고 join하는 구성을 우선합니다.')
e.finish('### 완료 통지의 최소 예시\n\n```c++\nstd::atomic<bool> ready{false};\n// 작업 스레드: result를 완성한 후\nready.store(true, std::memory_order_release);\n// 메인 스레드: 완료가 보이면 result를 소비\nif (ready.load(std::memory_order_acquire)) { /* result 사용 */ }\n```\n\n설명용 발췌입니다. `<atomic>`이 필요하고, 결과는 완료 통지 후 작업 스레드가 다시 수정하지 않는 구조를 전제로 합니다. atomic bool 하나가 씬·리소스 컨테이너 전체를 자동으로 보호하지는 않습니다.')

e=Edit(5,'HLSL 컴파일과 DX11 셰이더 객체 생성은 로딩 단계에서, Bind는 Draw 직전에 수행하도록 클래스로 나눕니다.','컴파일·생성 실패가 호출자에게 전달되고, 유효한 Shader만 바인딩됩니다.')
e.block_replace(1,'return S_FALSE;','return E_FAIL;')
e.block_replace(1,'\t\tif (stage == eShaderStage::VS)\n\t\t\tCreateVertexShader(fileName);\n\t\tif (stage == eShaderStage::PS)\n\t\t\tCreatePixelShader(fileName);\n\n\t\treturn true;','\t\tif (stage == eShaderStage::VS) return CreateVertexShader(fileName);\n\t\tif (stage == eShaderStage::PS) return CreatePixelShader(fileName);\n\t\treturn false;')
e.block_replace(2,'0, 0, application.GetWidth(), application.GetHeight(),','0.0f, 0.0f, static_cast<float>(application.GetWidth()), static_cast<float>(application.GetHeight()),')
e.block_replace(2,'mContext->Draw(3, 0);','mContext->DrawIndexed(3, 0, 0);')
e.para('픽셀 셰이더는 해당 픽셀 좌표에','픽셀 셰이더는 래스터라이저가 전달한 입력으로 출력 색 등을 계산합니다. 렌더 타깃의 기존 색을 자동으로 입력받는 단계는 아니며, 기존 색과 섞는 일은 일반적으로 Output Merger의 블렌딩이 담당합니다.')
e.finish('### 실패 코드와 단계 구분\n\n`S_FALSE`는 HRESULT의 실패 값이 아니므로 `FAILED(S_FALSE)`는 false입니다. 생성 실패에는 `E_FAIL` 같은 실패 HRESULT를 반환하도록 고쳤습니다. 이 글의 실행 예제는 VS·PS 중심이며, 멤버로 선언한 HS·DS·GS까지 모두 로딩하는 구현은 아닙니다. 마지막 Draw 예제는 바인딩한 인덱스 버퍼를 사용하도록 `DrawIndexed`로 맞췄습니다. DX12에서는 셰이더 바이트코드와 렌더 상태를 PSO로 묶는 차이가 있습니다: <mention-page url="'+NEW2+'"/>')

e=Edit(6,'정점 버퍼의 생성·바인딩을 감싸고, 실제 정점 구조체의 크기와 Input Layout을 맞춥니다.','빈 데이터·생성 실패·업데이트 용량 초과를 처리하고, 올바른 stride로 정점을 읽습니다.')
e.para('4. 성공 시 true, 실패 시 false 반환','4. 원래 ID3D11Device::CreateBuffer는 HRESULT를 반환합니다. 이 글의 GetDevice()->CreateBuffer 래퍼는 bool을 반환하므로 두 반환 형식을 섞어 검사하지 않습니다.')
e.para('4. 최고 성능','4. 초기화 후 읽기 전용이라는 용도에 맞게 선택하고, 실제 성능은 측정합니다.')
e.block_replace(4,'D3D11_BUFFER_DESC desc;','D3D11_BUFFER_DESC desc = {};')
e.block_replace(5,'\tdesc.ByteWidth = sizeof(Vertex) * vertexes.size();','\tif (vertexes.empty() || vertexes.size() > UINT_MAX / sizeof(Vertex)) return false;\n\tdesc = {};\n\tdesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexes.size());')
for n in (5,14):e.block_replace(n,'assert(NULL, "Create vertex buffer failed!");','return false; // Release 빌드에서도 실패를 전달')
e.block(10,'''DEFAULT: GPU 사용 중심, UpdateSubresource 또는 복사로 갱신
DYNAMIC: CPU 쓰기·GPU 읽기, Map/Unmap 갱신
IMMUTABLE: 생성 때 초기화하고 이후 내용 변경 불가
STAGING: CPU 접근·복사용, 렌더링에 직접 바인딩하지 않음''','plain text')
e.block(32,'''bool UpdateVertexBuffer(VertexBuffer& vb, const std::vector<Vertex>& newData)
{
    if (newData.empty()) return true;
    if (newData.size() > vb.GetDesc().ByteWidth / sizeof(Vertex)) return false;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    auto* context = GetDevice()->GetContext();
    if (FAILED(context->Map(vb.GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        return false;
    memcpy(mapped.pData, newData.data(), sizeof(Vertex) * newData.size());
    context->Unmap(vb.GetBuffer(), 0);
    return true;
}''',caption='Dynamic 버퍼 업데이트 예시 — 생성 용량과 Map 성공 확인')
e.block(33,'''파티클 업데이트의 구조 예시
1. CPU 파티클 데이터에 위치·속도·수명을 저장한다.
2. 렌더링용 Vertex 배열에 위치·색상 등을 복사한다.
3. 생성해 둔 Dynamic 버퍼 용량 안에서 업데이트한다.
4. 선택한 topology와 셰이더에 맞는 Draw를 호출한다.
기본 Vertex에는 velocity가 없으므로 particle.velocity를 바로 사용하지 않는다.''','plain text')
e.block(36,'''인스턴싱 확장 순서 — 현재 VertexBuffer 인터페이스에 추가할 기능
1. InstanceData를 받는 버퍼 생성 경로를 만든다.
2. 메시 버퍼를 슬롯 0, 인스턴스 버퍼를 슬롯 1에 바인딩한다.
3. 인스턴스 입력의 InputSlotClass를 PER_INSTANCE_DATA로 설정한다.
4. InstanceDataStepRate와 행렬의 각 입력 semantic을 정의한다.
5. 인스턴스 데이터를 읽는 VS와 DrawInstanced를 연결한다.
기존 Create(vector<Vertex>)와 고정 슬롯 0의 Bind만으로는 완성되지 않는다.''','plain text')
e.block_replace(37,'HRESULT hr = GetDevice()->CreateBuffer(&desc, &sub, buffer.GetAddressOf());\n    if (FAILED(hr))','const bool created = GetDevice()->CreateBuffer(&desc, &sub, buffer.GetAddressOf());\n    if (!created)')
e.block_replace(37,'LOG_ERROR("Failed to create vertex buffer. HRESULT: 0x%08X", hr);','LOG_ERROR("Failed to create vertex buffer");')
e.block_replace(37,'size_t maxSize = 256 * 1024 * 1024; // 256 MB','size_t maxSize = 256 * 1024 * 1024; // 이 예제의 정책값, API 공통 상한이 아님')
e.block_replace(37,'    desc.ByteWidth = sizeof(Vertex) * vertexes.size();','    desc = {};\n    desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexes.size());')
e.finish('### 도형·메모리 예시 읽기\n\n원 예제는 중심과 둘레 정점만 생성합니다. DX11에는 Triangle Fan topology가 없으므로 삼각형 목록 인덱스 `(0, i+1, i+2)`를 만들어 DrawIndexed로 연결하고, segments는 3 이상인지 검사해야 합니다. 사각형도 정점 4개만으로 Triangle List Draw(4)가 완성되지 않습니다. `position`과 실제 프로젝트의 `pos` 같은 멤버명은 사용할 Vertex 선언과 맞춥니다.\n\n정점의 C++ 메모리 레이아웃은 `sizeof`·`offsetof`와 Input Layout으로 확인합니다. 패딩 예제의 BetterVertex는 flag 필드를 제거한 다른 데이터이므로 단순히 필드 순서만 바꿔 절약한 사례가 아닙니다. GPU 스키닝은 정점 버퍼를 매 프레임 CPU에서 갱신하지 않는 방식도 가능합니다.')

e=Edit(7,'IndexBuffer와 ConstantBuffer의 생성·바인딩·갱신 책임을 나누고, bool 래퍼와 HRESULT API의 차이를 확인합니다.','Release에서도 생성 실패가 반환되고, 상수 버퍼 크기·데이터 범위·바인딩 슬롯이 일치합니다.')
e.para('- 안전한 멀티스레드 사용 가능','- const 멤버 함수라고 해서 스레드 안전성이 보장되지는 않습니다. Bind는 Context의 외부 상태를 바꿉니다.')
e.para('- `DXGI_FORMAT_R16_UINT`: 16비트, 최대','- `DXGI_FORMAT_R16_UINT`: 인덱스 값 0~65,535를 표현합니다. Triangle List에서는 65,536개의 로컬 정점을 참조할 수 있으며 strip cut 설정은 별도로 고려합니다.')
e.para('- 권장 256 bytes 이하','- 256바이트 이하라는 공통 권장 제한은 없습니다. 실제 데이터와 업데이트 빈도에 맞춰 크기를 정합니다.')
e.para('- `D3D11_MAP_WRITE_DISCARD`: 기존 데이터 폐기, 가장 빠름','- `D3D11_MAP_WRITE_DISCARD`: Dynamic 리소스의 이전 내용을 유지하지 않고 새 값을 씁니다.')
e.block_replace(0,'\tdesc.ByteWidth = sizeof(UINT) * indices.size();','\tif (indices.empty() || indices.size() > UINT_MAX / sizeof(UINT)) return false;\n\tdesc = {};\n\tdesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());')
for n in (0,7,37):e.block_replace(n,'assert(NULL && "indices buffer create fail!!");','return false;')
e.block_replace(10,'\tmType = type;','\tif (!size || size % 16 != 0 || size > 65536) return false;\n\tdesc = {};\n\tmType = type;')
e.block_replace(10,'assert(NULL, "Create constant buffer failed!");','return false;')
e.block_replace(21,'mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);','if (!buffer || !data || !size) return;\n    D3D11_BUFFER_DESC desc = {};\n    buffer->GetDesc(&desc);\n    if (size > desc.ByteWidth) return;\n    if (FAILED(mContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) return;')
e.block_replace(29,'MaterialData data;','MaterialData data = {};')
e.block(33,'''현재 Create(vector<UINT>)는 32비트 인덱스 경로입니다.
16비트 지원을 추가하려면 vector<USHORT> 오버로드 또는 타입 인자를 만들고,
생성 시 저장한 포맷을 Bind에서도 R16_UINT로 사용해야 합니다.
벡터의 타입만 바꿔 기존 함수를 호출하면 컴파일되지 않습니다.''','plain text')
e.block(34,'''// HLSL 패킹 예시입니다. C++ 레이아웃은 별도로 맞춰야 합니다.
cbuffer Example : register(b0)
{
    float4 vector;
    float value1;
    float3 padding;
}; // 32 bytes''','hlsl')
e.block(35,'''// 전체 TransformData를 버퍼 크기에 맞춰 준비합니다.
for (auto& obj : objects)
{
    TransformData data = MakeTransformData(obj); // 프로젝트에서 구현할 준비 함수
    transformCB.SetData(&data);
    transformCB.Bind(eShaderStage::VS);
    obj.Render();
}
// 값이 같은지 비교해 생략하려면 버퍼의 마지막 전체 데이터와 비교합니다.
// 192바이트 버퍼에 Matrix 하나의 주소만 넘기면 범위를 넘겨 읽을 수 있습니다.''',caption='버퍼 크기와 전달 데이터의 타입을 맞추는 개념 예시')
e.block(36,'''풀링 확장 설계
버퍼 타입·크기·사용 여부를 기록하고, 반환한 포인터가 안정적으로 유지되게 한다.
vector<ConstantBuffer> 확장 후 기존 원소 포인터를 보관하면 무효화될 수 있다.
unique_ptr로 각 버퍼를 소유하거나 안정적인 핸들을 반환하는 방법을 사용한다.
GetType / GetSize / IsInUse / SetInUse는 별도로 구현할 인터페이스이다.
DX12의 재사용은 CPU 사용 여부뿐 아니라 GPU Fence 완료까지 확인한다.''','plain text')
e.block(38,'''// 이 엔진의 bool 래퍼를 호출하는 경우
if (!GetDevice()->CreateBuffer(&desc, &sub, buffer.GetAddressOf()))
    return false;
// 원래 ID3D11Device::CreateBuffer를 직접 호출할 때만 HRESULT와 FAILED를 사용합니다.''')
e.block_replace(39,'const std::wstring& name','const std::string& name')
e.block_replace(39,'name.size(),','static_cast<UINT>(name.size()),')
e.block_replace(40,'indexBuffer.SetDebugName(L"PlayerMesh_IndexBuffer");','#ifdef _DEBUG\nindexBuffer.SetDebugName("PlayerMesh_IndexBuffer");\n#endif')
e.block(41,'''bool ConstantBuffer::Create(eCBType type, UINT size, void* data)
{
    if (size == 0 || size % 16 != 0 || size > 65536) return false;
    // 여기서 위의 실제 생성 로직으로 이어집니다.
    // size만 반올림하고 원래 data를 그대로 전달하면 원본 범위를 넘겨 읽을 수 있습니다.
}''',caption='크기 검사의 발췌 — 전체 Create 함수를 대체하는 코드는 아님')
e.finish('### 기존 코드와 확장 예제 구분\n\n핵심 Create·Bind·SetData와 뒤의 풀링·16비트 지원·Mesh 통합 예시는 구분해서 읽습니다. `void*` 전달은 컴파일러가 데이터 타입과 크기를 자동으로 검증하지 않으므로 타입 안전성을 따로 보강해야 합니다. 상수 버퍼의 BindFlags는 단독 사용하지만 인덱스 버퍼 플래그까지 모두 조합 불가인 것은 아닙니다. DEFAULT 버퍼는 CPUAccessFlags가 0이어도 UpdateSubresource 또는 복사로 갱신할 수 있습니다. Transform의 CPU 전치 여부는 대응 HLSL의 row_major 설정과 함께 맞춥니다.')
