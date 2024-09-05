#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : TEXCOORD3;
    float2 outUV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 outColor : TEXCOORD2;
};

struct SShaderStorageBufferObject
{
    float4x4 ModelMatrix;
    float4x4 NormalMatrix;
};

StructuredBuffer<SShaderStorageBufferObject> SSBO : register(t0);

cbuffer UBO : register(b1) // Binding 1 in Vulkan
{
    float4x4 m_ModelMatrix;
    float4x4 m_ViewMatrix;
    float4x4 m_ProjectionMatrix;
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
    uint vertexSize = 48;
    
    // Calculate the base offset for the current vertex
    uint vertexOffset = vertexIndex * vertexSize;

    // Load each attribute from the buffer with the correct offset
    float3 pos = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 0);
    float2 uv = vk::RawBufferLoad<float2>(pushConstant.BufferDeviceAddress + vertexOffset + 12);
    float3 normal = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 20);
    float4 color = vk::RawBufferLoad<float4>(pushConstant.BufferDeviceAddress + vertexOffset + 32);

    //loat4 worldPosition = mul(SSBO[0].ModelMatrix, float4(pos, 1.0));
    float4 worldPosition = mul(m_ModelMatrix, float4(pos, 1.0));
    float3 worldNormal = normalize(mul(SSBO[0].NormalMatrix, normal));  // Transform normal to world space
    float4 viewPosition = mul(m_ViewMatrix, worldPosition);
    output.worldPosition = worldPosition;
    output.position = mul(m_ProjectionMatrix, viewPosition);
    output.outUV = uv;
    output.Normal = worldNormal;
    output.outColor = color.xyz;

    return output;
}