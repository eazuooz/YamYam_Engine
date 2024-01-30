struct VS_Input
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VS_Output
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

float4 main(VS_Output output) : SV_Target
{
    return output.color;
}