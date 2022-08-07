#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;

in vec3 e_patch_coord[3];
in float e_w[3];

out vec3 g_primitive_coord;
out vec3 g_patch_coord;

out float g_w;

void main(void) {
    gl_Position = gl_in[0].gl_Position;
    g_patch_coord = e_patch_coord[0];
    g_primitive_coord = vec3(1, 0, 0);
    g_w = e_w[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    g_patch_coord = e_patch_coord[1];
    g_primitive_coord = vec3(0, 1, 0);
    g_w = e_w[1];
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    g_patch_coord = e_patch_coord[2];
    g_primitive_coord = vec3(0, 0, 1);
    g_w = e_w[2];
    EmitVertex();

    EndPrimitive();
}
