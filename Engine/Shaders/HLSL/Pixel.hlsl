struct SPSInput
{
    float4 Position : SV_POSITION;  // Screen-space position (can be ignored for lighting calculations)
    float3 worldPosition : TEXCOORD3;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : TEXCOORD1;      // Normal vector in world space
    float3 Color : TEXCOORD2;       // Vertex color
};

float4 main(SPSInput PSInput) : SV_Target
{
    // Normalize the input normal vector without any bias
    float3 Normal = normalize(PSInput.Normal);

    // Point light position in world space
    float3 LightPosition = float3(-3.0f, -2.0f, 0.0f);

    // Calculate the direction from the fragment to the light
    float3 LightDirection = normalize(LightPosition - PSInput.worldPosition);

    // Diffuse lighting calculation
    float Diffuse = max(dot(Normal, LightDirection), 0.0f);

    // Attenuation calculation
    float constant = 1.0f;
    float Linear = 0.09f;
    float quadratic = 0.032f;

    float distance = length(LightPosition - PSInput.worldPosition);
    float attenuation = 1.0f / (constant + Linear * distance + quadratic * (distance * distance));

    // Final color calculation with attenuation and diffuse lighting
    float3 Color = PSInput.Color * Diffuse * attenuation;
    return float4(Color, 1.0f);
}
