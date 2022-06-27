#version 410 core

uniform mat4 modelview;
uniform mat4 projection;

in uint id;
in vec3 position;
in uint l;
in vec4 tc;
in float weight;
in float contrast;
in vec2 g;

flat out uint v_id;
out vec3 v_v;
out vec4 v_tc;
out float v_w;
out float v_ctrst;
out vec2 v_g;

void main(void) {
    v_id = id;
    gl_Position = projection * modelview * vec4(position, 1.0);
    v_v = l == 0? vec3(1,0,0) : l == 1? vec3(0,1,0) : vec3(0,0,1);
    v_tc = tc;
    v_w = weight;
    v_g = g;
    v_ctrst = contrast;
}
