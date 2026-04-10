#version 330 core
// =============================================================================
// voxel_outline.frag — Inverted-Hull Ink Outline Pass (OpenGL Pass 2)
//
// Outputs flat black for the Borderlands ink-outline look.
// =============================================================================

out vec4 outColor;

void main()
{
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
