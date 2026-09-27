e=Edit(8,'Mesh가 정점·인덱스·Topology를 묶는 과정을 읽습니다. 구현 본문과 저장·LOD·인스턴싱 확장 예시를 구분합니다.','사각형의 정점 4개와 인덱스 6개가 DrawIndexed로 이어집니다.','flowchart LR\n    V[정점 속성] --> VB[Vertex Buffer]\n    I[정점 번호] --> IB[Index Buffer]\n    VB --> M[Mesh Bind]\n    IB --> M\n    T[Topology] --> M\n    M --> D[DrawIndexed]')
e.para('이렇게 중복이 되더라도','Triangle List는 정점 또는 인덱스를 세 개씩 묶는 도형 연결 방식입니다. 그림은 그중 인덱스를 사용하지 않는 예입니다. Indexed Triangle List도 같은 `D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST`를 사용하고 Draw 대신 DrawIndexed로 그립니다.')
e.para('삼각형 스트립은 다음과 같이 정점 버퍼 하나만','Triangle Strip은 앞 삼각형의 정점 두 개를 공유하며 이어지는 삼각형을 표현합니다. 아래 그림은 비인덱스 예제이며, Strip에도 인덱스 버퍼를 사용할 수 있습니다.')
e.para('- 최고의 캐시 효율','- 연속된 도형의 정점 참조 수를 줄일 수 있으나 실제 캐시 효율은 데이터 순서와 GPU에 따라 달라짐')
e.para('- **특수 메쉬**: Triangle Strip','- **특수 메쉬**: 연결된 띠 모양 등에서 Triangle Strip을 검토하고 실제 성능 측정')
e.para('- **Triangle List**: 단순하지만','- **Triangle List**: 삼각형마다 세 정점 참조, 인덱스 사용 여부는 별도 선택')
e.para('- **Triangle Strip**: 최고 효율','- **Triangle Strip**: 연결된 삼각형을 간결하게 표현, 항상 최고 성능인 것은 아님')
e.block(3,'Triangle List + Draw: 정점 세 개씩 읽음\nTriangle List + DrawIndexed: 인덱스 세 개로 정점을 재사용\nTriangle Strip: 첫 세 개 이후 정점 참조 하나마다 삼각형 추가\n위치가 같아도 UV·법선·색상이 다르면 서로 다른 정점입니다.\n메모리와 속도는 정점 크기·공유율·순서·GPU로 결정됩니다.','plain text')
e.block_replace(15,'return S_OK;','return E_NOTIMPL;')
for n,arr,typ in [(8,'vertices','Vertex'),(10,'indices','UINT')]:
    e.block_replace(n,'    D3D11_BUFFER_DESC bufferDesc = {};',f'    if ({arr}.empty() || {arr}.size() > UINT_MAX / sizeof({typ})) return false;\n    D3D11_BUFFER_DESC bufferDesc = {{}};')
    e.block_replace(n,f'bufferDesc.ByteWidth = sizeof({typ}) * {arr}.size();',f'bufferDesc.ByteWidth = static_cast<UINT>(sizeof({typ}) * {arr}.size());')
e.block_replace(16,'    return S_OK;','    return file.fail() ? E_FAIL : S_OK;')
e.block(17,'''// 파일 로더 설계 순서 — 실행 코드가 아닌 구현 체크리스트
// 1. magic/version, 고정 폭 정수로 저장된 개수, vertex format 확인
// 2. 모든 read의 성공 여부와 남은 파일 크기 확인
// 3. 정점·인덱스 개수에 프로젝트 메모리 상한 적용
// 4. 임시 배열에 읽고 모든 인덱스 < 정점 수인지 확인
// 5. CreateVB/CreateIB 성공을 확인한 뒤 완성된 Mesh를 등록
// 실패 시 기존 리소스를 유지하고 오류를 반환합니다.
// size_t와 C++ 구조체를 그대로 덤프한 위 Save 형식은
// 같은 빌드 내부의 임시 예시이며 교환용 파일 포맷이 아닙니다.''',caption='Load 구현 조건 — 검증 없이 파일의 개수만큼 할당하던 예제 정리')
for n in (18,20):
    e.block_replace(n,'    Mesh* mesh = new Mesh();','    for (auto& v : vertices) v.color = Vector4(1.f, 1.f, 1.f, 1.f);\n    Mesh* mesh = new Mesh();')
    e.block_replace(n,'    mesh->CreateVB(vertices);\n    mesh->CreateIB(indices);','    if (!mesh->CreateVB(vertices) || !mesh->CreateIB(indices))\n    { delete mesh; return nullptr; }')
e.block_replace(20,'    std::vector<Vertex> vertices;','    // 이 예제의 메모리 정책값입니다. API 자체의 상한은 아닙니다.\n    if (width < 1 || height < 1 || width > 2048 || height > 2048) return nullptr;\n    std::vector<Vertex> vertices;')
e.block_replace(20,'Vertex v;','Vertex v = {};')
e.block_replace(23,'// 압축된 정점 (20 bytes)','// 압축된 정점 (24 bytes, 아래 타입이 주석의 크기일 때)')
e.block_replace(23,'    // 58% 메모리 절약','    UINT packedColor;      // 4 bytes, RGBA8\n    // 48 → 24 bytes. 정밀도를 줄이고 색상 필드도 유지합니다.\n    // USHORT에는 float 수치가 아닌 변환된 half 비트 패턴을 저장합니다.')
e.block_replace(24,'        if (distance < 10.0f)','        if (mLODLevels.size() < 3) return nullptr; // 호출자가 대체 Mesh 선택\n        if (distance < 10.0f)')
e.finish('### 예제와 구현 범위\n\n`Mesh::CreateVB/CreateIB/Bind`가 이 단계의 핵심입니다. 버퍼 내부의 `mBuffer`, 정점의 `position` 등은 설명용 이름이므로 프로젝트 선언과 맞춥니다. `GetIndexCount`, 인스턴스 버퍼, LOD 선택, 캐시 최적화와 저장 포맷은 별도 구현이 필요한 확장 설계입니다. 큐브 예제는 한 면만 보인 발췌이며 완성된 6면 메시가 아닙니다. 인스턴싱에는 별도 버퍼뿐 아니라 per-instance Input Layout과 바인딩이 필요합니다.\n\nCPU의 `mData`를 수정한다고 GPU 버퍼가 자동으로 갱신되지는 않습니다. 명시적인 재생성 또는 갱신 경로를 호출해야 합니다.')

e=Edit(9,'이미지를 CPU에서 디코딩한 뒤 GPU 텍스처와 SRV를 만들어 HLSL의 t 슬롯으로 연결합니다.','파일 로드·GPU 리소스·SRV·Sampler의 역할과 소유권을 구분합니다.','flowchart LR\n    F[이미지 파일] --> C[ScratchImage: CPU 디코딩]\n    C --> T[GPU Texture]\n    T --> V[SRV: 셰이더에서 보는 방법]\n    V --> P[HLSL t0]\n    S[Sampler s0] --> P')
e.para('**Texture(텍스처)**는','**Texture(텍스처)**는 셰이더가 사용할 이미지 등의 데이터를 담는 리소스입니다. 이 문서는 표면 색상을 표현하는 2D 텍스처에 집중합니다. 1D·3D·배열·큐브 텍스처도 있으며 모든 텍스처가 단순한 2D 사진인 것은 아닙니다.')
e.para('**텍스처 리소스 생성 →','**이미지 파일 디코딩 → GPU 텍스처 생성·데이터 전달 → SRV 생성 → 셰이더에 바인딩**')
e.para('- **Unordered Access View','- **Unordered Access View (UAV)**: 지원 단계와 형식에서 읽기·쓰기에 사용합니다. Compute 전용 뷰는 아닙니다.')
e.para('- **자동 포맷 변환**','- **포맷 변환**: 필요할 때 Convert 같은 함수를 명시적으로 호출')
e.para('- **밉맵 생성**: 자동','- **밉맵 생성**: GenerateMipMaps를 명시적으로 호출하거나 미리 생성된 밉을 로드')
e.para('- 참조 카운팅 또는 공유 리소스 지원','- Resource 상속만으로 참조 카운팅이 생기지는 않습니다. 매니저와 객체의 실제 소유·해제 규칙을 확인합니다.')
e.para('CreateShaderResourceView는 셰이더 리소스 뷰를','여기서 `DirectX::CreateShaderResourceView`는 DirectXTex 도우미로, GPU 텍스처와 SRV를 함께 만듭니다. 이름이 같은 `ID3D11Device::CreateShaderResourceView`는 이미 존재하는 리소스에 뷰만 만듭니다. [DirectXTex 함수 문서](https://github.com/microsoft/DirectXTex/wiki/CreateShaderResourceView)')
e.para('텍스처 리소스는 기본적으로 Release','생성 함수와 GetResource로 얻은 소유 참조는 모두 해제해야 합니다. ComPtr가 소유하면 자동으로 Release하며, raw pointer로 소유하면 해당 참조를 직접 Release합니다. ComPtr 안의 포인터를 별도로 Release하면 중복 해제가 됩니다.')
e.para('- `input.uv`: 텍스처 좌표','- `input.uv`: 텍스처 좌표입니다. 0~1 밖의 값은 Sampler의 주소 모드로 처리합니다.')
e.para('- 자동 밉맵 생성 및 포맷 변환','- 명시적 밉맵 생성 및 포맷 변환 지원')
e.replace('- 최신 DirectX 권장 방식\n**텍스처 시스템 구조**','- 이미지 처리·에디터 도구에 적합합니다. 런타임 단순 로더는 DirectXTK의 DDS/WIC Texture Loader도 검토합니다.\n**텍스처 시스템 구조**')
e.block(0,'D3DX: 과거 SDK의 도우미 라이브러리\nDirectXTex: 이미지 디코딩·변환·밉 생성·압축 등의 도구 기능\nDirectXTK DDS/WIC Texture Loader: 단순 런타임 로드에 적합한 별도 도우미\n필요한 포맷과 처리 기능을 기준으로 선택합니다.','plain text')
e.block_replace(1,'D3D11_TEXTURE2D_DESC mDesc;','D3D11_TEXTURE2D_DESC mDesc = {};')
e.block_replace(4,'.extension();','.extension().wstring();')
e.block_replace(4,'mSRV.GetAddressOf()','mSRV.ReleaseAndGetAddressOf()')
resource='''Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    mSRV->GetResource(resource.GetAddressOf());
    mTexture.Reset();
    hr = resource.As(&mTexture);
    if (FAILED(hr)) return hr; // 이 클래스는 Texture2D를 다룹니다.
    mTexture->GetDesc(&mDesc);'''
e.block_replace(4,'mSRV->GetResource((ID3D11Resource**)mTexture.GetAddressOf());',resource)
e.block(5,'Microsoft::WRL::ComPtr<ID3D11Resource> resource;\nmSRV->GetResource(resource.GetAddressOf());\nmTexture.Reset();\nHRESULT hr = resource.As(&mTexture);\nif (FAILED(hr)) return hr;\nmTexture->GetDesc(&mDesc);',caption='HRESULT 멤버 함수 내부 — QueryInterface를 사용하는 안전한 형식 변환')
e.block_replace(17,'ID3D11Texture2D* texture','ID3D11Resource* texture')
e.block_replace(17,'(ID3D11Resource**)&texture','&texture')
e.block_replace(19,'    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};','    mTexture->GetDesc(&mDesc);\n    if (!(mDesc.BindFlags & D3D11_BIND_RENDER_TARGET) ||\n        mDesc.SampleDesc.Count != 1 || mDesc.ArraySize != 1) return E_INVALIDARG;\n    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};')
e.block(20,'''// RGBA8, 단일 샘플·단일 배열 텍스처의 mip 0 읽기 예시.
// HRESULT를 반환하는 함수 내부이며 vector/cstdint/cstring 헤더가 필요합니다.
if (mDesc.Format != DXGI_FORMAT_R8G8B8A8_UNORM ||
    mDesc.SampleDesc.Count != 1 || mDesc.ArraySize != 1) return E_INVALIDARG;
D3D11_TEXTURE2D_DESC stagingDesc = mDesc;
stagingDesc.Usage = D3D11_USAGE_STAGING;
stagingDesc.BindFlags = 0;
stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
stagingDesc.MiscFlags = 0;
Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
HRESULT hr = GetDevice()->GetID3D11Device()->CreateTexture2D(
    &stagingDesc, nullptr, staging.GetAddressOf());
if (FAILED(hr)) return hr;
auto* context = GetDevice()->GetContext();
context->CopyResource(staging.Get(), mTexture.Get());
const size_t rowBytes = size_t(mDesc.Width) * 4;
std::vector<uint8_t> pixels(rowBytes * mDesc.Height);
D3D11_MAPPED_SUBRESOURCE mapped = {};
hr = context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
if (FAILED(hr)) return hr;
for (UINT y = 0; y < mDesc.Height; ++y)
    memcpy(pixels.data() + y * rowBytes,
        static_cast<const uint8_t*>(mapped.pData) + y * mapped.RowPitch, rowBytes);
context->Unmap(staging.Get(), 0);
// pixels를 사용합니다. Map은 복사 완료를 기다릴 수 있습니다.''')
e.block(21,'작업 스레드: COM 초기화(WIC 사용 시) → 파일 읽기·디코딩 → 결과 전달\n메인/리소스 스레드: 결과 수신 → GPU 리소스 생성 → 매니저에 등록\n종료: 작업 완료·취소 처리 → 작업 스레드 join → 객체 파괴\nthis를 캡처한 비동기 Load만으로는 객체 수명과 공유 상태가 보호되지 않습니다.','plain text',caption='비동기 로딩 확장 설계 — 이 예제만으로 구현 완료되지 않음')
e.block_replace(22,'// 로드 시 밉맵 자동 생성','// GPU 텍스처 생성 전에 명시적으로 실행합니다.')
e.block_replace(22,'if (!mImage.GetMetadata().mipLevels > 1)','if (mImage.GetMetadata().mipLevels <= 1)')
e.block_replace(22,'    GenerateMipMaps(','    HRESULT hr = GenerateMipMaps(')
e.block_replace(22,'    mImage = std::move(mipChain);','    if (FAILED(hr)) return hr;\n    mImage = std::move(mipChain);')
e.block(24,'''// ScratchImage의 논리적 이미지 바이트 수입니다. 실제 GPU 할당량과 다릅니다.
size_t GetImageBytes(const DirectX::ScratchImage& image)
{
    size_t bytes = 0;
    const auto* images = image.GetImages();
    for (size_t i = 0; i < image.GetImageCount(); ++i)
        bytes += images[i].slicePitch;
    return bytes;
}
// mip마다 크기가 다르므로 기본 레벨 크기 × mip 개수로 계산하면 안 됩니다.''')
e.finish('### 적용 범위와 그림 읽기\n\n앞의 구조 그림은 **하나의 리소스와 여러 접근 뷰의 관계**를 나타냅니다. 모든 파일 텍스처에 RTV를 추가할 수 있다는 뜻은 아닙니다. 생성할 때 Render Target 바인딩 플래그와 지원 포맷을 갖춰야 합니다. Staging 예제는 RGBA8의 행 간격(RowPitch)을 고려하며, BC 압축·MSAA·배열은 별도 경로가 필요합니다.\n\nWIC 로드는 호출 스레드에서 COM 초기화가 필요합니다. DirectXTex 버전에 따라 TGA 함수 시그니처가 다를 수 있으므로 설치된 헤더와 맞춥니다. 여러 텍스처를 샘플링하는 HLSL은 바인딩 예시이며 완성된 PBR 조명 모델은 아닙니다. [DirectXTex 공식 안내](https://github.com/microsoft/DirectXTex/wiki/DirectXTex)\n\nDX12의 업로드·Resource Barrier·descriptor 수명은 다음 문서에서 이어집니다.\n<mention-page url="'+NEW1+'"/>')

e=Edit(10,'UV로 읽을 위치를 정하고 Sampler로 필터·주소·LOD를 정합니다. 각 비교 그림에서는 확대, 축소, 비스듬한 바닥 중 어떤 조건인지 먼저 확인합니다.','같은 텍스처를 Point/Linear로 바꿨을 때의 차이와 Wrap/Clamp의 경계 동작을 설명할 수 있습니다.')
e.para('포인트 필터링은 축소되거나','포인트 필터링은 해당 밉 레벨에서 가장 가까운 텍셀 하나를 선택합니다. 여러 텍셀의 색을 섞지 않기 때문에 확대하면 픽셀의 네모난 형태가 유지됩니다.')
e.para('- 3D: 8개의 텍셀 보간','- 이 2D 텍스처 문맥에서는 인접한 두 밉 레벨에서 각각 4개 텍셀을 쌍선형 보간하고, 두 결과를 다시 보간합니다. 3D 볼륨 텍스처의 8개 이웃 보간과 구분합니다.')
e.para('비등방성이란 물체의 물리적','비등방성 필터링은 화면 픽셀을 텍스처 공간에 투영했을 때 길게 늘어나는 영역을 고려합니다. 특히 비스듬한 바닥처럼 방향별 축소율이 다른 표면에서 세부 무늬를 유지하는 데 도움이 됩니다.')
e.para('밉매핑의 기반은 텍스처가','\t비스듬한 표면은 텍스처의 두 방향이 서로 다른 비율로 축소됩니다. 하나의 축소율만으로 밉 레벨을 선택하면 한 방향이 지나치게 흐려질 수 있습니다.')
e.para('예를 들어 원 텍스처의 크기가','\t일반적인 밉 체인은 1024×1024 → 512×512 → 256×256처럼 줄어듭니다. 하드웨어 비등방성 필터링은 이 밉 체인에서 늘어난 영역을 따라 여러 위치를 샘플링하는 방식으로 필터링합니다.')
e.para('따라서 가로축은 그대로이고','\t가로·세로 해상도를 따로 줄인 모든 이미지를 저장하는 방식은 ripmap의 아이디어입니다. D3D11의 Anisotropic Sampler를 켜기 위해 그런 이미지를 직접 만들 필요는 없습니다.')
e.para('이렇게 1024×512, 1024×256','\t아래 축별 해상도 그림은 ripmap 개념의 참고 그림입니다. 일반적인 하드웨어 비등방성 필터링 구현 자체로 읽지 않습니다. [Microsoft: 비등방성 필터링](https://learn.microsoft.com/en-us/windows/uwp/graphics-concepts/anisotropic-texture-filtering)')
e.para('- **단점**: 성능 비용 높음, 메모리','- **비용**: 샘플링 작업이 늘 수 있습니다. 같은 밉 체인을 사용하며, 별도의 ripmap 메모리를 반드시 추가하는 기능은 아닙니다.')
e.para('- 음수: 더 선명한 밉맵','- 음수: 더 높은 해상도의 밉을 선택하도록 치우치며, 깜빡임·앨리어싱이 늘 수 있습니다.')
e.para('- 비등방성 필터링의 샘플 수','- 허용할 비등방성의 상한입니다. 매 픽셀의 고정 샘플 수와 같은 뜻은 아닙니다.')
e.para('- `D3D11_COMPARISON_NEVER`: 비교','- `D3D11_COMPARISON_NEVER`는 비교 결과가 항상 실패라는 뜻입니다. 일반(non-comparison) 필터에서는 이 필드가 사용되지 않습니다.')
e.para('- 1\\~2: 미러 (한 번만)','- 좌표에 절댓값을 적용한 뒤 0~1로 Clamp합니다. 예: -0.3 → 0.3, 1.3 → 1.0.')
e.para('- 2 이상: CLAMP','- 따라서 1~2 구간을 한 번 더 반사하는 방식이 아닙니다. [주소 모드 정의](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_texture_address_mode)')
e.para('비등방성 필터링을 사용하려면','비등방성 필터와 MaxAnisotropy를 함께 설정합니다. 보통 1·2·4·8·16을 비교하지만, 필요한 상한은 장면·해상도·GPU에 따라 정합니다. 모든 장면에서 항상 가장 느리다는 절대 순위로 해석하지 않습니다.')
e.para('- 나머지 함수들은 모든 셰이더','- 각 함수의 Shader Model·텍스처 형식·스테이지 조건을 확인합니다. SampleGrad는 명시적인 변화율을 받아 VS에서도 지원되지만, `ddx/ddy` 화면 미분을 VS에서 계산할 수 있다는 뜻은 아닙니다. [SampleGrad 지원 표](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-samplegrad)')
e.block(0,'일반적인 Direct3D 2D 텍스처 좌표\n(0,0): 이미지 왼쪽 위 / (1,1): 이미지 오른쪽 아래 경계\n이것이 화면의 어느 위치에 보이는지는 메시의 UV 배치로 결정됩니다.\n텍셀 (i,j)의 중심은 ((i+0.5)/width, (j+0.5)/height)입니다.','plain text')
e.block_replace(2,'UV 좌표: (0.3, 0.7)','인접한 네 텍셀 사이의 로컬 보간 비율: (0.3, 0.7)\n아래 0과 1은 전체 텍스처의 UV가 아니라 이웃 텍셀 간 비율입니다.')
e.block(3,'Point: 텍셀 경계를 유지 — 픽셀 아트 등\nLinear: 이웃 색을 보간 — 확대 시 부드러움\nTrilinear: 두 밉 레벨 사이까지 보간\nAnisotropic: 방향별 축소율을 고려 — 비스듬한 표면\n품질은 의도한 표현으로, 비용은 실제 GPU 시간으로 비교합니다.','plain text')
e.block_replace(5,'samplerDesc.MinLOD = 0;','samplerDesc.MaxAnisotropy = 16;\nsamplerDesc.MinLOD = 0;')
e.block_replace(5,'GetDevice()->CreateSamplerState(','// 이 엔진 래퍼는 bool을 반환합니다. 실패 시 초기화를 중단합니다.\nif (!GetDevice()->CreateSamplerState(')
e.block_replace(5,'.GetAddressOf());','.GetAddressOf()))\n    return false; // bool 초기화 함수 내부')
e.block_replace(13,'MaxAnisotropy = 4;','MaxAnisotropy = 1; // LINEAR에서는 이 값이 적용되지 않습니다.')
e.block(16,'''// C++: 기존 samplerDesc의 나머지 유효 설정을 유지합니다.
samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
// 이 desc로 실제 SamplerState를 생성해 PS의 s4에 바인딩합니다.
```

**HLSL — 비교 샘플러를 읽는 슬롯**

```hlsl
Texture2D<float> shadowMap : register(t4);
SamplerComparisonState shadowSampler : register(s4);
// PS 함수 내부: uv와 depth는 해당 표면의 그림자 좌표입니다.
// float shadow = shadowMap.SampleCmpLevelZero(shadowSampler, uv, depth);''',lang='c++')
e.block(17,e.blocks[17][3],lang='hlsl')
e.block_replace(21,'// 초기화 시 한 번만 생성','// HLSL의 슬롯 선언입니다. C++에서 만든 상태 객체를 이 슬롯에 연결합니다.')
e.block_replace(22,'// 같은 설정의 샘플러를 여러 개 생성 (비효율적)','// 슬롯 선언만으로 GPU SamplerState 객체가 생성되지는 않습니다.\n// C++에서 같은 상태 객체를 여러 슬롯에 바인딩할 수도 있습니다.')
e.block(23,'''// D3D11 GPU 밉 생성의 필요한 단계 — 설정 발췌
// Width/Height/Format/ArraySize/SampleDesc도 유효하게 설정해야 합니다.
texDesc.MipLevels = 0; // 전체 체인 공간 확보, 이미지 생성 자체는 아님
texDesc.Usage = D3D11_USAGE_DEFAULT;
texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
// 지원 포맷을 확인하고 Texture와 전체 밉을 포함하는 SRV 생성
// UpdateSubresource로 mip 0을 채운 다음:
context->GenerateMips(shaderResourceView);''')
e.block(25,'측정 조건: 같은 카메라·텍스처·해상도·Draw 수 유지\n변경 항목: 필터와 MaxAnisotropy만 변경\n기록: GPU 시간, 먼 무늬의 깜빡임, 경사면 선명도\n하드웨어와 장면에 무관한 1.2배~5배 비용 표는 사용하지 않습니다.','plain text')
e.finish('### 샘플러 코드와 비교 이미지 읽기\n\nC++의 `CreateSamplerState`가 실제 상태 객체를 만들고 `PSSetSamplers`가 슬롯에 연결합니다. HLSL의 `SamplerState : register(s0)`는 그 슬롯을 읽는 선언입니다. 이미지의 Point/Linear 차이는 확대 조건으로, Anisotropic 차이는 경사진 표면으로 비교해야 합니다. Normal/Roughness를 샘플링하는 블록은 텍스처 조합 예시이며 조명 계산 전체는 포함하지 않습니다. 이 문서의 셰이더 제약 설명은 DX11의 전통적인 SM4/5 경로를 기준으로 읽습니다.')

e=Edit(11,'Material은 외관과 렌더 상태를, Input Layout은 정점 바이트의 해석을 맡습니다. 두 책임을 분리해 읽습니다.','C++ 구조체·Input Layout·VS 입력 시맨틱 세 곳이 같은 데이터를 가리킵니다.','flowchart LR\n    C[C++ Vertex 메모리] --> I[Input Layout: 형식·오프셋·시맨틱]\n    I --> V[Vertex Shader 입력]\n    M[Material] --> S[Shader·Texture·State]\n    S --> D[Draw]\n    V --> D')
e.para('- `Save()`와 `Load()` 메서드를','- Save/Load 인터페이스가 선언되어 있습니다. 실제 직렬화 구현 여부는 각 함수 본문으로 확인합니다.')
e.para('- 불투명(Opaque), 투명(Transparent), 가산','- 이 엔진의 기본 렌더 모드는 Opaque, CutOut, Transparent입니다. 가산 블렌드 상태와 렌더 큐 열거형은 같은 개념이 아닙니다.')
e.para('그 다음 버텍스 버퍼에 넘겨진','정점별 데이터는 PER_VERTEX_DATA와 StepRate 0을 사용합니다. 인스턴스별 데이터는 PER_INSTANCE_DATA로 지정하고, **같은 요소를 몇 인스턴스 동안 사용할지** StepRate로 정합니다. 1이면 매 인스턴스마다 다음 요소를 읽습니다. 총 그릴 인스턴스 수는 DrawInstanced/DrawIndexedInstanced의 별도 인자입니다. [Input Element 정의](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_input_element_desc)')
e.para('- 정점 버퍼가 바인딩될 슬롯 번호','- 정점 버퍼가 바인딩될 슬롯 번호입니다. 허용 개수는 Feature Level을 확인하고, 이 입문 예제는 0번 슬롯 하나를 사용합니다.')
e.para('DirectX 11에서는 **성능을 향상시키기','Input Layout 생성 시 정점 요소 선언과 VS의 컴파일된 입력 시그니처를 함께 검증합니다. 실제 VB의 stride·offset·수명과 Draw 시의 상태까지 모두 자동으로 보장하는 것은 아니므로 Debug Layer에서 실행 중 오류도 확인합니다.')
e.block_replace(4,'// 입력 슬롯 번호 (0~15)','// 입력 슬롯 번호 (지원 Feature Level 확인)')
for n in (6,8):
    for a,b in [('float3','DirectX::XMFLOAT3'),('float4','DirectX::XMFLOAT4'),('float2','DirectX::XMFLOAT2')]:
        if a in e.blocks[n][3]:e.block_replace(n,a,b)
e.block(12,'''// <stdexcept> 필요. 엔진의 CreateInputLayout 래퍼는 bool 반환.
void InputLayout::CreateInputLayout(UINT elementCount,
    D3D11_INPUT_ELEMENT_DESC* layout,
    const void* shaderBytecode, SIZE_T bytecodeLength)
{
    // elementCount는 정점 수가 아니라 POSITION/COLOR 등의 요소 수입니다.
    if (!GetDevice()->CreateInputLayout(layout, elementCount,
        shaderBytecode, bytecodeLength, mInputLayout.ReleaseAndGetAddressOf()))
        throw std::runtime_error("Create input layout failed");
}''')
e.block(14,e.blocks[14][3],lang='plain text')
e.finish('### 이 단계에 구현된 범위\n\n위 Material 헤더의 Data는 albedo 경로만 포함하고, Bind 본문의 실동작은 Shader 바인딩부터 시작합니다. 주석으로 제시된 Texture·State·Material 상수 갱신, PBR·Toon·직렬화는 이 헤더만으로 완성되지 않습니다. GameObject 렌더링 블록은 호출 책임을 보여 주는 조합 예시이므로 사용 중인 컴포넌트 인터페이스에 맞춰 연결합니다.\n\nInput Layout의 숫자 오프셋은 선언된 C++ 구조체의 `offsetof`와 `sizeof`로 확인하고, Shader 검색·컴파일 성공을 확인한 뒤 Blob을 읽습니다. 이 문서의 단순 이동 VS는 당시 학습 단계 코드이며 최신 World/View/Projection 경로와 구분합니다. DX12에서는 Material 모드를 공유 Shader의 PSO 선택에 전달하는 방식으로 이어집니다.\n<mention-page url="'+NEW2+'"/>')
