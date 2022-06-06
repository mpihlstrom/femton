#version 410 core

#define ID gl_InvocationID

layout(vertices = 3) out;

uniform int tessellation_level;

in uint v_id[];
in vec4 v_color[];
in vec4 v_color2[];
in vec3 v_b021[], v_b012[], v_b210[], v_b120[], v_b102[], v_b201[], v_b111[];
in float v_weight[];

out uint c_id[];
out vec4 c_color[];
out vec4 c_color2[];
out vec3 c_b021[], c_b012[], c_b210[], c_b120[], c_b102[], c_b201[], c_b111[];
out float c_weight[];

void main(void) {

    c_id[ID] = v_id[ID];
    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
    c_color[ID] = v_color[ID];
    c_color2[ID] = v_color2[ID];
    c_weight[ID] = v_weight[ID];

    //perform only for one index?
    c_b021[ID] = v_b021[ID];
    c_b012[ID] = v_b012[ID];
    c_b210[ID] = v_b210[ID];
    c_b120[ID] = v_b120[ID];
    c_b102[ID] = v_b102[ID];
    c_b201[ID] = v_b201[ID];
    c_b111[ID] = v_b111[ID];

    //only needs to be written once
    if(gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
        gl_TessLevelInner[0] = 1;
    }
}
