삼각형 하나를 그렸으니 이번에는 사각형을 만들고, 같은 사각형을 화면 왼쪽과 오른쪽에 하나씩 배치해 보겠습니다. 여기에는 서로 다른 두 가지 문제가 있습니다. **사각형 안에서 공유하는 꼭짓점은 어떻게 재사용할까? 같은 사각형의 위치만 바꾸려면 무엇을 갱신해야 할까?**

첫 번째 문제에는 인덱스 버퍼, 두 번째 문제에는 상수 버퍼를 사용합니다. 두 버퍼 모두 `ID3D11Buffer`로 만들지만 담는 정보와 연결되는 곳이 다릅니다. 실습이 끝나면 같은 정점·인덱스 버퍼를 유지한 채 상수 값만 바꾸어 두 사각형을 그릴 수 있습니다.

{{M0}}

## 1. 사각형은 삼각형 두 개입니다

GPU의 Triangle List로 사각형을 그리려면 대각선을 기준으로 두 삼각형을 만듭니다. 사각형의 모서리는 네 개이지만, 두 삼각형이 각각 정점 세 개를 요구하므로 단순히 배열을 이어 붙이면 여섯 항목이 필요합니다.

{{M1}}

두 삼각형이 공유하는 대각선의 양 끝점에는 같은 위치와 색이 두 번 들어갑니다. 정점에 UV·법선까지 추가하면 이 중복도 더 커집니다.

{{M2}}

인덱스 버퍼를 쓰면 정점은 네 개만 저장하고, 삼각형을 구성할 때는 정점의 **번호**를 읽습니다. 인덱스가 정점 데이터를 압축해서 안에 넣는 것은 아닙니다. 정점 버퍼와 별개의 작은 번호 배열을 추가하는 것입니다.

{{M3}}

이번 코드에서는 정점을 왼쪽 위부터 시계 방향으로 0·1·2·3이라고 부르겠습니다. 그러면 첫 삼각형은 `(0,1,2)`, 두 번째는 `(0,2,3)`입니다. 0번과 2번이 공유되는 것을 확인해 보세요.

```mermaid
flowchart LR
    I[인덱스 배열: 0 1 2 / 0 2 3] --> A[첫 삼각형: 정점 0 1 2]
    I --> B[둘째 삼각형: 정점 0 2 3]
    A --> V[정점 버퍼: 네 모서리의 위치와 색]
    B --> V
```

정점 하나가 이전 글처럼 28바이트라면 중복 저장은 `6×28=168`바이트입니다. 32비트 인덱스를 사용하면 `4×28 + 6×4=136`바이트입니다. 인덱스도 메모리를 사용하므로 절약률이 항상 절반인 것은 아닙니다. 정점이 크고 공유가 많을수록 유리하며, GPU의 정점 처리 결과 재사용에도 도움이 될 수 있습니다.

다만 위치가 같아도 UV나 법선이 다른 정점은 그대로 공유할 수 없습니다. 예를 들어 상자의 날카로운 모서리에서 면마다 법선을 다르게 쓰려면, 같은 위치에 서로 다른 정점 항목이 필요합니다.

## 2. 네 정점과 여섯 인덱스를 준비합니다

이전 글의 `Vertex { XMFLOAT3 position; XMFLOAT4 color; }`와 DX11 Device·Context를 이어 사용합니다. 정점·인덱스는 초기화 때 한 번 생성하고 렌더링하는 동안 유지합니다.

```c++
const Vertex vertices[] = {
    {{-0.3f,  0.3f, 0}, {1, 1, 1, 1}}, // 0: 왼쪽 위
    {{ 0.3f,  0.3f, 0}, {1, 1, 1, 1}}, // 1: 오른쪽 위
    {{ 0.3f, -0.3f, 0}, {1, 1, 1, 1}}, // 2: 오른쪽 아래
    {{-0.3f, -0.3f, 0}, {1, 1, 1, 1}}, // 3: 왼쪽 아래
};
const UINT indices[] = {0, 1, 2, 0, 2, 3};
```

정점 버퍼 생성은 이전과 같고 `ByteWidth=sizeof(vertices)`만 네 정점에 맞춰집니다. 인덱스 버퍼도 같은 `CreateBuffer`를 사용합니다. 다른 점은 초기 데이터가 위치·색 대신 정수 배열이고, 용도가 `D3D11_BIND_INDEX_BUFFER`라는 것입니다.

{{M4}}

```c++
Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
D3D11_BUFFER_DESC ibDesc{};
ibDesc.ByteWidth = sizeof(indices);
ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

D3D11_SUBRESOURCE_DATA ibData{};
ibData.pSysMem = indices;
Check(device->CreateBuffer(&ibDesc, &ibData,
                           indexBuffer.GetAddressOf()));

// Draw 직전. 정점 버퍼·Input Layout·셰이더는 이전 글처럼 선택합니다.
context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
context->DrawIndexed(6, 0, 0);
```

`R32_UINT`는 인덱스 하나를 32비트 부호 없는 정수로 읽겠다는 뜻입니다. 배열을 `uint16_t`로 만들었다면 `R16_UINT`로 맞춰야 합니다. 여기의 0은 버퍼 내부의 시작 바이트 위치입니다.

`DrawIndexed(6,0,0)`의 6은 정점 수 네 개가 아니라 **읽을 인덱스 수 여섯 개**입니다. 두 번째 값은 인덱스 배열에서 시작할 위치, 세 번째 값은 읽은 번호에 더할 정점 기준값입니다. 지금은 모두 처음부터 읽으므로 0을 사용합니다. 마지막 세 인덱스를 빼고 `DrawIndexed(3,0,0)`을 호출하면 사각형의 절반만 보이는지 확인해 보세요.

## 3. 위치만 바꾸는데 정점 버퍼를 다시 만들어야 할까요?

왼쪽 사각형과 오른쪽 사각형은 모양이 같습니다. 매번 네 정점의 x를 CPU에서 바꾸고 GPU 버퍼를 다시 만드는 대신, 정점 셰이더에 “이번에는 x를 -0.5만큼 옮겨라”라는 값 하나를 전달할 수 있습니다. 이처럼 셰이더 실행에 사용할 매개변수를 담는 것이 상수 버퍼입니다.

{{M5}}

이름의 ‘상수’는 프로그램 실행 중 영원히 바뀌지 않는다는 뜻이 아닙니다. Draw 전에 새로운 값을 넣을 수 있습니다. 한 번의 Draw에서 여러 정점이 공통으로 읽는 위치·크기·행렬·재질 값 등을 생각하면 됩니다.

## 4. CPU와 HLSL의 16바이트를 맞춥니다

처음부터 행렬 세 개를 넣기보다, 화면상 이동과 크기만 넣어 데이터가 전달되는 모습을 확인하겠습니다. 이번 예제는 원리 확인용 2D 변환입니다. 실제 카메라 변환은 뒤에서 World·View·Projection 행렬로 확장합니다.

```c++
// C++
struct DrawConstants
{
    float offset[2]; // 바이트 0~7: x, y 이동
    float scale;     // 바이트 8~11: 크기
    float padding;   // 바이트 12~15: 사용하지 않음
};
static_assert(sizeof(DrawConstants) == 16);
```

HLSL에는 같은 순서로 선언합니다.

```c++
// HLSL: 정점 셰이더에 추가합니다.
cbuffer DrawData : register(b0)
{
    float2 offset;
    float scale;
    float padding;
};
```

HLSL 상수 버퍼는 16바이트 레지스터 경계를 기준으로 값을 묶습니다. 모든 float를 각각 16바이트로 만드는 것은 아닙니다. 이 경우 `float2 + float + float`가 정확히 16바이트 하나에 들어갑니다. C++의 메모리 순서와 HLSL이 읽는 순서가 같아야 scale 자리에서 padding이나 다른 값을 읽지 않습니다.

DX11의 상수 버퍼 생성 크기는 16바이트 배수여야 합니다. 뒤에 나오는 **DX12 CBV의 256바이트 정렬은 별도의 규칙**입니다. 이 예제를 DX11에서 256바이트로 만들어야 하는 것은 아닙니다. 행렬·배열을 추가할 때는 단순히 전체 크기만 맞추지 말고 각 멤버의 offset도 맞춥니다.

## 5. 매 Draw 전에 갱신할 버퍼를 만듭니다

```c++
Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
D3D11_BUFFER_DESC cbDesc{};
cbDesc.ByteWidth = sizeof(DrawConstants);
cbDesc.Usage = D3D11_USAGE_DYNAMIC;
cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
Check(device->CreateBuffer(&cbDesc, nullptr,
                           constantBuffer.GetAddressOf()));
```

정점·인덱스는 고정되어 있었지만 이 값은 매번 바꿀 예정이므로 `DYNAMIC`으로 만들었습니다. CPU가 쓸 것이므로 `CPU_ACCESS_WRITE`도 함께 지정합니다. 상수 버퍼용 BindFlags는 다른 용도의 BindFlags와 조합하지 않습니다.

초기 데이터를 `nullptr`로 두었으므로 첫 Draw 전에 반드시 값을 써야 합니다. 생성만 한 미초기화 버퍼를 셰이더에서 읽게 하면 기대하는 이동·크기 값이 들어 있다고 보장할 수 없습니다.

## 6. 정점 셰이더가 공통 값을 사용합니다

이전 글의 `VSInput`, `VSOutput`은 유지하고 main만 바꿉니다. 위의 `cbuffer` 선언도 같은 파일에 있어야 합니다.

```c++
// HLSL
VSOutput main(VSInput input)
{
    VSOutput output;
    float2 xy = input.position.xy * scale + offset;
    output.position = float4(xy, input.position.z, 1.0f);
    output.color = input.color;
    return output;
}
```

`scale=1`, `offset=(-0.5,0)`을 주면 네 정점 모두 x가 0.5만큼 작아집니다. 정점 간 간격은 그대로이므로 모양은 유지되고 위치만 왼쪽으로 움직입니다. `scale=0.5`라면 먼저 크기를 절반으로 줄인 뒤 offset만큼 이동합니다. CPU는 이 식의 결과 정점 네 개를 계산하지 않고, 식에서 공통으로 읽을 숫자만 전달합니다.

## 7. 갱신 → 바인딩 → Draw를 두 번 반복합니다

```c++
#include <cstring>

// Clear 뒤, VB·IB·셰이더·RTV·Viewport를 설정한 렌더링 함수 내부
auto drawAt = [&](float x)
{
    DrawConstants data{{x, 0.0f}, 1.0f, 0.0f};
    D3D11_MAPPED_SUBRESOURCE mapped{};
    Check(context->Map(constantBuffer.Get(), 0,
                       D3D11_MAP_WRITE_DISCARD, 0, &mapped));
    std::memcpy(mapped.pData, &data, sizeof(data));
    context->Unmap(constantBuffer.Get(), 0);

    ID3D11Buffer* cb = constantBuffer.Get();
    context->VSSetConstantBuffers(0, 1, &cb); // VS의 b0
    context->DrawIndexed(6, 0, 0);
};

drawAt(-0.5f);
drawAt( 0.5f);
// 두 Draw가 끝난 뒤 Present
```

`Map`은 CPU가 쓸 메모리를 얻고, `memcpy`는 준비한 16바이트를 그곳에 복사하며, `Unmap`은 쓰기를 마칩니다. `WRITE_DISCARD`는 이전 내용을 보존하지 않겠다는 설정입니다. 그래서 필요한 값 전체를 다시 씁니다. padding까지 0으로 초기화한 것도 메모리에 미정 값이 남지 않게 하기 위해서입니다.

`VSSetConstantBuffers(0,...)`는 정점 셰이더의 `register(b0)`와 연결됩니다. 픽셀 셰이더에서 같은 버퍼가 필요하다면 `PSSetConstantBuffers`에도 연결해야 합니다. VS에 바인딩했다고 모든 셰이더 단계에 자동으로 들어가는 것은 아닙니다.

이 DX11 예제의 반복 갱신은 런타임의 Dynamic 버퍼 관리와 WRITE_DISCARD 사용법을 따릅니다. DX12에서는 업로드 메모리 한 주소를 두 Draw 사이에 CPU로 덮어쓰는 것만으로 같은 동작을 보장할 수 없습니다. GPU가 나중에 읽기 때문에 Draw마다 별도 주소를 배정하는 이유를 DX12 후속 강의에서 다룹니다.

## 화면에서 확인할 연결

실행하면 같은 모양의 흰 사각형이 좌우에 보입니다. 두 번째 `drawAt`의 x를 0으로 바꾸면 오른쪽 사각형만 가운데로 옮겨집니다. 정점 버퍼와 인덱스 버퍼는 바뀌지 않았습니다. 두 번째 Draw가 읽는 상수만 바뀐 결과입니다.

두 사각형이 같은 위치에 겹친다면 상수 갱신이 각 Draw 앞에 있는지 살펴보세요. 예를 들어 왼쪽 값과 오른쪽 값을 연속으로 써 놓고 그 뒤에 Draw 두 번을 호출하면, 두 Draw 모두 마지막으로 설정한 값을 사용하게 됩니다. 코드의 줄 순서가 어떤 데이터와 Draw를 묶는지 직접 표시해 보면 원인을 찾기 쉽습니다.

다음에는 이 생성·갱신·바인딩 코드를 클래스로 나눕니다. 클래스를 만드는 목적은 함수 수를 늘리는 것이 아니라, 버퍼를 소유하는 곳과 매번 사용하는 곳을 분리하여 같은 사각형을 여러 오브젝트에서 안전하게 재사용하는 것입니다.

[Microsoft: HLSL 패킹 규칙](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules) · [Microsoft: D3D11_MAP](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_map)
