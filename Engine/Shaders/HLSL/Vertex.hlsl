#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

struct FVertexOutput
{
    float4 Position : SV_POSITION;
};

struct FGlobalStorageBuffer
{
};

StructuredBuffer<FGlobalStorageBuffer> GlobalStorageBuffer : register(t0, space0);
StructuredBuffer<FGlobalStorageBuffer> CameraStorageBuffer : register(t0, space1);
StructuredBuffer<FGlobalStorageBuffer> MaterialStorageBuffer : register(t0, space2);
StructuredBuffer<FGlobalStorageBuffer> ObjectStorageBuffer : register(t0, space3);

FVertexOutput main(uint vertexIndex : SV_VertexID) 
{
    FVertexOutput Output;
    return Output;
}

// (UBO) Set 0 Binding 0 = Global Data
// (UBO) Set 0 Binding 1 = Camera Data

// (SSBO) Set 1 Binding 0 = Material Data
// (SSBO) Set 1 Binding 1 = Object Data
