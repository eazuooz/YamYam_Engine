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


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0.0f;
    output.pos = float4(input.pos, 1.0);
    output.color = input.color;

    return output;
}
