
RWTexture2D<float4> outputTex : register(u0);


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    outputTex.GetDimensions(width, height);
    
    uint2 pixel = DTid.xy;
    
    if (pixel.x >= width || pixel.y >= height)
        return;
    
    float2 texSize = float2(width, height);
    float2 normalizedCord = float2(pixel) / float2(texSize);
    
    //normalizedCord = normalizedCord * 2.0f - 1.0f;
    
    //normalizedCord.y = 1.0f - normalizedCord.y;

    
    outputTex[pixel] = float4(1.0f - normalizedCord.x, normalizedCord.x, 0.0f, 1.0f);

}