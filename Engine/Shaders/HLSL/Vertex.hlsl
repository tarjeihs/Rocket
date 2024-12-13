#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

struct FVertex
{
    float3 Position : POSITION;
};

struct FResult
{
    float4 Position : SV_POSITION;
};

struct FGlobalStorageBuffer
{
    float3 Value;
};

struct FCameraStorageBuffer
{
    float Value;
};

struct FMaterialStorageBuffer
{
    float Value;
};

struct FObjectStorageBuffer
{
    float Value;
};

StructuredBuffer<FGlobalStorageBuffer> GlobalStorageBuffer : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer> CameraStorageBuffer : register(t0, space1);
StructuredBuffer<FMaterialStorageBuffer> MaterialStorageBuffer : register(t0, space2);
StructuredBuffer<FObjectStorageBuffer> ObjectStorageBuffer : register(t0, space3);

FResult main(float4 Position : POSITION)
{
    FResult Result;
    Result.Position = float4(Position.xyz, 1.0);
    float3 A = GlobalStorageBuffer[0].Value;
    return Result;
}

// (UBO) Set 0 Binding 0 = Global Data
// (UBO) Set 0 Binding 1 = Camera Data

// (SSBO) Set 1 Binding 0 = Material Data
// (SSBO) Set 1 Binding 1 = Object Data
