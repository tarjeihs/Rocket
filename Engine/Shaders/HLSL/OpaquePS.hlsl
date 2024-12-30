#define uint32  uint
#define int32   int

struct FPSInput
{
    float4 ClipSpacePosition        : SV_POSITION;
    float3 WorldSpacePosition       : WORLD_POS;
    float3 Normal                   : NORMAL;
    float2 TexCoord                 : TEXCOORD0;
};

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
};

struct FMaterialStorageBuffer
{
    uint32 AlbedoTextureID;
    uint32 NormalTextureID;
};

struct FObjectStorageBuffer
{
    float4x4 Transform;
    float4x4 TransformInverseTranspose;
};

Texture2D<float4> Textures[] : register(t0, space1);
SamplerState Sampler : register(s0, space1);

float4 main(FPSInput Input) : SV_TARGET
{
    return float4(normalize(Input.Normal * 0.5f + 0.5f), 1.0f);
}