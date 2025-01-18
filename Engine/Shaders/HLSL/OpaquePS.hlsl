#define uint32  uint
#define int32   int
#define PI      3.14f

float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

struct FPSInput
{
    float4 ClipSpacePosition        : SV_POSITION;
    float3 WorldSpacePosition       : WORLD_POS;
    float3 Normal                   : NORMAL;
    float2 TexCoord                 : TEXCOORD0;
    float3 Tangent                  : TEXCOORD1;
    float3 Bitangent                : TEXCOORD2;
    uint32 InstanceID               : COLOR0;
};

struct FGlobalStorageBuffer
{
    float Time;
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
    int32 AlbedoTextureID;
    int32 NormalTextureID;
    int32 RoughnessTextureID;
    int32 MetallicTextureID;
};

struct FInstanceStorageBuffer
{
    float4x4 Transform;
    float4x4 TransformInverseTranspose;
};

StructuredBuffer<FGlobalStorageBuffer> GlobalStorageBuffer[] : register(t0, space0);
StructuredBuffer<FCameraStorageBuffer> CameraStorageBuffer[] : register(t0, space0);
StructuredBuffer<FMaterialStorageBuffer> MaterialStorageBuffer[] : register(t0, space0);
StructuredBuffer<FInstanceStorageBuffer> InstanceStorageBuffer[] : register(t0, space0);

static const int BINDLESS_BUFFER_INDEX_GLOBAL = 0;
static const int BINDLESS_BUFFER_INDEX_CAMERA = 1;
static const int BINDLESS_BUFFER_INDEX_MATERIAL = 2;
static const int BINDLESS_BUFFER_INDEX_INSTANCE = 3;

Texture2D<float4> Textures[] : register(t2, space0);
SamplerState Sampler : register(s2, space0);

float4 main(FPSInput Input) : SV_TARGET
{
    FGlobalStorageBuffer GlobalBuffer = GlobalStorageBuffer[BINDLESS_BUFFER_INDEX_GLOBAL][0];
    FCameraStorageBuffer CameraBuffer = CameraStorageBuffer[BINDLESS_BUFFER_INDEX_CAMERA][0];
    FMaterialStorageBuffer MaterialBuffer = MaterialStorageBuffer[BINDLESS_BUFFER_INDEX_MATERIAL][Input.InstanceID];
    FInstanceStorageBuffer InstanceBuffer = InstanceStorageBuffer[BINDLESS_BUFFER_INDEX_INSTANCE][Input.InstanceID];
    
    // Sample textures
    float4 Albedo = Textures[MaterialBuffer.AlbedoTextureID].Sample(Sampler, Input.TexCoord);
    float3 Normal = Textures[MaterialBuffer.NormalTextureID].Sample(Sampler, Input.TexCoord).rgb;
    float Metallic = Textures[MaterialBuffer.MetallicTextureID].Sample(Sampler, Input.TexCoord).r;
    float Roughness = Textures[MaterialBuffer.RoughnessTextureID].Sample(Sampler, Input.TexCoord).r;

    // Convert normal map from [0,1] to [-1,1]
    Normal = Normal * 2.0f - 1.0f;

    // Transform normal to world space
    float3 T = normalize(Input.Tangent);
    float3 B = normalize(Input.Bitangent);
    float3 N = normalize(Input.Normal);
    float3 WorldNormal = normalize(Normal.x * T + Normal.y * B + Normal.z * N);

    // Light and view directions
    float3 LightDirection = normalize(float3(1.0f, 1.0f, 1.0f)); // Point toward object
    float3 ViewDirection = normalize(CameraBuffer.Position.xyz - Input.WorldSpacePosition);
    float3 HalfVector = normalize(LightDirection + ViewDirection);

    // NdotL: Dot product of normal and light direction (clamp to [0, 1])
    float NdotL = max(dot(WorldNormal, LightDirection), 0.0f);

    // NdotV: Dot product of normal and view direction (clamp to [0, 1])
    float NdotV = max(dot(WorldNormal, ViewDirection), 0.0f);

    // NdotH: Dot product of normal and half vector
    float NdotH = max(dot(WorldNormal, HalfVector), 0.0f);

    // VdotH: Dot product of view direction and half vector
    float VdotH = max(dot(ViewDirection, HalfVector), 0.0f);

    // Fresnel-Schlick approximation
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), Albedo.rgb, Metallic);
    float3 Fresnel = F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);

    // Normal distribution function (GGX)
    float alpha = Roughness * Roughness;
    float alphaSq = alpha * alpha;
    float D = alphaSq / (PI * pow(NdotH * NdotH * (alphaSq - 1.0f) + 1.0f, 2.0f));

    // Geometry function (Schlick-GGX)
    float k = Roughness * Roughness / 2.0f;
    float Geometry = GeometrySchlickGGX(NdotL, k) * GeometrySchlickGGX(NdotV, k);

    // Apply energy conservation to diffuse
    float3 Diffuse = Albedo.rgb * (1.0f - Metallic) * (1.0f - Fresnel) * NdotL;

    // Final specular term
    float3 Specular = (D * Fresnel * Geometry) / (4.0f * NdotL * NdotV + 0.001f);

    // Combine diffuse and specular
    float3 FinalColor = Diffuse + Specular;

    // Return final color
    //return float4(FinalColor, 1.0f);
    
    float3 normal = normalize(Input.Normal); // Assuming the normal is passed from the vertex shader
    float3 normalColor = 0.5f * normal + 0.5f; // Normalize to [0, 1] range

    return float4(normalColor, 1.0f);
}