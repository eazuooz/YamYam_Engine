// TODO: Texture 구현 후 sprite.Sample(anisotropicSampler, input.uv) 로 교체 예정

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(VSOutput input) : SV_Target
{
    return input.color;
}
