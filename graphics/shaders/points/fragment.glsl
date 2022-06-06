#version 410 core

layout(location = 0, index = 0) out vec4 f_color;
layout(location = 1) out uint f_id;

flat in uint v_id;
flat in float v_w;
flat in vec4 v_c;
flat in vec2 v_g;

void main(void) {
    f_id = v_id;
    if(length(gl_PointCoord.xy - vec2(.5)) > 0.5)
        discard;
    //f_color = vec4(vec3(1.),pow(length(gl_PointCoord.xy),2.));
    f_color = v_c;
    //f_color = vec4(v_w);
    //f_color.xy = v_g; //f_color.b = v_w;
    f_color.a = 1.0;
}
