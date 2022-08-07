#version 410 core

in vec2 position;
//in vec2 texcoord;
out vec2 v_position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    v_position = position;
}
