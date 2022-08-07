#version 420
//layout(binding=0) uniform sampler2D surface_buffer;
//layout(binding=1) uniform sampler2D wireframe_buffer;

uniform sampler2D surface_buffer;
uniform sampler2D wireframe_buffer;
uniform sampler2D control_points_buffer;
uniform sampler2D control_nets_buffer;

uniform sampler2D surface_depth_buffer;
uniform sampler2D control_points_depth_buffer;
uniform sampler2D control_net_depth_buffer;
uniform sampler2D wireframe_depth_buffer;

uniform int draw_surface;
uniform int draw_wireframe;
uniform int draw_control_points;
uniform int draw_control_net;
uniform vec3 bg_color;
in vec2 v_position;
in vec4 gl_FragCoord;

out vec4 f_color;

vec3 semi(vec3 c) {
    return vec3(
        c.r > 0.5? c.r - 0.5 : c.r + 0.5,
        c.g > 0.5? c.g - 0.5 : c.g + 0.5,
        c.b > 0.5? c.b - 0.5 : c.b + 0.5);
}

vec4 semi(vec4 c) {
    return vec4(semi(c.rgb), c.a);
}

vec4 h120(vec4 c) {
    return vec4(c.brg, c.a);
}

vec3 inv(vec3 c) {
    return vec3(1.0) -vec3(c);
}

vec4 inv(vec4 c) {
    return vec4(inv(c.rgb), c.a);
}


void main() {

    vec2 viewport_coord = (v_position + vec2(1.0,1.0))  / 2.0;
    f_color = vec4(bg_color, 1.0);

    if(draw_surface == 1) {
        vec4 c = texture(surface_buffer, viewport_coord);
        f_color = c*c.a + f_color*(1.0-c.a);
    }

    if(draw_control_net == 1) {
        vec4 db_surface = texture(surface_depth_buffer, viewport_coord);
        vec4 db_ctrl_wf = texture(control_net_depth_buffer, viewport_coord);
        vec4 c = texture(control_nets_buffer, viewport_coord);

        if(draw_surface == 0 || db_ctrl_wf.r < db_surface.r) {
            f_color = h120(inv(f_color))*c.a + f_color*(1.0-c.a);
        }
    }

    if(draw_wireframe == 1) {
        vec4 db_surface = texture(surface_depth_buffer, viewport_coord);
        vec4 db_wireframe = texture(wireframe_depth_buffer, viewport_coord);
        if(draw_surface == 0 || db_wireframe.r <= db_surface.r) {
            vec4 c = texture(wireframe_buffer, viewport_coord);
            f_color = c*c.a + f_color*(1.0-c.a);
        }
    }

    if(draw_control_points == 1) {
        vec4 cds = texture(surface_depth_buffer, viewport_coord);
        vec4 cdc = texture(control_points_depth_buffer, viewport_coord);
        vec4 c = texture(control_points_buffer, viewport_coord);

        if(draw_surface == 0 || cdc.r < cds.r) {
            f_color = semi(f_color)*c.a + f_color*(1.0-c.a);
        }
    }
}
