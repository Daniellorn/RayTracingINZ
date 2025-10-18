
struct VSinput
{
    float2 pos : POSITION;
};

struct VSoutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSoutput main( VSinput input )
{
    VSoutput output;
    
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.pos * 0.5f + 0.5f;
    
    return output;
}