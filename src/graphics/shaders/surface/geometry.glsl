#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;

in uint e_id[];
in vec4 e_color[];
in vec4 e_color2[];
in vec4 e_corner_color[];
in vec3 e_patch_coord[3];
in vec3 e_patch_distance[3];
in vec3 e_pos[];
in float e_weight[];

flat out uint g_id;
out vec3 g_primitive_coord;
out vec3 g_patch_coord;
out vec4 g_color;
out vec4 g_color2;
out vec4 g_corner_color;
out float g_weight;

void main(void) {

    g_id = e_id[0];
    gl_Position = gl_in[0].gl_Position;
    g_patch_coord = e_patch_coord[0];
    g_primitive_coord = vec3(1, 0, 0);
    g_color = e_color[0];
    g_color2 = e_color2[0];
    g_corner_color = e_corner_color[0];   
    g_weight = e_weight[0];
    EmitVertex();

    g_id = e_id[1];
    gl_Position = gl_in[1].gl_Position;
    g_patch_coord = e_patch_coord[1];
    g_primitive_coord = vec3(0, 1, 0);
    g_corner_color = e_corner_color[1];
    g_color = e_color[1];
    g_color2 = e_color2[1];
    g_weight = e_weight[1];
    EmitVertex();

    g_id = e_id[2];
    gl_Position = gl_in[2].gl_Position;
    g_patch_coord = e_patch_coord[2];
    g_primitive_coord = vec3(0, 0, 1);
    g_corner_color = e_corner_color[2];
    g_color = e_color[2];
    g_color2 = e_color2[2];
    g_weight = e_weight[2];
    EmitVertex();

    EndPrimitive();
}
