//#include "ConstantBuffers.hlsli"

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
    //float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    //float2 uv : TEXCOORD;
};


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0.0f;
    
    //float4 pos = mul(float4(input.pos, 1.0f), WorldMatrix);
    //float4 viewPos = mul(pos, ViewMatrix);
    //float4 projPos = mul(viewPos, ProjectionMatrix);
    //output.pos = projPos;
    output.pos = float4(input.pos, 1.0); // <- 이거 중요!
    output.color = input.color;

    return output;
}
