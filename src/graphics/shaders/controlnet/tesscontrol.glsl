#version 410 core

#define ID gl_InvocationID

layout(vertices = 3) out;

in float v_w[];
out float c_w[];

void main(void) {
    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
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
