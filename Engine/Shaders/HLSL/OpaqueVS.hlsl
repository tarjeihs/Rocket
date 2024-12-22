#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

#define uint32  uint
#define int32   int

struct FVSInput
{
    float3 Position                 : POSITION;
    float3 Normal                   : NORMAL;
    float2 TexCoord                 : TEXCOORD;
};

struct FVSOutput
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
    float4x4 Normal;
};

StructuredBuffer<FGlobalStorageBuffer> GlobalStorageBuffer      : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer> CameraStorageBuffer      : register(t1, space0);
StructuredBuffer<FMaterialStorageBuffer> MaterialStorageBuffer  : register(t2, space0);
StructuredBuffer<FObjectStorageBuffer> ObjectStorageBuffer      : register(t3, space0);

FVSOutput main(FVSInput Input, uint32 InstanceID : SV_InstanceID)
{
    FVSOutput Output;

    float4 WorldSpacePosition   = mul(ObjectStorageBuffer[InstanceID].Transform, float4(Input.Position, 1.0f));
    float4 ViewPosition         = mul(CameraStorageBuffer[0].View, WorldSpacePosition);
    float4 ClipSpacePosition    = mul(CameraStorageBuffer[0].Projection, ViewPosition);

    Output.ClipSpacePosition    = ClipSpacePosition;
    Output.WorldSpacePosition   = WorldSpacePosition.xyz;
    Output.Normal               = Input.Normal;
    Output.TexCoord             = Input.TexCoord;
    return Output;
}