#version 410 core

layout(location = 0, index = 0) out vec4 f_color;

in vec3 g_patch_coord;
in vec3 g_primitive_coord;
in vec4 g_color;
in vec4 g_corner_color;

uniform vec3 wireframe_color;
uniform float line_width;

float edgeFactor() {
    vec3 d = fwidth(g_primitive_coord);
    vec3 a3 = smoothstep(vec3(0), d * 40 * line_width /* / modelview[3][3]*/, g_primitive_coord);
    return min(min(a3.x, a3.y), a3.z);
}

void main(void) {
    f_color = vec4(0);
    f_color.a = 1.0-edgeFactor();
    if(f_color.a <= 0) discard;
}
