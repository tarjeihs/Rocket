// Input structure
struct PSInput
{
    float3 inColor : COLOR;
};

// Output
float4 main(PSInput input) : SV_Target
{
    // Return the color with an alpha of 1.0
    return float4(input.inColor, 1.0f);
}
