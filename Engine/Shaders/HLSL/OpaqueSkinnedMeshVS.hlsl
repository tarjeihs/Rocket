#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

#define uint32  uint
#define int32   int

struct FVSInput
{
    float3 Position                 : POSITION;
    float3 Normal                   : NORMAL;
    float2 TexCoord                 : TEXCOORD;
    float4 Weight                   : BLENDWEIGHT;
    int4 Indices                    : BLENDINDICES;
};

struct FVSOutput
{
    float4 ClipSpacePosition        : SV_POSITION;
    float3 WorldSpacePosition       : WORLD_POS;
    float3 Normal                   : NORMAL;
    float2 TexCoord                 : TEXCOORD0;
    float4 Weight                   : BLENDWEIGHT;
    int4 Indices                    : BLENDINDICES;
};

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
    uint32 AlbedoTextureID;
    uint32 NormalTextureID;
};

struct FInstanceStorageBuffer
{
    float4x4 Transform;
    float4x4 TransformInverseTranspose;
};

struct FAnimationStorageBuffer
{
    float4x4 Transform; // Bone
};

StructuredBuffer<FGlobalStorageBuffer>      GlobalStorageBuffer[]         : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer>      CameraStorageBuffer[]         : register(t0, space0);
StructuredBuffer<FMaterialStorageBuffer>    MaterialStorageBuffer[]       : register(t0, space0);
StructuredBuffer<FInstanceStorageBuffer>    InstanceStorageBuffer[]       : register(t0, space0);
StructuredBuffer<FAnimationStorageBuffer>   AnimationStorageBuffer[]       : register(t0, space0);

static const int BINDLESS_BUFFER_INDEX_GLOBAL       = 0;
static const int BINDLESS_BUFFER_INDEX_CAMERA       = 1;
static const int BINDLESS_BUFFER_INDEX_MATERIAL     = 2;
static const int BINDLESS_BUFFER_INDEX_INSTANCE     = 3;
static const int BINDLESS_BUFFER_INDEX_ANIMATION    = 4;

FVSOutput main(FVSInput Input, uint32 InstanceID : SV_InstanceID)
{
    FVSOutput Output;

    FGlobalStorageBuffer        GlobalBuffer        = GlobalStorageBuffer       [BINDLESS_BUFFER_INDEX_GLOBAL]      [0];
    FCameraStorageBuffer        CameraBuffer        = CameraStorageBuffer       [BINDLESS_BUFFER_INDEX_CAMERA]      [0];
    FMaterialStorageBuffer      MaterialBuffer      = MaterialStorageBuffer     [BINDLESS_BUFFER_INDEX_MATERIAL]    [InstanceID];
    FInstanceStorageBuffer      InstanceBuffer      = InstanceStorageBuffer     [BINDLESS_BUFFER_INDEX_INSTANCE]    [InstanceID];

    float4 WorldSpacePosition   = mul(InstanceBuffer.Transform, float4(Input.Position, 1.0f));
    float4 ViewPosition         = mul(CameraBuffer.View, WorldSpacePosition);
    float4 ClipSpacePosition    = mul(CameraBuffer.Projection, ViewPosition);
    float3 WorldSpaceNormal     = mul(InstanceBuffer.TransformInverseTranspose, float4(Input.Normal, 0.0f)).xyz;

    Output.ClipSpacePosition    = ClipSpacePosition;
    Output.WorldSpacePosition   = WorldSpacePosition.xyz;
    Output.Normal               = normalize(WorldSpaceNormal);
    Output.TexCoord             = Input.TexCoord;
    Output.Weight = Input.Weight;
    Output.Indices = Input.Indices;

    return Output;
}