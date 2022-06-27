#version 410 core

layout(location = 0, index = 0) out vec4 f_color;
layout(location = 1) out uint f_id;

flat in uint v_id;

void main(void) {
    f_id = v_id;
    f_color = vec4(1.);
}
