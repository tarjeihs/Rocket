#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

struct FVSOutput
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
    float3 Normal       : TEXCOORD1;
    float3 Color        : TEXCOORD2;
    float3 Tangent      : TEXCOORD4;
    float3 Bitangent    : TEXCOORD5;
};

struct FEnvironment
{

};

struct FCamera
{
    
};

struct FMaterial
{
    
};

struct FObject
{
    
};

struct FPushConstant
{
    uint EnvironmentID;
    uint CameraID;
    uint MaterialID;
    uint ObjectID;
};

StructuredBuffer<FEnvironment> Environment : register(t0, space1);
StructuredBuffer<FCamera> Camera : register(t1, space1);
StructuredBuffer<FMaterial> Material : register(t2, space1);
StructuredBuffer<FObject> Object : register(t3, space1);

[[vk::push_constant]]
FPushConstant PushConstant;

FVSOutput main(uint vertexIndex : SV_VertexID) 
{
    FVSOutput VSOutput;

    return VSOutput;
}

//uint cameraBufferIndex = 0;  // Example: index for the camera data buffer
//uint objectBufferIndex = 1;  // Example: index for object data buffer
//uint materialBufferIndex = 2; // Example: index for material data buffer
//
//// Offset in bytes within the buffer for the respective struct (assuming data is aligned correctly).
//uint cameraDataOffset = 0;
//uint objectDataOffset = 0;
//uint materialDataOffset = 0;
//
//// Load camera data from the bindless SSBO
//FCameraData GetCameraData(uint index) {
//    FCameraData data;
//    data.position = bindless_buffers[index].Load4(cameraDataOffset);         // Load float4
//    data.viewDirection = bindless_buffers[index].Load4(cameraDataOffset + 16); // Load float4 (offset by 16 bytes for float4)
//    data.viewMatrix = bindless_buffers[index].LoadMatrix4x4(cameraDataOffset + 32);  // Load float4x4 (offset by 32 bytes for 2 float4)
//    data.projMatrix = bindless_buffers[index].LoadMatrix4x4(cameraDataOffset + 96);  // Load float4x4 (offset by 64 bytes for the first matrix)
//    return data;
//}
//
//// Load object data from the bindless SSBO
//FObjectData GetObjectData(uint index) {
//    FObjectData data;
//    data.position = bindless_buffers[index].Load4(objectDataOffset); // Load float4 for position
//    data.orientation = bindless_buffers[index].Load4(objectDataOffset + 16); // Load float4 for orientation
//    data.scale = bindless_buffers[index].Load4(objectDataOffset + 32); // Load float4 for scale
//    data.modelMatrix = bindless_buffers[index].LoadMatrix4x4(objectDataOffset + 48); // Load model matrix
//    return data;
//}
//
//// Load material data from the bindless SSBO
//FMaterialData GetMaterialData(uint index) {
//    FMaterialData data;
//    data.baseColor = bindless_buffers[index].Load4(materialDataOffset); // Load float4 for base color
//    data.roughness = bindless_buffers[index].Load(materialDataOffset + 16); // Load float for roughness
//    data.metallic = bindless_buffers[index].Load(materialDataOffset + 20);  // Load float for metallic
//    return data;
//}
//