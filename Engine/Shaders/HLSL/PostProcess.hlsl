// A tone mapping HDR16-to-SRGB8 compute shader.
// TODO: Bloom Effect, Vignette, Film Grain, Chromatic Aberration, Color Grading, Depth of Field (DOF), Tone Splitting, Lens Distortion, Motion Blur, Dithering

float3 BlurTexture(RWTexture2D<float4> texture, uint2 pixelCoord, int radius)
{
    float3 colorSum = float3(0.0f, 0.0f, 0.0f);
    int count = 0;
    float2 resolution = float2(1280, 720);
    
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            uint2 sampleCoord = pixelCoord + uint2(x, y);

            // Ensure we stay within texture bounds
            if (sampleCoord.x >= 0 && sampleCoord.x < resolution.x && sampleCoord.y >= 0 && sampleCoord.y < resolution.y)
            {
                float4 sample = texture.Load(int3(sampleCoord, 0));
                colorSum += sample.rgb;
                count++;
            }
        }
    }
    return colorSum / count;
}

float3 ApplyFilmGrain(float grainIntensity, float3 color, uint2 pixel)
{
    float randomValue = frac(sin(dot(pixel.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
    float3 grain = randomValue * grainIntensity;

    return color + grain;
}

float3 ApplyChromaticAberration(float intensity, float3 centerColor, float3 leftColor, float3 rightColor)
{
    return lerp(centerColor, float3(leftColor.r, centerColor.g, rightColor.b), intensity);
}

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

struct FGlobalStorageBuffer
{
    float Time;
};

struct FCameraStorageBuffer
{
    float4x4 View;
    float4x4 Projection;
    float4 Position;
    float4 Direction;
    float4 PP_FilmGrainIntensity;
    float4 PP_ChromaticAberration;
};

StructuredBuffer<FGlobalStorageBuffer>      GlobalStorageBuffer[]         : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer>      CameraStorageBuffer[]         : register(t0, space0);

static const int BINDLESS_BUFFER_INDEX_GLOBAL       = 0;
static const int BINDLESS_BUFFER_INDEX_CAMERA       = 1;

RWTexture2D<float4>                         Input16[]                 : register(u1, space0);
RWTexture2D<unorm float4>                   Output8[]                 : register(u1, space0);

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    FGlobalStorageBuffer        GlobalBuffer        = GlobalStorageBuffer       [BINDLESS_BUFFER_INDEX_GLOBAL]      [0];
    FCameraStorageBuffer        CameraBuffer        = CameraStorageBuffer       [BINDLESS_BUFFER_INDEX_CAMERA]      [0];

    uint2 Pixel = dispatchThreadID.xy;
 
    float4 HdrColor = Input16[0].Load(int3(Pixel.xy, 0));
    float4 LeftColor = Input16[0].Load(int3(Pixel.xy - uint2(1, 0), 0));
    float4 RightColor = Input16[0].Load(int3(Pixel.xy + uint2(1, 0), 0));

    float3 Color = HdrColor.rgb;

    //Color = ToACES(Color);
    //Color = ToSRGB(Color);
    //Color = ApplyFilmGrain(CameraBuffer.PP_FilmGrainIntensity.x, Color, Pixel.xy);
    //Color = ApplyChromaticAberration(CameraBuffer.PP_ChromaticAberration, Color, LeftColor.rgb, RightColor.rgb);

    Output8[1][Pixel] = float4(Color, HdrColor.a);
}