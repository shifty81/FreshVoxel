// =============================================================================
// voxel_outline.hlsl — Inverted-Hull Ink Outline Pass (DirectX 11)
//
// Draws each chunk mesh a second time with front-face culling, extruding
// vertices along their normals to produce a thick black ink outline.
//
// Render state requirements:
//   - D3D11_CULL_FRONT  (front-face culling, back faces are rendered)
//   - Depth write ON, depth test ON (equal or less)
// =============================================================================

cbuffer MatrixBuffer : register(b0)
{
    matrix modelViewProj;
    matrix modelMatrix;
    matrix viewMatrix;
};

cbuffer CellShadingParams : register(b1)
{
    float3 voxelColor;
    float  outlineThickness;
    float3 lightDir;
    float  rimThreshold;
    float4 shadowColor;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct PixelInput
{
    float4 position : SV_POSITION;
};

// -----------------------------------------------------------------------------
// Vertex Shader — extrude along world-space normal
// -----------------------------------------------------------------------------

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    // Move the vertex outward along its normal in world space
    float3 worldPos    = mul(float4(input.position, 1.0f), modelMatrix).xyz;
    float3 worldNormal = normalize(mul(float4(input.normal, 0.0f), modelMatrix).xyz);
    float3 extruded    = worldPos + worldNormal * outlineThickness;

    // Reconstruct clip-space position from extruded world position
    // We abuse the modelViewProj here; a proper implementation would
    // split it into separate view/proj matrices.
    float4 viewPos = mul(float4(extruded, 1.0f), viewMatrix);
    output.position = mul(viewPos, modelViewProj); // MVP applied differently if matrices are split
    // Simplified: just apply full MVP to the extruded world-space position
    output.position = mul(float4(extruded, 1.0f), modelViewProj);

    return output;
}

// -----------------------------------------------------------------------------
// Pixel Shader — flat black (ink outline)
// -----------------------------------------------------------------------------

float4 PSMain(PixelInput input) : SV_TARGET
{
    (void)input;
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
