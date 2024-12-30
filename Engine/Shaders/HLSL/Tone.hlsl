// A tone mapping HDR16-to-SRGB8 compute shader.
// TODO: Bloom Effect, Vignette, Film Grain, Chromatic Aberration, Color Grading, Depth of Field (DOF), Tone Splitting, Lens Distortion, Motion Blur, Dithering

float3 ToACES(float3 color)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return saturate((color * (a * color + b)) / (color * (c * color + d) + e));
}

float3 ToSRGB(float3 color)
{
    float3 lowRange = color * 12.92f;
    float3 highRange = pow(color, 1.0f / 2.4f) * 1.055f - 0.055f;
    return saturate(select(color <= 0.0031308f, lowRange, highRange));
}

RWTexture2D<float4>                         Input16[]                 : register(u1, space0);
RWTexture2D<unorm float4>                   Output8[]                 : register(u1, space0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 Pixel = dispatchThreadID.xy;
 
    float4 HdrColor = Input16[0].Load(int3(Pixel.xy, 0));
    float3 ToneMappedColor = ToACES(HdrColor.rgb);
    float3 SRGBColor = ToSRGB(ToneMappedColor);
 
    Output8[1][Pixel] = float4(SRGBColor, HdrColor.a);
}