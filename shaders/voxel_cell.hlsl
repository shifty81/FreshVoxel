// =============================================================================
// voxel_cell.hlsl — Borderlands-style Cell Shading for DirectX 11
//
// Pass 1 of 2: Main toon-shaded draw call.
// Pass 2 (inverted-hull outline) is in voxel_outline.hlsl.
//
// Constant buffer layout:
//   b0 — MatrixBuffer       (MVP, model, view matrices)
//   b1 — CellShadingParams  (lighting, outline, palette colour)
// =============================================================================

// -----------------------------------------------------------------------------
// Constant Buffers
// -----------------------------------------------------------------------------

cbuffer MatrixBuffer : register(b0)
{
    matrix modelViewProj;
    matrix modelMatrix;
    matrix viewMatrix;
};

cbuffer CellShadingParams : register(b1)
{
    float3 voxelColor;       // Per-draw-call block colour from palette
    float  outlineThickness; // World-space extrusion for outline pass (e.g. 0.04)
    float3 lightDir;         // Directional light direction in world space (normalised)
    float  rimThreshold;     // Rim-light activation dot-product threshold (e.g. 0.6)
    float4 shadowColor;      // Deep-shadow RGBA tint (e.g. (0.05, 0.02, 0.10, 1.0))
};

// -----------------------------------------------------------------------------
// Vertex / Pixel structures
// -----------------------------------------------------------------------------

struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct PixelInput
{
    float4 position  : SV_POSITION;
    float3 normal    : NORMAL;
    float3 worldPos  : TEXCOORD0;
    float3 viewPos   : TEXCOORD1;
};

// -----------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(float4(input.position, 1.0f), modelViewProj);
    output.normal   = normalize(mul(float4(input.normal, 0.0f), modelMatrix).xyz);
    output.worldPos = mul(float4(input.position, 1.0f), modelMatrix).xyz;
    // Camera is at the origin in view space
    output.viewPos  = -mul(float4(0.0f, 0.0f, 0.0f, 1.0f), viewMatrix).xyz;
    return output;
}

// -----------------------------------------------------------------------------
// Pixel Shader — toon lighting with 4 discrete bands + rim highlight
// -----------------------------------------------------------------------------

float4 PSMain(PixelInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightDir);
    float3 V = normalize(input.viewPos - input.worldPos);

    // ---- Quantised diffuse (4 bands) ----
    float NdotL = saturate(dot(N, L));
    float toonBand;
    if      (NdotL > 0.75f) toonBand = 1.00f;  // full lit
    else if (NdotL > 0.45f) toonBand = 0.65f;  // mid-lit
    else if (NdotL > 0.20f) toonBand = 0.35f;  // shadow
    else                    toonBand = 0.12f;  // deep shadow

    // ---- Rim / Fresnel highlight ----
    float rimDot  = 1.0f - saturate(dot(V, N));
    float rimBand = step(rimThreshold, rimDot) * 0.35f;

    // ---- Colour composition ----
    // Interpolate from shadowColor (darks) toward voxelColor * toon factor (lights)
    float3 litColor  = voxelColor * (toonBand + rimBand);
    float3 darkColor = shadowColor.rgb;
    float  blend     = smoothstep(0.0f, 0.3f, toonBand);
    float3 finalColor = lerp(darkColor, litColor, blend);

    return float4(finalColor, 1.0f);
}
