#version 410 core

uniform mat4 modelview;
uniform mat4 projection;

in uint id;

in vec3 position;
in vec4 color;
in uint l;
in vec3 v0;
in vec3 v1;
in vec3 v2;

in vec3 ew;
in vec3 nw;

flat out uint v_id;

out vec4 v_c;
out vec3 v_v;
flat out vec3 v_v0;
flat out vec3 v_v1;
flat out vec3 v_v2;
out vec3 v_pos;

flat out vec3 v_ew;
flat out vec3 v_nw;

void main(void) {

    v_id = id;
    gl_Position = projection * modelview * vec4(position, 1.0);
    v_c = color;
    v_v = l == 0? vec3(1,0,0) : l == 1? vec3(0,1,0) : vec3(0,0,1);

    v_v0 = vec3(projection * modelview * vec4(v0, 1.0));
    v_v1 = vec3(projection * modelview * vec4(v1, 1.0));
    v_v2 = vec3(projection * modelview * vec4(v2, 1.0));
    v_pos = vec3(projection * modelview * vec4(position, 1.0));

    v_ew = ew;
    v_nw = nw;
}
