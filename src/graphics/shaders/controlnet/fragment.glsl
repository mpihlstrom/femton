#version 410 core
//layout(location = 0, index = 0) out vec4 f_color;
layout(location = 0) out vec4 f_color;

uniform mat4 modelview;
uniform float line_width;
uniform float screen_scale_x;
uniform float screen_scale_y;
uniform float zoom;
uniform int W;
uniform int H;

in vec4 v_c;
in float v_w;
in vec3 v_v;

flat in vec3 v_v0;
flat in vec3 v_v1;
flat in vec3 v_v2;

in vec3 v_pos;

flat in vec3 v_ew;
flat in vec3 v_nw;

float min3(vec3 a3) {
    return min(min(a3.x, a3.y), a3.z);
}

float psin(vec2 v1, vec2 v2) {
    vec2 v1v2 = v2 - v1;
    vec2 v1p = vec2(v_pos) - v1;
    return (v1v2.x*v1p.y - v1v2.y*v1p.x) / length(v1v2);
}

void main(void) {
    //g_colors[0]*(p.x) + g_colors[1]*(p.y) + g_colors[2]*(p.z);
    f_color = v_c;
    f_color.a = 1;

    float sinab = psin(vec2(v_v0), vec2(v_v1));
    float sinbc = psin(vec2(v_v1), vec2(v_v2));
    float sinca = psin(vec2(v_v2), vec2(v_v0));

    float WH = float(W+H)/2;
    float lw = line_width / zoom * (1.0/((screen_scale_x+screen_scale_y)/2.0)) * 1/WH;

    bool dis = true;
    if((sinab < lw) || (sinbc < lw) || (sinca < lw)) dis = false;

    if(dis)
        discard;
}
