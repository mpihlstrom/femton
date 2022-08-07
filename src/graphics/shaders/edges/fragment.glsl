#version 410 core

//layout(location = 0, index = 0) out vec4 f_color;
layout(location = 2) out vec2 f_coordinate;
layout(location = 1) out uint f_id;
layout(location = 0) out vec4 f_color;

uniform mat4 modelview;
uniform float line_width;
uniform int W;
uniform int H;
uniform float screen_scale_x;
uniform float screen_scale_y;
uniform float zoom;

in vec4 v_c;
in float v_w;
in vec3 v_v;

flat in vec3 v_v0;
flat in vec3 v_v1;
flat in vec3 v_v2;
in vec3 v_pos;

flat in vec3 v_ew;
flat in vec3 v_nw;

flat in uint v_id;


float min3(vec3 a3) {
    return min(min(a3.x, a3.y), a3.z);
}

int minlabel(vec3 a3) {
    if(a3.x < a3.y && a3.x < a3.z) return 0;
    if(a3.y < a3.z && a3.y < a3.x) return 1;
    if(a3.z < a3.x && a3.z < a3.y) return 2;
}

float psin(vec2 v1, vec2 v2) {
    vec2 v1v2 = v2 - v1;
    vec2 v1p = vec2(v_pos) - v1;
    return (v1v2.x*v1p.y - v1v2.y*v1p.x) / length(v1v2);
}

float pdist(vec2 v) {
    vec2 vp = vec2(v_pos) - v;
    return length(vp);
}

void main(void) {
    f_id = v_id;
    f_coordinate = vec2(v_v.xy);
    f_color = v_c;

    float sinab = psin(vec2(v_v0), vec2(v_v1));
    float sinbc = psin(vec2(v_v1), vec2(v_v2));
    float sinca = psin(vec2(v_v2), vec2(v_v0));

    float WH = float(W+H)/2;
    float lw = 1.0 * line_width / zoom * (1.0/((screen_scale_x+screen_scale_y)/2.0)) * 1/WH;
    //float lw = 4.0 / zoom * (1.0/((screen_scale_x+screen_scale_y)/2.0)) * 1/WH;

    vec3 sins = vec3(sinab, sinbc, sinca);
    float min = minlabel(sins);

    bool dis = true;
    /*
    if(v_ew.x > 0 && sinab < lw) dis = false;
    if(v_ew.y > 0 && sinbc < lw) dis = false;
    if(v_ew.z > 0 && sinca < lw) dis = false;
    */

    vec2 ab = vec2(v_v1 - v_v0), bc = vec2(v_v2 - v_v1), ca = vec2(v_v0 - v_v2);
    vec2 av = vec2(v_pos - v_v0), bv = vec2(v_pos - v_v1), cv = vec2(v_pos - v_v2);
    vec2 ab_u = normalize(ab), bc_u = normalize(bc), ca_u = normalize(ca), av_u = normalize(av), bv_u = normalize(bv), cv_u = normalize(cv);
    if((v_ew.x > 0 && sinab < lw) && dot(ab_u, av_u) > 0 && dot(bv_u, -ab_u) > 0) dis = false;
    if((v_ew.y > 0 && sinbc < lw) && dot(bc_u, bv_u) > 0 && dot(cv_u, -bc_u) > 0) dis = false;
    if((v_ew.z > 0 && sinca < lw) && dot(ca_u, cv_u) > 0 && dot(av_u, -ca_u) > 0) dis = false;

    /*
    if(v_nw.x > 0 && pdist(vec2(v_v0)) < lw) dis = false;
    if(v_nw.y > 0 && pdist(vec2(v_v1)) < lw) dis = false;
    if(v_nw.z > 0 && pdist(vec2(v_v2)) < lw) dis = false;
    */

    if(dis)
        f_color.a = 0;
}
