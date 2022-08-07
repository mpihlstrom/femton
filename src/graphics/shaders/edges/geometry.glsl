#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;

in vec3 e_patch_coord[3];
in vec4 e_c[3];
in float e_w[3];

in vec4 e_c0[3];
in vec4 e_c1[3];
in vec4 e_c2[3];
in float e_w0[3];
in float e_w1[3];
in float e_w2[3];

out vec3 g_primitive_coord;
out vec3 g_patch_coord;
out vec4 g_c;
out float g_w;
out vec4 g_colors[3];
out float g_weights[3];
out vec4 g_vert_pos[3];

void common_out() {
    g_vert_pos[0] = gl_in[0].gl_Position;//gl_in[0].gl_Position;
    g_vert_pos[1] = gl_in[1].gl_Position;
    g_vert_pos[2] = gl_in[2].gl_Position;
    g_colors[0] = e_c0[0];
    g_colors[1] = e_c1[0];
    g_colors[2] = e_c2[0];
    g_weights[0] = e_w0[0];
    g_weights[1] = e_w1[0];
    g_weights[2] = e_w2[0];
}

void main(void) {
    gl_Position = gl_in[0].gl_Position;
    g_patch_coord = e_patch_coord[0];
    g_primitive_coord = vec3(1, 0, 0);
    g_c = e_c[0];
    g_w = e_w[0];
    common_out();
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    g_patch_coord = e_patch_coord[1];
    g_primitive_coord = vec3(0, 1, 0);
    g_c = e_c[1];
    g_w = e_w[1];
    common_out();
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    g_patch_coord = e_patch_coord[2];
    g_primitive_coord = vec3(0, 0, 1);
    g_c = e_c[2];
    g_w = e_w[2];
    common_out();
    EmitVertex();

    EndPrimitive();
}
