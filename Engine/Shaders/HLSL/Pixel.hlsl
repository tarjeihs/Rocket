// Constants
#define PI 3.14159265359

// Input structure
struct SPSInput
{
    float4 Position : SV_POSITION;         // Screen-space position
    float3 worldPosition : TEXCOORD3;      // World-space position
    float2 TexCoord : TEXCOORD0;           // UV coordinates
    float3 Normal : TEXCOORD1;             // World-space normal vector
    float3 Color : TEXCOORD2;              // Vertex color
    float3 Tangent : TEXCOORD4;            // Tangent vector
    float3 Bitangent : TEXCOORD5;          // Bitangent vector
};

// Constant buffer (UBO)
cbuffer UBO : register(b0, space1) // Binding 1 in Vulkan
{
    float4x4 m_ModelMatrix;
    float4x4 m_ViewMatrix;
    float4x4 m_ProjectionMatrix;
    float3 CameraWorldPosition;  // Pass the camera position directly here
};

// Material data buffer
cbuffer SMaterialData : register(b0, space2)
{
    float4 BaseColor;       // RGB for albedo, A for alpha
    float Metallic;         // Metallic factor
    float Roughness;        // Roughness factor
    float Specular;         // Specular intensity
    float Subsurface;       // Subsurface scattering factor
    float SpecularTint;     // Specular tint factor
    float Anisotropic;      // Anisotropy factor
    float Sheen;            // Sheen factor
    float SheenTint;        // Sheen tint factor
    float Clearcoat;        // Clearcoat factor
    float ClearcoatGloss;   // Clearcoat gloss factor
};

// Light data buffer
cbuffer SLightData : register(b1, space2)
{
    float3 LightPosition;
    float3 LightColor;
};

float4 main(SPSInput input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
