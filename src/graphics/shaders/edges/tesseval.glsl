#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelview;
uniform mat4 projection;

in vec4 c_c[];
in float c_w[];

out vec4 e_c;
out float e_w;

out vec3 e_patch_coord;

out vec4 e_c0;
out vec4 e_c1;
out vec4 e_c2;
out float e_w0;
out float e_w1;
out float e_w2;

void main() {
    vec4 pos =
        gl_TessCoord.x * gl_in[0].gl_Position +
        gl_TessCoord.y * gl_in[1].gl_Position +
        gl_TessCoord.z * gl_in[2].gl_Position;

    gl_Position = projection * modelview * pos;
    e_patch_coord = gl_TessCoord;

    e_c0 = c_c[0]; e_c1 = c_c[1]; e_c2 = c_c[2];
    e_w0 = c_w[0]; e_w1 = c_w[1]; e_w2 = c_w[2];

    e_c = c_c[0] * (gl_TessCoord.x)
        + c_c[1] * (gl_TessCoord.y)
        + c_c[2] * (gl_TessCoord.z);

    e_w = c_w[0] * (gl_TessCoord.x)
        + c_w[1] * (gl_TessCoord.y)
        + c_w[2] * (gl_TessCoord.z);
}
