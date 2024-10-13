#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;         // Screen-space position
    float3 worldPosition : TEXCOORD3;      // World-space position
    float2 TexCoord : TEXCOORD0;           // UV coordinates
    float3 Normal : TEXCOORD1;             // World-space normal vector
    float3 Color : TEXCOORD2;              // Vertex color
    float3 Tangent : TEXCOORD4;            // Tangent vector
    float3 Bitangent : TEXCOORD5;          // Bitangent vector
};

struct SShaderStorageBufferObject
{
    float4x4 ModelMatrix;
    float4x4 NormalMatrix;
};

StructuredBuffer<SShaderStorageBufferObject> SSBO : register(t0, space0);

// A cbuffer is fixed in size, and is NOT an array.
cbuffer UBO : register(b0, space1) // Binding 1 in Vulkan
{
    float4x4 m_ModelMatrix;
    float4x4 m_ViewMatrix;
    float4x4 m_ProjectionMatrix;
    float3 CameraWorldPosition;  // Pass the camera position directly here
};

struct PushConstant 
{
    uint64_t BufferDeviceAddress;
    uint ObjectID;
};

[[vk::push_constant]]
PushConstant pushConstant;

VS_OUTPUT main(uint vertexIndex : SV_VertexID) 
{
    VS_OUTPUT output;

    // Size of a single vertex in the buffer
    uint vertexSize = 96;
    
    // Calculate the base offset for the current vertex
    uint vertexOffset = vertexIndex * vertexSize;

    // Load each attribute from the buffer with the correct offset
    float3 pos = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 0);
    float2 uv = vk::RawBufferLoad<float2>(pushConstant.BufferDeviceAddress + vertexOffset + 16);
    float3 normal = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 32);
    float4 color = vk::RawBufferLoad<float4>(pushConstant.BufferDeviceAddress + vertexOffset + 48);
    float3 Tangent = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 64);
    float3 Bitangent = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 80);

    float4 worldPosition = mul(SSBO[pushConstant.ObjectID].ModelMatrix, float4(pos, 1.0));
    float3 worldNormal = normalize(mul(SSBO[pushConstant.ObjectID].NormalMatrix, normal));  // Transform transpose inverse normal to world space
    float4 viewPosition = mul(m_ViewMatrix, worldPosition);
    output.worldPosition = worldPosition;
    output.Position = mul(m_ProjectionMatrix, viewPosition);
    output.TexCoord = uv;
    output.Normal = worldNormal;
    output.Color = color.xyz;
    output.Tangent = mul((float3x3)SSBO[pushConstant.ObjectID].ModelMatrix, Tangent);
    output.Bitangent = mul((float3x3)SSBO[pushConstant.ObjectID].ModelMatrix, Bitangent);

    return output;
}