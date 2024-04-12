#include "Samplers.hlsli"
#include "Textures.hlsli"


struct VS_Input
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(VS_Output input) : SV_Target
{
    float4 color = albedo.Sample(anisotropicSampler, input.uv);
    return color;
}