#version 410 core

layout(location = 3) out vec4 f_color2;
layout(location = 2) out vec2 f_coordinate;
layout(location = 1) out uint f_id;
layout(location = 0) out vec4 f_color;

uniform int selected;
uniform vec3 wireframe_color;

flat in uint v_id;

in vec3 v_v;
in vec4 v_tc;
in float v_w;
in float v_ctrst;
in vec2 v_g;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 semi(vec3 c) {
    return vec3(
        c.r > 0.5? c.r - 0.5 : c.r + 0.5,
        c.g > 0.5? c.g - 0.5 : c.g + 0.5,
        c.b > 0.5? c.b - 0.5 : c.b + 0.5);
}

vec4 semi(vec4 c) {
    return vec4(semi(c.rgb), c.a);
}

vec4 h120(vec4 c) {
    return vec4(c.brg, c.a);
}

vec3 inv(vec3 c) {
    return vec3(1.0) -vec3(c);
}

void main(void) {
    f_id = v_id;
    f_color = v_tc;
    //f_color.a += 0.0001;
    f_coordinate = vec2(v_v);

    //use channels for different parameters
    f_color2 = vec4(v_w, v_ctrst, v_g.x, v_g.y);
}
