#include "Samplers.hlsli"

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 main(VSOutput input) : SV_Target
{
#if defined(YA_ALPHA_TEST)
    clip(input.color.a - 0.01f);
#endif
    return input.color;
}