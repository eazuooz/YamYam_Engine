지금까지 삼각형의 색을 정점에 넣어 그렸다면, 이번에는 사각형 위에 그림 파일을 표시해 보겠습니다. 정점 네 개에 색 네 개만 넣는 방법으로는 고양이 그림이나 타일 무늬를 표현하기 어렵습니다. 표면의 많은 지점에 서로 다른 색을 주려면, 그 색을 모아 둔 이미지와 “이미지의 어디를 읽을지”를 알려 주는 좌표가 필요합니다.

이때 색 데이터를 담는 것이 **Texture**, 이미지 위의 위치를 가리키는 것이 **UV**입니다. 먼저 한 장의 PNG가 픽셀 셰이더까지 도착하는 과정을 만들고, 마지막에 사각형 위에 그려 보겠습니다.

## 1. 파일을 읽었는데 왜 아직 GPU에서 쓸 수 없을까요?

PNG 파일에는 압축된 이미지와 파일 형식을 설명하는 정보가 들어 있습니다. GPU의 픽셀 셰이더가 그 PNG 파일을 직접 열어서 해석하는 것은 아닙니다. CPU가 파일을 읽고 해독한 뒤, GPU가 읽을 수 있는 텍스처 리소스를 만들어야 합니다.

{{PATH_IMAGE}}

그림의 첫 번째 상자가 DirectXTex의 `ScratchImage`입니다. 이름에 Image가 있지만 GPU 텍스처는 아닙니다. **CPU 메모리에 있는 픽셀 데이터와 이미지 정보**를 관리합니다. 두 번째 상자의 `ID3D11Texture2D`를 만들어야 그 데이터를 DX11 리소스로 사용할 수 있습니다.

예를 들어 압축을 풀어 얻은 이미지가 256×256 크기의 RGBA8이라면 한 픽셀은 R·G·B·A 네 바이트입니다. 기본 이미지의 픽셀 데이터는 256×256×4=262,144바이트입니다. PNG 파일의 디스크 크기가 이 값보다 작을 수 있는 것은 파일이 압축되어 있기 때문입니다. 파일 크기, CPU에서 해독한 이미지 크기, 실제 GPU 할당 크기는 같은 개념이 아닙니다.

여기서 **texel**이라는 말도 함께 기억해 두겠습니다. 텍스처 이미지의 원소를 texel이라고 하고, 화면의 원소를 pixel이라고 부릅니다. 그림을 크게 확대하면 texel 하나가 화면의 여러 pixel에 영향을 줄 수 있습니다.

## 2. Texture와 SRV를 따로 만드는 이유

GPU에 이미지 데이터를 저장했다고 해서 셰이더가 어떤 방식으로 읽을지까지 정해진 것은 아닙니다. 동일한 리소스라도 포맷을 어떻게 해석할지, 어떤 mip 구간을 읽을지 등 접근 방식이 필요합니다. 이 읽기용 설명이 **Shader Resource View, SRV**입니다.

{{ORIGINAL_RESOURCE_IMAGE}}

위 그림은 리소스와 뷰를 구별해서 보세요. 리소스는 실제 데이터를 보관하고, 뷰는 파이프라인에서 그 데이터에 접근하는 방법을 정합니다. 이 글에서는 파일 이미지를 픽셀 셰이더에서 읽으므로 SRV를 사용합니다.

나중에 카메라가 텍스처에 그림을 **쓰게** 만들면 RTV(Render Target View)를 사용합니다. 그 결과를 다시 셰이더에서 **읽을 때**는 SRV를 사용합니다. 두 뷰가 텍스처 두 장을 뜻하는 것은 아닙니다. 생성 용도와 포맷이 허용한다면 하나의 리소스에 서로 다른 뷰를 만들 수 있습니다. Scene/Game 뷰가 이 원리를 사용하지만, 지금은 읽기 전용 이미지 한 장부터 연결하겠습니다.

## 3. DirectXTex로 PNG를 해독합니다

파일 형식을 직접 해석하는 코드는 작성하지 않고 DirectXTex를 사용하겠습니다. 이 라이브러리의 `LoadFromWICFile`은 Windows Imaging Component를 통해 PNG·JPEG 같은 이미지를 읽어 `ScratchImage`에 담습니다.

아래 예제는 DX11의 로딩 과정을 보기 위해 정리한 실습 코드입니다. DirectXTex의 헤더와 라이브러리가 프로젝트에 연결되어 있고, DX11 Device가 이미 생성되어 있다고 가정합니다. WIC를 사용하는 스레드에는 COM 초기화가 필요하므로 프로그램 초기화 단계에서 이를 준비합니다.

```c++
// WIC를 사용할 스레드의 시작 지점에서 실행합니다.
HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
if (FAILED(comResult))
    return comResult;

// 이 스레드에서 이미지 로드 등의 작업을 수행합니다.
// 스레드 종료 시, 위 초기화가 성공한 횟수에 맞춰 CoUninitialize()를 호출합니다.
```

이미 프로그램의 다른 초기화 코드가 같은 스레드의 COM을 관리한다면 그 수명에 맞춰 연결합니다. 텍스처 파일을 하나 읽을 때마다 별도의 스레드 초기화 정책을 만들 필요는 없습니다.

먼저 로딩의 첫 절반만 보겠습니다.

```c++
#include <DirectXTex.h>

DirectX::ScratchImage image;
HRESULT hr = DirectX::LoadFromWICFile(
    L"Resources/Textures/sample.png",
    DirectX::WIC_FLAGS_NONE,
    nullptr,
    image);
if (FAILED(hr))
    return hr;
```

마지막 인자인 `image`가 결과를 받습니다. 앞의 `nullptr`는 별도로 받을 metadata 출력 인자를 생략한다는 뜻입니다. 로드된 정보는 이후 `image.GetMetadata()`로도 얻을 수 있습니다.

이 코드가 성공한 직후에는 파일을 읽는 일까지만 끝났습니다. 아직 셰이더에 바인딩할 SRV는 없습니다. 그러므로 여기서 화면이 바뀌지 않는 것은 정상입니다. 실패했다면 GPU를 먼저 의심하기보다 파일 경로, 실행 파일의 작업 디렉터리, WIC/COM 초기화부터 확인합니다.

## 4. 해독한 이미지로 GPU 텍스처와 SRV를 만듭니다

이제 `ScratchImage`를 입력으로 DirectXTex의 GPU 생성 도우미를 호출합니다.

```c++
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

hr = DirectX::CreateShaderResourceView(
    device,
    image.GetImages(),
    image.GetImageCount(),
    image.GetMetadata(),
    srv.GetAddressOf());
if (FAILED(hr))
    return hr;
```

`GetImages()`는 이미지 데이터의 시작 위치를, `GetImageCount()`는 이미지 개수를 전달합니다. 텍스처 하나에 mip 여러 단계가 들어갈 수도 있기 때문에 이미지 한 개의 포인터만 넘기는 형태가 아니라 배열과 개수를 함께 전달합니다. `GetMetadata()`에는 폭·높이·포맷·mip 수 같은 생성 정보가 있습니다.

여기서 함수 이름을 주의해서 보세요. **`DirectX::CreateShaderResourceView`는 DirectXTex의 도우미**이며 텍스처 리소스와 SRV를 함께 만듭니다. 이름이 같은 **`device->CreateShaderResourceView`는 이미 생성한 리소스에 뷰만 만듭니다.** 두 함수를 같은 역할이라고 생각하면 텍스처가 어느 시점에 생겼는지 헷갈리게 됩니다.

이번에는 도우미가 만든 GPU 텍스처도 가져와 보겠습니다.

```c++
Microsoft::WRL::ComPtr<ID3D11Resource> resource;
srv->GetResource(resource.GetAddressOf());

Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
hr = resource.As(&texture);
if (FAILED(hr))
    return hr;

D3D11_TEXTURE2D_DESC desc = {};
texture->GetDesc(&desc);
```

SRV는 자신이 가리키는 리소스를 알고 있습니다. `GetResource`로 그 리소스의 인터페이스를 받고, `As`로 Texture2D 인터페이스를 요청합니다. 단순 C++ 캐스팅과 달리 COM의 QueryInterface를 사용하므로 실제로 지원하는 인터페이스인지 확인할 수 있습니다.

`GetDesc`까지 호출하면 실제 생성된 Width·Height·Format을 확인할 수 있습니다. 여기서 기대한 이미지 크기가 나온다면 CPU 디코딩과 GPU 생성이 연결된 것입니다. 아직 화면이 검다면 다음 단계인 바인딩과 UV를 살펴볼 차례입니다.

## 5. 매번 파일을 읽지 않도록 Texture 클래스로 묶습니다

게임은 같은 이미지를 여러 프레임 동안 사용합니다. 매 Draw마다 PNG를 읽으면 디스크 접근과 이미지 해독, GPU 리소스 생성이 반복됩니다. 따라서 `Load`에서 한 번 생성하고, `Bind`에서는 이미 만든 SRV만 선택하도록 나누겠습니다.

```c++
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXTex.h>

class Texture2DExample
{
public:
    HRESULT LoadPNG(ID3D11Device* device, const wchar_t* path)
    {
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(
            path, DirectX::WIC_FLAGS_NONE, nullptr, image);
        if (FAILED(hr)) return hr;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> newSRV;
        hr = DirectX::CreateShaderResourceView(
            device, image.GetImages(), image.GetImageCount(),
            image.GetMetadata(), newSRV.GetAddressOf());
        if (FAILED(hr)) return hr;

        mSRV = newSRV;
        return S_OK;
    }

    void BindPS(ID3D11DeviceContext* context, UINT slot) const
    {
        ID3D11ShaderResourceView* view = mSRV.Get();
        context->PSSetShaderResources(slot, 1, &view);
    }

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSRV;
};
```

`LoadPNG`의 `ScratchImage`는 지역 변수입니다. GPU 리소스 생성에 필요한 초기 데이터를 전달한 뒤 함수가 끝나면 CPU 쪽 이미지 메모리를 해제합니다. 여기서는 나중에 CPU로 이미지를 다시 편집하지 않으므로 계속 보관할 이유가 없습니다. CPU 이미지 처리도 할 계획이라면 별도로 보관할 수 있습니다.

멤버에는 SRV만 남겼지만 그 SRV가 GPU 리소스를 참조하므로 텍스처를 계속 사용할 수 있습니다. Texture2D의 크기 조회나 복사 기능이 필요해지면 앞에서 본 `GetResource`와 `As`를 이용해 Texture2D 멤버도 보관하면 됩니다.

ComPtr는 자신이 소유한 COM 참조를 해제합니다. 그래서 소멸자에서 `mSRV.Get()->Release()`를 다시 호출하지 않습니다. 그렇게 하면 ComPtr가 관리하는 참조를 중복 해제할 수 있습니다. 반대로 `BindPS` 안의 `view`는 `.Get()`으로 잠깐 빌린 raw pointer이므로 여기서도 직접 Release하지 않습니다.

## 6. t0, s0, UV를 한 지점으로 연결합니다

이제 GPU 텍스처가 준비되었습니다. 하지만 셰이더가 어떤 텍스처를 읽어야 할지 알려 주어야 합니다. C++에서 픽셀 셰이더의 0번 SRV 슬롯에 바인딩하면 HLSL의 `register(t0)`와 연결됩니다.

```c++
// 초기화 시 한 번 로드한 texture를 매 Draw 전에 선택합니다.
texture.BindPS(context, 0); // Pixel Shader의 t0
```

이제 **어느 위치를 어떻게 읽을지**가 남았습니다. UV는 읽을 위치이고, Sampler는 그 위치 주변 texel을 어떻게 선택하고 섞을지 정합니다.

{{UV_IMAGE}}

위 그림에서 왼쪽 위 정점에 `(0,0)`, 오른쪽 위에 `(1,0)`, 왼쪽 아래에 `(0,1)`, 오른쪽 아래에 `(1,1)`을 줍니다. 정점 셰이더가 이 UV를 출력하면 Rasterizer가 삼각형 내부의 UV를 보간해 픽셀 셰이더로 넘깁니다. 가운데에 UV `(0.5,0.5)`를 가진 정점을 따로 추가할 필요는 없습니다.

화면 위치와 UV도 구분해야 합니다. 사각형을 오른쪽으로 이동하면 화면 위치는 변하지만, 각 모서리에 붙은 UV를 유지하면 고양이 그림은 사각형 위에 그대로 붙어 이동합니다. 반대로 정점의 위치를 유지하고 UV만 움직이면 사각형은 그대로 있고 그 위의 무늬가 움직여 보입니다.

처음 실습에서는 Point 필터와 Clamp 주소 모드를 사용하겠습니다. Point는 가장 가까운 texel을 고르므로 확대했을 때 원본 texel의 경계를 보기 쉽습니다. Clamp는 UV가 이미지 범위를 벗어나면 가장자리 값을 사용합니다.

```c++
D3D11_SAMPLER_DESC samplerDesc = {};
samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
samplerDesc.MaxAnisotropy = 1;
samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
samplerDesc.MinLOD = 0.0f;
samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
HRESULT hr = device->CreateSamplerState(
    &samplerDesc, sampler.GetAddressOf());
if (FAILED(hr)) return hr;

// Draw 전에 바인딩합니다.
ID3D11SamplerState* state = sampler.Get();
context->PSSetSamplers(0, 1, &state); // Pixel Shader의 s0
```

SRV와 Sampler를 둘 다 0번에 바인딩해도 서로 덮어쓰지 않습니다. **t 슬롯과 s 슬롯은 별도 공간**이기 때문입니다. t0은 이미지, s0은 그 이미지를 읽는 규칙을 받습니다.

## 7. 픽셀 셰이더가 색을 읽는 순간

아래 HLSL은 클립 위치와 UV를 전달받아 이미지를 표시하는 가장 작은 픽셀 셰이더입니다. Vertex Shader의 출력에도 같은 `TEXCOORD` 의미의 UV가 있어야 합니다.

```c++
// HLSL: TexturePS.hlsl
Texture2D imageTexture : register(t0);
SamplerState pointClamp : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_Target
{
    return imageTexture.Sample(pointClamp, input.uv);
}
```

한 픽셀에서 이 함수가 실행된다고 생각해 보세요. `imageTexture`는 C++에서 t0에 연결한 SRV를 통해 텍스처를 읽습니다. `input.uv`는 그 픽셀에서 보간된 이미지 좌표입니다. `pointClamp`는 s0에 넣은 규칙으로 texel을 선택합니다. `Sample`이 반환한 RGBA를 그대로 출력하므로 사각형 위에 원본 그림이 나타납니다.

여기까지의 Draw 순서는 **정점·인덱스와 셰이더 설정 → t0에 SRV → s0에 Sampler → Draw**입니다. UV를 새로 정점 구조체에 추가했다면 Input Layout에도 `TEXCOORD`와 `DXGI_FORMAT_R32G32_FLOAT`를 넣어야 합니다. C++ 메모리에 UV를 적는 것만으로 GPU가 그 위치를 알아내지는 못합니다.

## 8. 값 하나씩 바꾸며 연결을 확인해 봅시다

먼저 PS의 반환값을 `float4(input.uv, 0, 1)`로 바꿔 보세요. 이미지 대신 UV가 색으로 보입니다. 왼쪽에서 오른쪽으로 R이, 위에서 아래로 G가 증가하는 그라데이션이라면 UV 전달은 연결된 것입니다. 이 상태는 정상인데 `Sample`을 사용할 때만 검게 보인다면 파일 로드, t0 SRV, s0 Sampler 쪽을 좁혀서 볼 수 있습니다.

이번에는 UV를 전부 `(0.5,0.5)`로 바꿔 보세요. 사각형 전체가 이미지 가운데의 같은 색을 읽게 됩니다. “텍스처를 로드하면 자동으로 사각형에 맞게 펴진다”는 생각과 실제 동작이 어떻게 다른지 확인할 수 있습니다.

마지막으로 `AddressU`를 WRAP으로 바꾸고 가로 UV를 0에서 2까지 주면 무늬가 두 번 반복됩니다. 같은 UV 범위에서 CLAMP를 사용하면 1을 넘는 부분은 가장자리 색이 늘어납니다. 텍스처 데이터는 같고 샘플링 규칙만 바뀐 결과입니다. 다음 Sampler 강의에서 확대·축소 필터와 이 주소 모드를 더 자세히 다룹니다.

## 9. mip과 렌더 타깃으로 넓혀 가기

멀리 있는 작은 사각형에 큰 이미지를 그리면 화면 pixel 하나가 많은 texel을 대표하게 됩니다. 이때 더 작은 크기로 미리 만든 이미지들이 **mip chain**입니다. DirectXTex로 mip을 생성하려면 `GenerateMipMaps`를 명시적으로 호출하거나 mip이 포함된 DDS를 읽습니다. 위 PNG 로드 예제가 모든 mip을 자동 생성한다고 생각하지 않습니다.

지금 만든 텍스처는 파일의 픽셀에서 출발했습니다. 다음에는 같은 위치에 GPU가 렌더링한 화면을 담을 수 있습니다. 쓰는 동안에는 RTV로 사용하고, 렌더링이 끝난 결과를 읽을 때에는 SRV로 사용하면 카메라 화면을 다른 사각형이나 에디터 패널에 표시할 수 있습니다. 이것이 오프스크린 렌더링의 출발점입니다.

DX12에서는 여기에 업로드용 리소스와 상태 전환, descriptor 수명 관리가 추가됩니다. 먼저 이 글의 CPU 이미지·GPU 리소스·읽기 뷰·샘플러 구분을 이해하면, <mention-page url="https://app.notion.com/p/3dc0b1ffa61e814e9e8bd403ca145ebe"/>에서 늘어난 코드가 각각 무엇을 책임지는지 따라갈 수 있습니다.

[DirectXTex: CreateShaderResourceView](https://github.com/microsoft/DirectXTex/wiki/CreateShaderResourceView) · [DirectXTex 공식 안내](https://github.com/microsoft/DirectXTex/wiki/DirectXTex)
