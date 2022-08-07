#version 410 core

uniform mat4 modelview;
uniform mat4 projection;
uniform float point_size;

in vec3 position;
in uint id;
in float weight;
in float contrast;
in vec4 nc;
in vec2 g;

flat out uint v_id;
flat out float v_w;
flat out vec4 v_c;
flat out vec2 v_g;

void main(void) {
    gl_Position = projection * modelview * vec4(position, 1.0);
    //gl_PointSize = (0.1+contrast)*point_size;// / modelview[3][3];
    gl_PointSize = 1*point_size;// / modelview[3][3];
    v_id = id;
    v_w = weight;
    //v_c = contrast;
    v_c = nc;
    v_g = g;
}
