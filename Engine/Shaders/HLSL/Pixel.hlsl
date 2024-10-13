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

// Function to compute luminance
float Luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)); // sRGB luminance coefficients
}

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Schlick's approximation for diffuse Fresnel term
float SchlickFresnel(float u)
{
    return pow(1.0 - u, 5.0);
}

// GGX Normal Distribution Function
float D_GGX(float NdotH, float alpha)
{
    float alpha2 = alpha * alpha;
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom + 1e-7); // Added epsilon to avoid division by zero
}

// Schlick-GGX Geometry function
float G_SchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k + 1e-7); // Added epsilon
}

// Smith's Geometry function
float G_Smith(float NdotV, float NdotL, float roughness)
{
    float G_V = G_SchlickGGX(NdotV, roughness);
    float G_L = G_SchlickGGX(NdotL, roughness);
    return G_V * G_L;
}

// Anisotropic GGX Normal Distribution Function
float D_AnisotropicGGX(float3 N, float3 H, float3 X, float3 Y, float alphaX, float alphaY)
{
    float NdotH = dot(N, H);
    float NdotH2 = NdotH * NdotH;

    float HdotX = dot(H, X);
    float HdotY = dot(H, Y);

    float HdotX_over_alphaX = HdotX / alphaX;
    float HdotY_over_alphaY = HdotY / alphaY;

    float exponent = HdotX_over_alphaX * HdotX_over_alphaX + HdotY_over_alphaY * HdotY_over_alphaY + NdotH2;

    float D = 1.0 / (PI * alphaX * alphaY * exponent * exponent + 1e-7); // Added epsilon

    return D;
}

// Disney BRDF function
float3 BRDF(float3 L, float3 V, float3 N, float3 X, float3 Y)
{
    // Compute H
    float3 H = normalize(V + L);

    // Compute dot products
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float LdotH = saturate(dot(L, H));
    float VdotH = saturate(dot(V, H));

    // Compute perceptual roughness
    float alpha = Roughness * Roughness;

    // Compute base reflectivity (F0)
    float3 baseColor = BaseColor.rgb;
    float luminance = max(Luminance(baseColor), 1e-4); // Avoid division by zero
    float3 cTint = baseColor / luminance;

    float3 dielectricSpecular = 0.08 * Specular * lerp(float3(1.0, 1.0, 1.0), cTint, SpecularTint);
    float3 F0 = lerp(dielectricSpecular, baseColor, Metallic);

    // Disney Diffuse term
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2.0 * Roughness * LdotH * LdotH;
    float Fd = (1.0 + (Fd90 - 1.0) * FL) * (1.0 + (Fd90 - 1.0) * FV);

    float3 diffuseTerm = (1.0 - Metallic) * baseColor * Fd / PI;
    float Fss90 = LdotH * LdotH * Roughness;
    float ss = 1.0 + (Fss90 - 1.0) * FL * FV;
    diffuseTerm *= lerp(1.0, ss, Subsurface);

    // Specular term with anisotropy
    float aspect = sqrt(1.0 - Anisotropic * 0.9);
    float alphaX = (Roughness * Roughness) / aspect;
    float alphaY = (Roughness * Roughness) * aspect;

    float D = D_AnisotropicGGX(N, H, X, Y, alphaX, alphaY);
    float G = G_Smith(NdotV, NdotL, Roughness);
    float3 F = FresnelSchlick(LdotH, F0);
    float3 specularTerm = D * G * F / (4.0 * NdotL * NdotV + 1e-7);

    // Sheen term
    float3 sheenColor = lerp(float3(1.0, 1.0, 1.0), cTint, SheenTint);
    float3 sheenTerm = Sheen * sheenColor * SchlickFresnel(LdotH);

    // Clearcoat term
    float clearcoatAlpha = lerp(0.1, 0.001, ClearcoatGloss);
    float D_clearcoat = D_GGX(NdotH, clearcoatAlpha);
    float F_clearcoat = FresnelSchlick(LdotH, float3(0.04, 0.04, 0.04));
    float G_clearcoat = G_Smith(NdotV, NdotL, 0.25);
    float clearcoatTerm = Clearcoat * D_clearcoat * F_clearcoat * G_clearcoat / (4.0 * NdotL * NdotV + 1e-7);

    // Combine terms
    float3 color = (diffuseTerm + specularTerm + sheenTerm + clearcoatTerm) * LightColor * NdotL;

    return color;
}

// Cook-Torrance BRDF function
float3 BRDF2(float3 L, float3 V, float3 N)
{
    // Half vector
    float3 H = normalize(L + V);

    // Fresnel-Schlick approximation
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), BaseColor.xyz, Metallic);
    float3 F = F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);

    // Geometry term (using Smith's Schlick-GGX approximation)
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float HdotN = saturate(dot(H, N));
    float k = pow(Roughness + 1.0, 2.0) / 8.0;
    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G = G_V * G_L;

    // GGX Normal Distribution Function (NDF)
    float alpha = Roughness * Roughness;
    float alpha2 = alpha * alpha;
    float denom = (HdotN * HdotN) * (alpha2 - 1.0) + 1.0;
    float D = alpha2 / (PI * denom * denom);

    // Specular reflection
    float3 specular = (F * G * D) / (4.0 * NdotV * NdotL + 0.001);

    // Lambertian diffuse reflection
    float3 kS = F;  // Specular reflectance
    float3 kD = 1.0 - kS;  // Diffuse reflectance (energy conservation)
    kD *= 1.0 - Metallic;  // Metallic materials have no diffuse reflection

    // Final shading: Diffuse + Specular
    float3 diffuse = kD * BaseColor.xyz / PI;
    return (diffuse + specular) * NdotL;
}

float4 main(SPSInput input) : SV_TARGET
{
    // Normalize normal and view vectors
    float3 N = normalize(input.Normal);
    float3 V = normalize(CameraWorldPosition - input.worldPosition);
    float3 L = normalize(LightPosition - input.worldPosition);

    // Get base color and material properties
    float3 X = normalize(input.Tangent);
    float3 Y = normalize(input.Bitangent);

    // Call the simplified PBR BRDF
//    float3 color = BRDF(L, V, N, X, Y);
    float3 color = BRDF2(L, V, N);

    // Return the final color with alpha
    return float4(color, BaseColor.a);
}
