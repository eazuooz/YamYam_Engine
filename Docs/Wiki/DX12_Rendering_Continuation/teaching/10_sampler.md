{{M0}}

텍스처 파일을 사각형에 표시했는데 확대하면 네모가 보이거나, 바닥을 비스듬히 보면 무늬가 흐려집니다. 같은 이미지인데 화면에서 보이는 모습은 왜 달라질까요? 이번에는 이미지의 **어디를 읽는지 정하는 UV**와 **그 위치 주변의 색을 어떻게 고르는지 정하는 Sampler**를 나누어 살펴보겠습니다.

작은 바둑판 이미지를 준비해 사각형을 크게, 작게, 비스듬하게 보이도록 비교하면 차이가 잘 보입니다. 이미지의 원소인 texel과 화면의 원소인 pixel은 항상 1:1로 대응하지 않습니다. 화면 pixel 하나에 어떤 색을 줄지 결정하는 과정이 샘플링입니다.

## 1. UV는 이미지 안의 위치를 나타냅니다

일반적인 Direct3D 2D 텍스처에서 왼쪽 위는 `(0,0)`, 오른쪽 아래는 `(1,1)`입니다. U는 오른쪽으로, V는 아래쪽으로 증가합니다. 256×256 이미지든 1024×1024 이미지든 가운데를 읽는 UV는 `(0.5,0.5)`입니다.

사각형의 네 모서리에 이 UV를 넣고 VS에서 전달하면, 래스터라이저가 내부의 UV를 보간합니다. 사각형 가운데의 모든 픽셀을 위해 C++ 정점을 따로 만들지 않아도 됩니다. 정점의 화면 위치를 바꾸면 사각형이 움직이고, 위치는 둔 채 UV만 바꾸면 표면 위의 무늬가 움직입니다.

좌표 0과 1은 이미지의 경계입니다. 가로 W개 texel에서 i번 texel의 중심은 `(i+0.5)/W`에 있습니다. 경계 근처를 Linear로 읽으면 주변 texel과 주소 모드의 영향을 함께 받는다는 점을 뒤에서 다시 보겠습니다.

## 2. Point: 가장 가까운 texel 하나를 선택합니다

{{M1}}

작은 이미지를 크게 표시하면 texel 하나가 화면의 여러 pixel에 해당합니다. Point는 주변 색을 섞지 않고 가장 가까운 texel의 값을 고르므로, 확대된 네모가 그대로 보입니다. 픽셀 아트의 모양을 유지하려면 이 결과가 의도에 맞을 수 있습니다. 따라서 Point를 무조건 ‘나쁜 품질’이라고 부르는 것은 적절하지 않습니다.

위 이미지에서는 경계가 선명하게 끊기는 부분을 보세요. 선택 위치가 texel 경계를 넘어가면 색이 바로 바뀝니다. 필터를 바꾸어도 원본 텍스처 파일 자체가 바뀌지는 않습니다.

## 3. Linear: 주변 색을 거리 비율로 섞습니다

{{M2}}

2D의 Linear 필터는 선택한 mip 안에서 인접한 네 texel을 U·V 방향으로 보간합니다. 우선 한 줄만 생각해 보겠습니다. 왼쪽이 빨강 `(1,0,0)`, 오른쪽이 파랑 `(0,0,1)`이고 위치가 왼쪽에서 오른쪽으로 1/4 지점이면, 왼쪽 색의 비중은 3/4, 오른쪽은 1/4입니다. 결과는 `(0.75,0,0.25)`입니다.

2D에서는 위쪽 두 색과 아래쪽 두 색을 각각 U 방향으로 섞고, 그 두 결과를 V 방향으로 섞습니다. 이 과정을 bilinear라고 합니다. 확대된 경계가 부드러워지는 이유는 새 이미지 정보를 만들어서가 아니라, 주변 색의 중간값을 채우기 때문입니다.

멀리 있는 작은 사각형은 화면 pixel 하나에 많은 texel이 대응합니다. 원본 이미지의 네 texel만 읽는 것으로는 그 넓은 영역을 안정적으로 대표하기 어렵습니다. 그래서 크기를 단계적으로 줄여 둔 **mip chain**을 함께 사용합니다. 인접한 두 mip에서 bilinear한 결과를 다시 섞는 것이 이 문맥의 trilinear입니다.

## 4. Anisotropic: 비스듬한 표면의 길어진 영역을 고려합니다

{{M3}}

바닥을 비스듬히 보면 화면 pixel이 텍스처 위에서 차지하는 영역이 길게 늘어날 수 있습니다. U와 V가 같은 비율로 줄어드는 상황이 아닙니다. 한 축에 맞춰 작은 mip만 선택하면 다른 방향의 무늬까지 지나치게 흐려질 수 있습니다. Anisotropic 필터는 이처럼 늘어난 영역을 고려하여 샘플링합니다.

<callout icon="💡" color="gray_bg">
	아래는 가로·세로 해상도를 각각 줄이는 ripmap의 개념 그림입니다. 일반적인 D3D11 Anisotropic Sampler를 사용하려고 이 모든 이미지를 별도로 만들 필요는 없습니다. 보통의 mip chain을 사용하면서 하드웨어가 비등방성 필터링을 수행합니다.
{{M4}}
</callout>

아래 비교에서는 같은 텍스처, 같은 카메라 각도, 같은 화면 크기인지 확인하고 먼 바닥의 무늬를 보세요. 정면의 픽셀 아트 확대와 비스듬한 바닥 축소는 서로 다른 문제이므로 하나의 필터 순위로 묶지 않습니다.

{{M5}}
{{M6}}

여러 필터를 같은 조건에서 비교하면 어떤 정보가 유지되고 어디가 흐려지는지 구분하기 쉽습니다.

{{M7}}

`MaxAnisotropy`는 허용할 비등방성의 상한입니다. 8을 넣었다고 모든 픽셀에서 정확히 여덟 번 Sample한다는 뜻은 아닙니다. 1~16 범위에서 장면의 화질과 실제 GPU 시간을 함께 비교합니다.

## 5. C++에서 규칙을 만들고 s0에 연결합니다

이번에는 Linear+Wrap을 만들겠습니다. 아래 코드는 유효한 DX11 `device`와 `context` 포인터가 있고 `<d3d11.h>`, `<wrl/client.h>`가 포함되어 있다고 가정합니다. `sampler`는 렌더링하는 동안 유지합니다.

```c++
D3D11_SAMPLER_DESC desc{};
desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
desc.MaxAnisotropy = 1;
desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
desc.MinLOD = 0.0f;
desc.MaxLOD = D3D11_FLOAT32_MAX;

Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
HRESULT hr = device->CreateSamplerState(&desc, sampler.GetAddressOf());
if (FAILED(hr)) return hr;

// Draw 직전
ID3D11SamplerState* state = sampler.Get();
context->PSSetSamplers(0, 1, &state);
```

Filter의 MIN은 축소, MAG는 확대, MIP는 mip 사이 선택 방식을 나타냅니다. `MIN_MAG_MIP_LINEAR`는 세 선택 모두 Linear를 사용합니다. `MaxLOD`를 충분히 크게 두어 존재하는 mip들을 사용할 수 있게 했습니다. Sampler를 만들었다고 mip 이미지가 생성되는 것은 아닙니다. 텍스처를 로드하거나 준비할 때 mip chain이 있어야 합니다.

일반 Sample용 필터에서는 `ComparisonFunc`를 사용하지 않습니다. 비교용 필터를 선택했을 때 의미가 있습니다. 상태 객체는 생성 후 불변이므로 `desc.Filter` 값을 바꾸는 것만으로 이미 만든 sampler가 변하지 않습니다. 비교할 상태를 각각 생성해 두고 Bind하는 객체를 바꿉니다.

## 6. UV가 0~1 밖으로 나가면 Address 모드가 결정합니다

U를 0에서 2까지 주면 사각형이 원본 이미지의 가로 두 배 구간을 읽으려 합니다. 이때 밖의 좌표를 반복할지, 뒤집을지, 가장자리에 고정할지를 정합니다.

{{M8}}

다음 표는 주소가 대응하는 위치를 이해하기 위한 예입니다. 실제 Linear 필터는 경계 주변의 texel이나 BorderColor를 함께 섞을 수 있습니다.

| AddressU | U=-0.2 | U=1.2 | U=2.2 | 화면에서 보이는 모습 |
| --- | --- | --- | --- | --- |
| WRAP | 0.8 | 0.2 | 0.2 | 같은 무늬 반복 |
| MIRROR | 0.2 | 0.8 | 0.2 | 정상·좌우 반전을 번갈아 반복 |
| CLAMP | 0 | 1 | 1 | 가장자리 색을 늘림 |
| MIRROR_ONCE | 0.2 | 1 | 1 | 절댓값을 취한 뒤 Clamp |
| BORDER | 경계색 | 경계색 | 경계색 | 범위 밖에 BorderColor 사용 |

Wrap의 음수 좌표는 C++의 음수 나머지를 그대로 대입하는 식으로 이해하지 않습니다. 반복되는 텍스처 공간에서 -0.2는 앞 타일의 0.8 위치와 대응합니다. MirrorOnce도 ‘1~2 구간을 한 번 더 반사한다’는 뜻이 아니라 `abs` 후 Clamp입니다.

바닥 타일은 Wrap, 이미지 한 장을 보여 주는 UI는 Clamp부터 비교해 보세요. Border에서 투명색을 쓰더라도 최종 화면이 투명해지려면 PS 알파와 Blend 상태가 함께 연결되어 있어야 합니다.

## 7. 확대 필터와 mip 선택을 구별해서 실험합니다

{{M9}}

먼저 사각형을 크게 만들어 Point와 Linear만 바꿉니다. 경계의 네모 모양과 부드러움이 달라집니다. 이번에는 사각형을 작게 만들고 mip이 준비된 텍스처를 사용합니다. 먼 무늬가 안정되는지 관찰합니다. 이 두 실험을 동시에 바꾸면 어느 설정이 원인인지 알기 어렵습니다.

Anisotropic 비교에서는 위 desc의 Filter를 `D3D11_FILTER_ANISOTROPIC`으로 바꾸고 `MaxAnisotropy=8` 같은 값을 지정해 새 객체를 생성합니다. `MipLODBias`를 음수로 주어 더 자세한 mip을 강제로 선택하는 방식은 선명해 보일 수 있지만 깜빡임이 늘 수 있으므로 같은 해결책이 아닙니다.

### 비교 샘플러는 색을 읽는 샘플러와 다릅니다

그림자 맵에서는 텍스처의 깊이값과 현재 표면의 깊이를 비교하여 빛이 도달하는지 판단할 수 있습니다. 일반 Sample은 값을 읽지만, SampleCmp 계열은 비교 결과를 반환합니다. 아래 참고 API와 함수에서 차이를 확인할 수 있습니다.

{{M10}}

예를 들어 유효한 나머지 desc 설정을 유지한 채 Filter를 `D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT`, ComparisonFunc를 `D3D11_COMPARISON_LESS_EQUAL`로 설정합니다. 대응하는 HLSL은 `SamplerComparisonState`를 사용합니다. 이 예제는 그림자 비교 한 부분이며 그림자용 카메라와 깊이 RT를 만드는 전체 구현은 별도입니다.

## 8. t0의 이미지와 s0의 규칙이 Sample에서 만납니다

{{M11}}

```c++
// HLSL: 픽셀 셰이더
Texture2D imageTexture : register(t0);
SamplerState imageSampler : register(s0);

float4 main(float4 position : SV_Position,
            float2 uv : TEXCOORD) : SV_Target
{
    return imageTexture.Sample(imageSampler, uv);
}
```

`t0`에는 Texture 강의에서 만든 SRV를 `PSSetShaderResources`로 연결하고, `s0`에는 위 Sampler를 `PSSetSamplers`로 연결합니다. 두 슬롯의 번호가 같아도 서로 덮어쓰지 않습니다. t0은 읽을 데이터이고 s0은 읽는 규칙입니다. VS가 TEXCOORD로 넘긴 UV는 Sample의 위치 인자로 들어갑니다.

이 DX11 SM4/5 픽셀 셰이더의 `Sample`은 주변 픽셀에서 UV가 변하는 정도를 이용해 mip을 선택합니다. 정점 셰이더처럼 같은 화면 미분 정보를 사용할 수 없는 단계에서는 필요한 경우 `SampleLevel`로 LOD를 명시하는 등 해당 단계가 지원하는 함수를 선택합니다. 함수 이름이 비슷하다고 모든 단계에서 동일하게 사용할 수 있는 것은 아닙니다.

이제 HLSL은 그대로 두고 Sampler만 Point로 바꿔 보세요. 다음에는 Linear를 유지하고 Wrap을 Clamp로 바꿔 U=0~2 사각형을 봅니다. 전자는 **경계 주변 색을 어떻게 고르는가**, 후자는 **범위 밖 위치를 어떻게 처리하는가**를 바꾼 실험입니다. Texture·UV·Sampler 세 가지의 역할을 나누어 보면, 화면이 흐리거나 무늬가 늘어날 때 어디를 수정해야 할지 찾을 수 있습니다.

[Microsoft: 텍스처 주소 모드](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_texture_address_mode) · [Microsoft: 비등방성 필터링](https://learn.microsoft.com/en-us/windows/win32/direct3d9/anisotropic-texture-filtering)
