#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

#define uint32  uint
#define int32   int

struct FVSInput
{
    float3 Position : POSITION;
};

struct FVSOutput
{
    float4 Position : SV_POSITION;
};

struct FGlobalStorageBuffer
{
    float4x4 View;
    float4x4 Projection;
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
};

StructuredBuffer<FGlobalStorageBuffer> GlobalStorageBuffer : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer> CameraStorageBuffer : register(t0, space1);
StructuredBuffer<FMaterialStorageBuffer> MaterialStorageBuffer : register(t0, space2);
StructuredBuffer<FObjectStorageBuffer> ObjectStorageBuffer : register(t0, space3);

FVSOutput main(FVSInput VSInput, uint32 InstanceID : SV_InstanceID)
{
    FVSOutput Output;

const float Columns = 1000.0f;      // Number of columns in the grid
const float Spacing = 10.0f;        // Spacing between grid points

float4 ModelPosition = float4(
    VSInput.Position.x + (InstanceID % Columns) * Spacing,   // Scale x position
    VSInput.Position.y,                 // Keep z position
    VSInput.Position.z + (InstanceID / Columns) * Spacing,   // Scale y position
    1.0f                                // Homogeneous coordinate
);


    // Transform the position using View and Projection matrices
    float4 ViewPosition = mul(GlobalStorageBuffer[0].View, ModelPosition);       // Transform to view space
    Output.Position = mul(GlobalStorageBuffer[0].Projection, ViewPosition);      // Transform to clip space
    return Output;
}

// (UBO) Set 0 Binding 0 = Global Data
// (UBO) Set 0 Binding 1 = Camera Data

// (SSBO) Set 1 Binding 0 = Material Data
// (SSBO) Set 1 Binding 1 = Object Data
