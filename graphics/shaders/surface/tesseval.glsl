#version 410 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec3 cps[];
uniform float z_units;
uniform int is_surface2;

in uint c_id[];
in vec4 c_color[];
in vec4 c_color2[];
in vec3 c_b021[], c_b012[], c_b210[], c_b120[], c_b102[], c_b201[], c_b111[];
in float c_weight[];

out uint e_id;
out vec4 e_color;
out vec4 e_color2;
out vec4 e_corner_color;
out float e_weight;
out vec3 e_pos;

struct ControlPoints
{
    vec3 b300, b210, b120, b030, b021, b012, b003, b102, b201, b111;
};

out vec3 e_patch_coord;
out vec3 e_patch_distance;


vec3 lerp_normal(vec3 u) {
    ControlPoints ctrl;
    ctrl.b300 = gl_in[0].gl_Position.xyz;
    ctrl.b210 = c_b210[0];
    ctrl.b120 = c_b120[0];
    ctrl.b030 = gl_in[1].gl_Position.xyz;
    ctrl.b021 = c_b021[0];
    ctrl.b012 = c_b012[0];
    ctrl.b003 = gl_in[2].gl_Position.xyz;
    ctrl.b102 = c_b102[0];
    ctrl.b201 = c_b201[0];
    ctrl.b111 = c_b111[0];

    //degree1
    vec3 b300b210b201 = ctrl.b300*u.x + ctrl.b210*u.y + ctrl.b201*u.z;
    vec3 b120b030b021 = ctrl.b120*u.x + ctrl.b030*u.y + ctrl.b021*u.z;
    vec3 b102b012b003 = ctrl.b102*u.x + ctrl.b012*u.y + ctrl.b003*u.z;

    vec3 b210b120b111 = ctrl.b210*u.x + ctrl.b120*u.y + ctrl.b111*u.z;
    vec3 b111b021b012 = ctrl.b111*u.x + ctrl.b021*u.y + ctrl.b012*u.z;
    vec3 b201b111b102 = ctrl.b201*u.x + ctrl.b111*u.y + ctrl.b102*u.z;

    //degree 2
    vec3 bbb1 = b300b210b201*u.x + b210b120b111*u.y + b201b111b102*u.z;
    vec3 bbb2 = b210b120b111*u.x + b120b030b021*u.y + b111b021b012*u.z;
    vec3 bbb3 = b201b111b102*u.x + b111b021b012*u.y + b102b012b003*u.z;

    //degree 3
    //p = bbb1*u.u + bbb2*u.v + bbb3*u.w;

    vec3 r = vec3(bbb2-bbb1);
    vec3 s = vec3(bbb3-bbb1);
    return vec3(r.y*s.z - r.z*s.y, r.z*s.x - r.x*s.z, r.x*s.y - r.y*s.x);
}

vec3 bezier_triangle(vec3 u3) {
    float u = u3.x;
    float v = u3.y;
    float w = u3.z;

    return u*gl_in[0].gl_Position.xyz +
           v*gl_in[1].gl_Position.xyz +
           w*gl_in[2].gl_Position.xyz;
    /*
    ControlPoints ctrl;
    ctrl.b300 = gl_in[0].gl_Position.xyz;
    ctrl.b210 = c_b210[0];
    ctrl.b120 = c_b120[0];
    ctrl.b030 = gl_in[1].gl_Position.xyz;
    ctrl.b021 = c_b021[0];
    ctrl.b012 = c_b012[0];
    ctrl.b003 = gl_in[2].gl_Position.xyz;
    ctrl.b102 = c_b102[0];
    ctrl.b201 = c_b201[0];
    ctrl.b111 = c_b111[0];

    float u_pow3 = pow(u, 3);
    float v_pow3 = pow(v, 3);
    float w_pow3 = pow(w, 3);
    float u_pow2 = pow(u, 2);
    float v_pow2 = pow(v, 2);
    float w_pow2 = pow(w, 2);

    return
        ctrl.b300 * w_pow3 +
        ctrl.b030 * u_pow3 +
        ctrl.b003 * v_pow3 +
        ctrl.b210 * 3.0 * w_pow2 * u +
        ctrl.b120 * 3.0 * w * u_pow2 +
        ctrl.b201 * 3.0 * w_pow2 * v +
        ctrl.b021 * 3.0 * u_pow2 * v +
        ctrl.b102 * 3.0 * w * v_pow2 +
        ctrl.b012 * 3.0 * u * v_pow2 +
        ctrl.b111 * 6.0 * w * u * v;
*/
}

void main() {

    e_id = c_id[0];

    vec3 pos = bezier_triangle(gl_TessCoord);

    gl_Position = projection * modelview * vec4(pos, 1.0);
    float i = pos.z / z_units;
    e_color = vec4(i, i, i, 1.0) + vec4(vec3(0.5),0.0);

    // I do not understand this index order.
    e_corner_color = c_color[1] * gl_TessCoord.x + c_color[2] * gl_TessCoord.y + c_color[0] * gl_TessCoord.z;
    e_weight = c_weight[1] * gl_TessCoord.x + c_weight[2] * gl_TessCoord.y + c_weight[0] * gl_TessCoord.z;

    float az = gl_in[0].gl_Position.z;
    float bz = gl_in[1].gl_Position.z;
    float cz = gl_in[2].gl_Position.z;


    vec4 color1[3];
    vec4 color2[3];
    vec3 u = gl_TessCoord.xyz;

    e_color  = color1[1] * u.x + color1[2] * u.y + color1[0] * u.z;
    e_color2 = color2[1] * u.x + color2[2] * u.y + color2[0] * u.z;

    e_patch_distance = gl_TessCoord;
    e_patch_coord = gl_TessCoord;

    vec3 pos0 =
        gl_in[1].gl_Position.xyz * gl_TessCoord.x +
        gl_in[2].gl_Position.xyz * gl_TessCoord.y +
        gl_in[0].gl_Position.xyz * gl_TessCoord.z;

    e_pos = vec3(projection * modelview * vec4(pos0, 1));
}
