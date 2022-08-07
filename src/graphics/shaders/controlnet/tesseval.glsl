#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelview;
uniform mat4 projection;

in float c_w[];

out float e_w;

out vec3 e_patch_coord;

void main() {
    vec4 pos =
        gl_TessCoord.x * gl_in[0].gl_Position +
        gl_TessCoord.y * gl_in[1].gl_Position +
        gl_TessCoord.z * gl_in[2].gl_Position;

    gl_Position = projection * modelview * pos;
    e_patch_coord = gl_TessCoord;

    e_w = c_w[1] * (1-gl_TessCoord.x)
        + c_w[2] * (1-gl_TessCoord.y)
        + c_w[0] * (1-gl_TessCoord.z);
}
