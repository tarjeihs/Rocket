struct VS_INPUT 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 color;
};

struct VS_OUTPUT 
{
    float4 position : SV_POSITION;
    float3 outColor : COLOR;
    float2 outUV : TEXCOORD0;
};

struct SShaderStorageBufferObject
{
    float4x4 ModelMatrix;
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

#define VK_RAW_BUFFER_LOAD(Type, Addr, Offset, Position) vk::RawBufferLoad<Type>(Addr + Offset + Position)

VS_OUTPUT main(uint vertexIndex : SV_VertexID) 
{
    VS_OUTPUT output;

    // Size of a single vertex in the buffer
    uint vertexSize = 48;
    
    // Calculate the base offset for the current vertex
    uint vertexOffset = vertexIndex * vertexSize;

    // Load each attribute from the buffer with the correct offset
    float3 pos = VK_RAW_BUFFER_LOAD(float3, pushConstant.BufferDeviceAddress, vertexOffset, 0);
    float uv_x = vk::RawBufferLoad<float>(pushConstant.BufferDeviceAddress + vertexOffset + 12);
    float3 normal = vk::RawBufferLoad<float3>(pushConstant.BufferDeviceAddress + vertexOffset + 16);
    float uv_y = vk::RawBufferLoad<float>(pushConstant.BufferDeviceAddress + vertexOffset + 28);
    float4 color = vk::RawBufferLoad<float4>(pushConstant.BufferDeviceAddress + vertexOffset + 32);

    float4 worldPosition = mul(SSBO[0].ModelMatrix, float4(pos, 1.0));
    float4 viewPosition = mul(m_ViewMatrix, worldPosition);
    output.position = mul(m_ProjectionMatrix, viewPosition);
    output.outColor = color.xyz;  // Explicitly truncate the float4 to float3
    output.outUV = float2(uv_x, uv_y);

    return output;
}
