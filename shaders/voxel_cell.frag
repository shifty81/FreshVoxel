#version 330 core
// =============================================================================
// voxel_cell.frag — Borderlands-style Cell Shading (OpenGL Pass 1)
//
// 4-band quantised diffuse + rim highlight + shadow colour blending.
// =============================================================================

in vec3 fragNormal;
in vec3 fragPos;
in vec3 fragViewDir;

out vec4 outColor;

uniform vec3  voxelColor;       // Per-draw-call block colour from palette
uniform vec3  lightDir;         // Normalised world-space light direction
uniform float rimThreshold;     // Rim-light activation threshold (default 0.6)
uniform vec4  shadowColor;      // Deep-shadow RGBA colour

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    vec3 V = normalize(fragViewDir);

    // ---- 4-band toon diffuse ----
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float toonBand;
    if      (NdotL > 0.75) toonBand = 1.00;
    else if (NdotL > 0.45) toonBand = 0.65;
    else if (NdotL > 0.20) toonBand = 0.35;
    else                   toonBand = 0.12;

    // ---- Rim highlight ----
    float rimDot  = 1.0 - clamp(dot(V, N), 0.0, 1.0);
    float rimBand = step(rimThreshold, rimDot) * 0.35;

    // ---- Final colour ----
    vec3 litColor  = voxelColor * (toonBand + rimBand);
    float blend    = smoothstep(0.0, 0.3, toonBand);
    vec3 finalColor = mix(shadowColor.rgb, litColor, blend);

    outColor = vec4(finalColor, 1.0);
}
