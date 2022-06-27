#version 410 core

uniform mat4 modelview;
uniform mat4 projection;

in vec3 position;
in uint id;

flat out uint v_id;

void main(void) {
    gl_Position = projection * modelview * vec4(position, 1.0);
    v_id = id;
}
