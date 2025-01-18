struct FGlobalStorageBuffer
{
    float DeltaTime;
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
    uint AlbedoTextureID;
    uint NormalTextureID;
};

struct FInstanceStorageBuffer
{
    float4x4 Transform;
    float4x4 TransformInverseTranspose;
};

StructuredBuffer<FGlobalStorageBuffer>      GlobalStorageBuffer[]         : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer>      CameraStorageBuffer[]         : register(t0, space0);
StructuredBuffer<FMaterialStorageBuffer>    MaterialStorageBuffer[]       : register(t0, space0);
StructuredBuffer<FInstanceStorageBuffer>    InstanceStorageBuffer[]       : register(t0, space0);

struct FDrawCommand
{
    uint IndexCount;
    uint InstanceCount;
    uint FirstIndex;
    int VertexOffset;
    uint FirstInstance;
};