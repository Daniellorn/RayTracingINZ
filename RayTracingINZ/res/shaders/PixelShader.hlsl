Texture2D<float4> inputTex : register(t0);
SamplerState samp : register(s0);

struct VSinput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};


float4 main(VSinput input) : SV_Target
{
    float4 hdrTex = inputTex.Sample(samp, input.uv);
    
    //return float4(mapped, 1.0f);
    return hdrTex;
} 