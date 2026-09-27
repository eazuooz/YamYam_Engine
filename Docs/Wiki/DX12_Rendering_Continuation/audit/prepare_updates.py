from pathlib import Path
import json, re, textwrap

HERE = Path(__file__).resolve().parent
ITEMS = json.loads((HERE/'inventory.json').read_text(encoding='utf-8'))
PLANS = []
PACK = 'https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules'
MAP = 'https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map'
RASTER = 'https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-getting-started'
NEW1 = 'https://app.notion.com/p/3dc0b1ffa61e814e9e8bd403ca145ebe'
NEW2 = 'https://app.notion.com/p/3dc0b1ffa61e8153b02fd321cc48e655'

class Edit:
    def __init__(self, i, purpose, check, diagram=None):
        self.i=i; self.item=ITEMS[i]
        data=json.loads((HERE/'original'/f"{self.item['id']}.json").read_text(encoding='utf-8'))
        self.original=data['text'].split('<content>\n',1)[1].split('\n</content>',1)[0]
        self.body=self.original; self.updates=[]; self.notes=[]
        self.blocks=list(re.finditer(r'(?m)^([\t ]*)```([^\n]*)\n([\s\S]*?)^\1```', self.original))
        self.code_changes={}
        group=self.item['group']
        stage = 'DX11 학습 단계' if group=='DX11' else 'DX12 개념·구현 단계'
        self.guide=f'<callout icon="📖" color="blue_bg">\n\t**{stage} · 읽는 순서**\n\t{purpose}\n\t**확인할 결과:** {check}\n</callout>\n'
        if diagram: self.guide+='\n```mermaid\n'+diagram+'\n```\n'
    def replace(self, old, new, note=''):
        if old==new: return
        if self.body.count(old)!=1: raise ValueError(f'Page {self.i} expected unique match ({self.body.count(old)}): {old[:100]}')
        self.body=self.body.replace(old,new,1)
        self.updates.append({'old_str':old,'new_str':new})
        if note:self.notes.append(note)
    def para(self, fragment, new, note=''):
        lines=self.body.splitlines()
        matches=[line for line in lines if fragment in line]
        if len(matches)!=1:raise ValueError(f'Page {self.i} paragraph matches {len(matches)}: {fragment}')
        self.replace(matches[0],new,note)
    def block(self, n, new, lang='c++', caption=None):
        self.code_changes[n]=(textwrap.dedent(new).strip(),lang,caption)
    def block_replace(self,n,old,new):
        if n in self.code_changes: code,lang,caption=self.code_changes[n]
        else: code,lang,caption=self.blocks[n][3].rstrip(),self.blocks[n][2],None
        if old not in code:raise ValueError(f'Page {self.i} code {n}: missing {old}')
        self.code_changes[n]=(code.replace(old,new),lang,caption)
    def finish(self, extra=''):
        if self.i in {14,15,40,41,42,43,44,45}:
            return  # User excluded mathematics pages; restored original content.
        for n,b in enumerate(self.blocks):
            if b[0] not in self.body and n not in self.code_changes:
                continue  # A reviewed section may have been consolidated deliberately.
            code,lang,caption=self.code_changes.get(n,(b[3].rstrip(),b[2],None))
            if lang in ('javascript','glsl','c++','c',''):
                if re.search(r'\b(D3D11_|D3D12_|ID3D11|ID3D12)|std::|ComPtr<',code):lang='c++'
                elif re.search(r'\b(cbuffer|SV_POSITION|SV_Position|SV_TARGET|SV_Target|sampler2D|gl_Position)\b|register\([bstu][0-9]',code):
                    lang='glsl' if 'gl_Position' in code or 'sampler2D' in code else 'hlsl'
                elif re.search(r'#include|\b(D3D\w*|ID3D\w*|DXGI_\w*|std::|class |struct |enum |void |HRESULT|ComPtr)',code):lang='c++'
                elif lang=='javascript':lang='plain text'
            if b[1]: continue  # Keep nested structure intact; nested corrections use explicit replacements.
            new=(f'**{caption}**\n\n' if caption else '')+'```'+lang+'\n'+code+'\n```'
            self.replace(b[0],new)
        # Preserve native image blocks. Their temporary URL text is not a stable edit target.
        first=next(line for line in self.body.splitlines() if line.strip() and line!='<empty-block/>' and not line.startswith(('<unknown','<video','<image','![')))
        if self.body.count(first)!=1:
            first='\n'.join(self.body.splitlines()[:8])
        if extra and re.search(r'!\[|<image', '\n'.join(self.body.rstrip().splitlines()[-6:])):
            self.guide+='\n'+extra+'\n'
            extra=''
        self.replace(first,self.guide+'\n'+first)
        if extra:
            last=self.body.rstrip().splitlines()[-1]
            if self.body.count(last)!=1:last='\n'.join(self.body.rstrip().splitlines()[-6:])
            self.replace(last,last+'\n\n'+extra)
        plan={'page_id':self.item['id'],'title':self.item['title'],'index':self.i,'notes':self.notes,'content_updates':self.updates}
        PLANS.append(plan)
        (HERE/'revised').mkdir(exist_ok=True)
        (HERE/'revised'/f"{self.item['id']}.md").write_text(self.body,encoding='utf-8',newline='\n')

e=Edit(0,'정점이 화면의 색으로 바뀌는 순서를 먼저 살펴봅니다. 필수 경로인 IA → VS → Rasterizer → PS → OM을 익힌 뒤 선택 단계인 테셀레이션·GS·Compute를 읽습니다.','정점 데이터, 좌표 변환, 픽셀 처리, 깊이·블렌딩의 역할을 서로 구분할 수 있습니다.','flowchart LR\n    A[정점과 인덱스] --> I[Input Assembler]\n    I --> V[Vertex Shader]\n    V --> R[Clipping · 원근 나눗셈 · Rasterizer]\n    R --> P[Pixel Shader]\n    P --> O[깊이·스텐실·블렌딩]\n    O --> T[Render Target]')
e.para('여러 명령어가 중첩되어서','파이프라인은 하나의 작업을 여러 단계로 나누어 처리하는 구조입니다. 그래픽스에서는 앞 단계의 출력이 다음 단계의 입력이 됩니다. 서로 다른 정점·도형·픽셀의 처리가 겹쳐 진행될 수 있지만, 모든 단계가 하나의 클록 사이클 안에 끝나야 한다는 뜻은 아닙니다.','파이프라인을 한 사이클에 묶는 설명 수정')
e.para('여기서 모델은 점(vertex)','모델의 표면은 정점(vertex)과 그 정점들을 연결하는 도형으로 표현합니다. 폴리곤은 단순한 점의 집합이 아니라 정점과 변으로 둘러싸인 다각형이며, 이 강의에서는 삼각형을 기본 단위로 사용합니다.')
e.para('사각형을 구성하는 정점을 4개만','사각형의 정점 4개를 정점 버퍼에 저장하고 인덱스 버퍼에 0, 1, 2, 0, 2, 3처럼 삼각형을 구성하는 정점 번호를 저장하면 정점을 재사용할 수 있습니다. 한 붓 그리기와는 다른 개념이며, 인덱스 없이도 Draw로 그릴 수 있습니다.')
e.para('Local Space라고도 불리는','Local Space(오브젝트 공간)는 각 모델이 자신의 원점과 축을 기준으로 정의된 좌표 공간입니다. World 행렬을 적용하면 여러 모델이 공통된 World Space에 배치됩니다.')
e.para('바로 Z좌표(거리)로 모든 성분을', '원근 나눗셈은 클립 좌표 `(x_c, y_c, z_c, w_c)`의 x·y·z를 **클립 좌표의 w**로 나누어 NDC를 만드는 과정입니다. 일반적인 원근 투영에서 w는 뷰 공간 z와 관련되지만, 투영 후 z 자체로 나누는 것은 아닙니다. 결과는 `(x_c/w_c, y_c/w_c, z_c/w_c)`이며, Direct3D의 가시 NDC 범위는 x·y가 [-1, 1], z가 [0, 1]입니다.','원근 나눗셈과 깊이 공식 수정')
e.para('//(x/z, y/z, z/z, w(z))','`clip → (clip.xyz / clip.w) → NDC → viewport → 화면 좌표`')
e.para('카메라가 바라보고 있는 방향에 물체에 가려진 면적은','뒷면 제거는 삼각형이 화면에 투영된 뒤의 정점 회전 방향(winding)과 rasterizer의 CullMode·FrontCounterClockwise 설정으로 앞면·뒷면을 판정하는 기능입니다. 다른 물체에 가려졌는지는 별도의 깊이 검사 문제입니다.')
e.para('1. 이 삼각형이 포함하는 모든 픽셀마다','1. 래스터라이저가 도형의 커버리지를 계산하고 픽셀 셰이더에 필요한 입력을 만듭니다. 실제 셰이더 호출 수는 MSAA, 보조 호출, 조기 깊이 검사 등의 영향을 받으므로 항상 화면 픽셀 수와 일치하지는 않습니다.')
e.para('Directx에서는 이러한 과정을 통틀어서','래스터라이저는 셰이더 코드로 알고리즘 자체를 바꾸는 단계는 아니지만, culling·fill mode·scissor·viewport 등 상태를 설정할 수 있습니다. 기본 연산은 클리핑, 원근 나눗셈, 화면 좌표 변환과 커버리지 계산입니다. [Microsoft: Rasterizer 설정]('+RASTER+')')
e.para('데이터들은 보간(선형 보간)되어서','스캔 변환은 화면의 삼각형이 어느 샘플을 덮는지 결정합니다. 정점 속성은 보간되어 픽셀 셰이더로 전달되며, HLSL의 기본 선형 보간은 원근 보정을 포함합니다. `noperspective`, `nointerpolation` 등으로 보간 방식을 지정할 수 있습니다.')
e.finish()

e=Edit(1,'Device·Context·SwapChain의 역할을 나누고, 백버퍼 RTV와 깊이 DSV를 만들어 첫 프레임을 표시합니다.','창에 지정한 clear 색이 보이고, Debug Layer에 초기화 오류가 없습니다.','flowchart LR\n    D[Device: 리소스 생성] --> R[백버퍼 RTV + 깊이 DSV]\n    C[Context: 상태 설정과 Draw] --> R\n    R --> S[SwapChain Present]')
e.para('ComPtr 객체는 스마트 포인터','ComPtr는 COM 인터페이스의 참조 횟수를 관리하는 스마트 포인터입니다. 소유권이 끝날 때 `delete`가 아니라 `Release()`를 호출합니다. 이미 다른 인터페이스를 보유한 출력 인자에는 `ReleaseAndGetAddressOf()`가 필요한지 확인하고, 같은 raw pointer를 중복 해제하지 않도록 합니다.')
e.para('디바이스를 생성할 때 낮은 피처레벨','Feature Level은 지원 기능 집합입니다. 낮은 레벨을 요청하면 그 기능 범위에 맞는 GPU에서 D3D11 API를 사용할 수 있다는 뜻이며, DirectX 9나 10 API가 DirectX 11 API로 바뀐다는 뜻은 아닙니다.')
e.para('위 소스에서는 깊이값만을','위 Draw 예제는 `D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL`을 사용하므로 깊이와 스텐실을 모두 지웁니다. 깊이만 지우려면 `D3D11_CLEAR_DEPTH`만 지정합니다.')
e.para('<span color="blue">**Constant Buffer**','<span color="blue">**Constant Buffer**</span>: 셰이더에 상수 데이터를 전달하는 버퍼입니다. 이름의 constant는 정수(integer)를 뜻하지 않습니다. float, 정수, 벡터, 행렬 등을 담을 수 있습니다.')
e.para('이것은 개념적으로는 버텍스 버퍼','정점 버퍼와 같은 ID3D11Buffer 리소스를 사용하지만 바인딩 플래그, 크기 정렬, HLSL 패킹 규칙이 다릅니다.')
e.para('각 스테이지에는\u00a0D3D11_COMMON_SHADER','각 셰이더 단계에는 API로 사용할 수 있는 상수 버퍼 슬롯이 14개 있습니다.')
e.para('각 정수 버퍼에는 최대','D3D11의 기본 상수 버퍼 범위는 16바이트 레지스터 4096개, 즉 64 KiB입니다. 정수 4096개만 저장한다는 뜻은 아닙니다.')
e.block(0,'''class Application
{
public:
    virtual ~Application() = default;
    void Run();
    virtual void Initialize();
    virtual void Update(float dt);
    virtual void FixedUpdate();
    virtual void Render();
    void SetWindow(HWND hwnd, UINT width, UINT height);
    // 창 정보와 GraphicDevice 소유 멤버는 프로젝트 선언에 이어서 둡니다.
};''',caption='Application 인터페이스 개요 — 주석과 선언이 붙어 있던 서식 복구')
e.block(7,'''// 아래 descDepth는 다음 절의 D3D11_TEXTURE2D_DESC 설정을 사용합니다.
HRESULT hr = mDevice->CreateTexture2D(
    &descDepth, nullptr, mDepthStencilBuffer.GetAddressOf());
if (FAILED(hr)) return false;

hr = mDevice->CreateDepthStencilView(
    mDepthStencilBuffer.Get(), nullptr, mDepthStencilView.GetAddressOf());
if (FAILED(hr)) return false;''',caption='깊이 텍스처·DSV 생성 — bool 초기화 함수 내부의 발췌')
e.block_replace(2,'UINT DeviceFlag = D3D11_CREATE_DEVICE_DEBUG;','UINT DeviceFlag = 0;\n#if defined(_DEBUG)\n        DeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;\n#endif')
e.finish('### 초기화 예제의 적용 범위\n\n이 페이지는 단계별 API 예제입니다. `g_pd3dDevice`처럼 g_ 접두사를 쓰는 예제와 클래스 멤버 예제를 그대로 한 함수에 합치지 말고, 프로젝트의 실제 멤버 이름으로 맞춰 연결합니다. 생성 함수의 HRESULT를 검사한 뒤 다음 리소스를 만들고, D3D11 Debug Layer는 Windows Graphics Tools 설치 상태에 따라 사용할 수 있습니다.')

e=Edit(2,'정점 데이터, HLSL 바이트코드, Input Layout, Vertex Buffer를 연결해 삼각형을 그립니다.','정점 색이 보간된 삼각형이 나타나며, Draw 호출 전의 바인딩을 설명할 수 있습니다.')
e.para('버텍스 버퍼에는 버텍스 셰이더가 저장','버텍스 버퍼에는 위치·색상 등 **정점 데이터**가 저장됩니다. 버텍스 셰이더 바이트코드는 별도의 셰이더 객체를 만들 때 사용합니다.','정점 버퍼와 셰이더 혼동 수정')
e.para('3. 정점 정보를 Input Assembler를 생성','3. 정점 데이터의 메모리 배치를 설명하는 Input Layout을 생성합니다.')
e.para('삼각형 정보들로 정점셰이더를 생성해보자','정점 배열을 GPU가 읽을 수 있는 Vertex Buffer에 담습니다. 정점 데이터를 담는 버퍼와 정점 셰이더는 서로 다른 객체입니다.')
e.para('(이쪽이 벡터 행렬 연산을 효율적으로','행렬의 메모리 저장 순서(row-major / column-major)는 C++ 측 저장 방식과 맞춰야 합니다. 이것만으로 벡터를 왼쪽에 곱할지 오른쪽에 곱할지가 결정되지는 않습니다.')
e.block_replace(6,'0, 0, application.GetWidth(), application.GetHeight(),','0.0f, 0.0f, static_cast<float>(application.GetWidth()), static_cast<float>(application.GetHeight()),')
e.block_replace(1,'D3DCompileFromFile(', 'HRESULT hr = D3DCompileFromFile(')
e.block_replace(1,'shaderFlags, 0, &renderer::vsBlob, &errorBlob);','shaderFlags, 0, &renderer::vsBlob, &errorBlob);\n            if (errorBlob) { OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer())); errorBlob->Release(); }\n            if (FAILED(hr)) throw std::runtime_error("Vertex shader compilation failed");')
e.block_replace(1,'shaderFlags, 0, &renderer::psBlob, &errorBlob);','shaderFlags, 0, &renderer::psBlob, &errorBlob);\n            if (errorBlob) { OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer())); errorBlob->Release(); }\n            if (FAILED(hr)) throw std::runtime_error("Pixel shader compilation failed");')
e.finish('### 코드 연결 시 확인\n\n위 컴파일 오류 처리에는 `<stdexcept>`가 필요합니다. `CreateVertexShader`, `CreatePixelShader`, `CreateInputLayout`, `CreateBuffer`의 반환값도 성공을 확인한 뒤 Draw로 넘어갑니다. `D3DX11CompileFromFile` 블록은 과거 SDK의 참고 예시이며, 현재 프로젝트의 컴파일 경로는 `D3DCompileFromFile`입니다. Draw call 수는 오브젝트 수와 항상 같지는 않습니다. 하나의 오브젝트가 여러 패스로 그려지거나 인스턴싱으로 여러 오브젝트를 한 번에 그릴 수 있습니다.')

e=Edit(3,'인덱스로 정점을 재사용하고, HLSL 상수 버퍼에 오브젝트별 값을 전달합니다. 사용 예시 뒤에 클래스로 감싸는 다음 글을 읽습니다.','같은 메시를 서로 다른 변환 값으로 그리고, CPU 구조체와 HLSL의 크기·offset을 맞출 수 있습니다.')
e.para('- 각 변수는 16바이트(float4) 경계에 정렬','- HLSL 변수는 16바이트 레지스터 경계를 넘지 않도록 묶입니다. 모든 변수가 각각 16바이트를 차지하는 것은 아닙니다. 예를 들어 float3 뒤의 float는 같은 16바이트에 들어갈 수 있습니다. [HLSL 패킹 규칙]('+PACK+')','HLSL 패킹 설명 수정')
e.para('- `D3D11_MAP_WRITE_DISCARD`: 기존 데이터 무시, 가장 빠름','- `D3D11_MAP_WRITE_DISCARD`: 이전 내용을 유지하지 않고 Dynamic 버퍼에 새 데이터를 씁니다. 속도를 무조건 보장하는 표현으로 이해하지 않습니다.')
e.para('- `D3D11_MAP_WRITE_NO_OVERWRITE`: 기존 데이터 유지','- `D3D11_MAP_WRITE_NO_OVERWRITE`: GPU가 사용하는 영역을 덮어쓰지 않겠다는 약속입니다. Dynamic 상수 버퍼에서의 사용은 D3D11.1 기능 지원을 확인해야 합니다. [D3D11_MAP]('+MAP+')')
e.para('- 권장 크기: 256 bytes 이하','- 크기는 실제 데이터와 업데이트 빈도에 맞춰 결정합니다. 256바이트 이하라는 보편적인 권장 제한은 없습니다. D3D12의 CBV 주소 정렬 256바이트와도 구분합니다.')
e.para('- 정점 재사용으로 메모리 절약 (평균 60-85%)','- 정점 재사용으로 메모리를 절약할 수 있습니다. 절약률은 정점 크기, 공유 정도, 인덱스 크기에 따라 달라집니다.')
e.para('- 모든 복잡한 메쉬에 필수적','- 정점 재사용이 많은 메시에서 유용하지만, 모든 메시가 반드시 인덱스 버퍼를 사용해야 하는 것은 아닙니다.')
e.block(0,'''계산 예시: 정점 48 bytes, 인덱스 4 bytes
사각형: 비인덱스 6×48 = 288 bytes
        인덱스 4×48 + 6×4 = 216 bytes → 25% 절약
삼각형 10,000개와 고유 정점 5,000개를 가정하면:
비인덱스 30,000×48 = 1,440,000 bytes
인덱스 5,000×48 + 30,000×4 = 360,000 bytes → 75% 절약
실제 절약률은 메시 구성에 따라 달라집니다.''','plain text',caption='인덱스 메모리까지 포함한 계산')
e.block(2,'''삼각형 목록에서 필요한 최대 로컬 인덱스 기준
0 ~ 65,535에 들어감: R16_UINT 사용 가능
65,535를 초과함: R32_UINT 또는 메시/Draw 분할 검토
R16_UINT는 R32_UINT보다 인덱스 하나의 크기가 절반입니다.
정점 버퍼 전체 크기, BaseVertexLocation, strip cut 설정은 별도 고려합니다.''','plain text')
e.block(4,'''상수 버퍼: b 슬롯으로 셰이더 상수 전달, HLSL 패킹 규칙 적용
정점·인덱스 버퍼: IA가 정점과 도형 구성 데이터를 읽음
SRV/UAV 버퍼: 셰이더 리소스로 읽기 또는 읽기·쓰기
업데이트 빈도와 크기 제한은 용도·Usage·기능 수준에 따라 다릅니다.''','plain text')
e.block(7,'''DEFAULT   : GPU 사용 중심, UpdateSubresource 또는 복사로 갱신
DYNAMIC   : CPU 쓰기와 GPU 읽기, Map/Unmap 갱신
IMMUTABLE : 생성 때 초기화, 이후 내용 변경 불가
STAGING   : CPU 접근과 복사, 그래픽 파이프라인에 직접 바인딩하지 않음
Usage 이름만으로 절대적인 성능 순위를 정하지 않습니다.''','plain text')
e.block(14,'''DYNAMIC 버퍼: Map(WRITE_DISCARD) → 데이터 복사 → Unmap
DEFAULT 버퍼: UpdateSubresource 또는 적절한 GPU 복사 경로
업데이트 크기·빈도와 GPU 사용 패턴을 측정해 선택합니다.''','plain text')
e.block_replace(1,'D3D11_BUFFER_DESC bufferDesc;','D3D11_BUFFER_DESC bufferDesc = {};')
e.block_replace(5,'VsConstData.mWorldViewProj = {...};','XMStoreFloat4x4(&VsConstData.mWorldViewProj, XMMatrixIdentity());')
e.block_replace(15,'cbData.world = GetWorldMatrix();','cbData.world = GetWorldMatrix().Transpose();')
e.block_replace(15,'cbData.view = GetViewMatrix();','cbData.view = GetViewMatrix().Transpose();')
e.block_replace(15,'cbData.projection = GetProjectionMatrix();','cbData.projection = GetProjectionMatrix().Transpose();')
e.block_replace(15,'context->Map(transformCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);','if (FAILED(context->Map(transformCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;')
e.block_replace(17,'MaterialCB cbData;','MaterialCB cbData = {};')
e.block(19,'''// HLSL 패킹 예시: C++ 구조체의 자동 레이아웃으로 해석하지 않습니다.
cbuffer PackedExample : register(b0)
{
    float4 vector; // 0..15
    float value1;  // 16..19
    float value2;  // 20..23
    float2 padding; // 24..31
}; // 32 bytes
// C++에서도 같은 offset과 전체 크기를 맞춘 뒤 전달해야 합니다.''','hlsl')
e.block(21,'''// 오브젝트마다 Transform이 다르면 해당 데이터를 바인딩해야 합니다.
for (auto& obj : objects)
{
    UpdateConstantBuffer(obj.transform);
    Draw(obj);
}
// 공통 카메라·시간 데이터는 별도 PerFrame 버퍼로 분리하면
// 프레임당 한 번 갱신할 수 있습니다. 오브젝트 Transform 갱신을
// 임의로 건너뛰면 바로 앞 오브젝트의 값이 남을 수 있습니다.''',caption='서로 다른 Transform을 공통 값으로 대체하지 않기')
e.finish('### 이 예제와 현재 DX12 구현의 차이\n\nTransform 예제는 DirectXTK SimpleMath의 행벡터 행렬을 기본 column-major HLSL에 전달하는 조합으로 전치합니다. HLSL에 `row_major`를 명시하는 방식과 중복 적용하지 않습니다. MaterialCB의 `UpdateSubresource` 예제는 DEFAULT 버퍼를 전제로 합니다. 캐시 예제는 버퍼별 마지막 데이터와 padding 초기화를 함께 관리해야 하며 전체 엔진 구현을 대체하지 않습니다. DX12에서는 같은 프레임의 Draw마다 별도 GPU 주소를 쓰는 이유를 다음 글에서 다룹니다: <mention-page url="'+NEW1+'"/>')

# Additional reviewed pages are defined in follow-up modules.
for extension in sorted(HERE.glob('edits_*.py')):
    exec(compile(extension.read_text(encoding='utf-8'),str(extension),'exec'),globals())
(HERE/'updates.json').write_text(json.dumps(PLANS,ensure_ascii=False,indent=2),encoding='utf-8',newline='\n')
print(f'Prepared {len(PLANS)} pages, {sum(len(p["content_updates"]) for p in PLANS)} targeted updates')
