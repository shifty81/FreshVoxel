#version 330 core
// =============================================================================
// voxel_cell.vert — Borderlands-style Cell Shading (OpenGL Pass 1)
// =============================================================================

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 fragNormal;
out vec3 fragPos;
out vec3 fragViewDir;

uniform mat4 modelViewProj;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;

void main()
{
    vec4 worldPos = modelMatrix * vec4(inPosition, 1.0);
    gl_Position   = modelViewProj * vec4(inPosition, 1.0);

    fragNormal  = normalize(mat3(transpose(inverse(modelMatrix))) * inNormal);
    fragPos     = worldPos.xyz;
    fragViewDir = normalize(cameraPos - worldPos.xyz);
}
