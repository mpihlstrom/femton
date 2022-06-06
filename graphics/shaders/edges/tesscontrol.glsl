#version 410 core

#define ID gl_InvocationID

layout(vertices = 3) out;

in vec4 v_c[];
in float v_w[];
out vec4 c_c[];
out float c_w[];

void main(void) {
    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
    c_c[ID] = v_c[ID];
    c_w[ID] = v_w[ID];

    int tess = 1;

    //only needs to be written once
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = tess;
        gl_TessLevelOuter[1] = tess;
        gl_TessLevelOuter[2] = tess;
        gl_TessLevelInner[0] = tess;
    }
}
