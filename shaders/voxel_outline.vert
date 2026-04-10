#version 330 core
// =============================================================================
// voxel_outline.vert — Inverted-Hull Ink Outline Pass (OpenGL Pass 2)
//
// Render state: glCullFace(GL_FRONT)  (back faces rendered)
// Extrudes vertices along world-space normals by outlineThickness.
// =============================================================================

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

uniform mat4  modelViewProj;
uniform mat4  modelMatrix;
uniform float outlineThickness; // World-space extrusion distance (e.g. 0.04)

void main()
{
    vec3 worldPos    = (modelMatrix * vec4(inPosition, 1.0)).xyz;
    vec3 worldNormal = normalize(mat3(transpose(inverse(modelMatrix))) * inNormal);
    vec3 extruded    = worldPos + worldNormal * outlineThickness;

    gl_Position = modelViewProj * vec4(inPosition + inNormal * outlineThickness, 1.0);
}
