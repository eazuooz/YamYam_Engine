Texture2D sprite : register(t0);
SamplerState spriteSampler : register(s0);

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(VSOutput input) : SV_Target
{
    float4 color = sprite.Sample(spriteSampler, input.uv) * input.color;
    clip(color.a - 0.01f);
    return color;
}
