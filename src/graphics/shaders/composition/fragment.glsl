#version 410 core
layout(location = 1) out uint f_id;
layout(location = 0) out vec4 f_color;

uniform sampler2D  surface_buffer;
uniform sampler2D  surface_depth_buffer;
uniform usampler2D surface_id_buffer;
uniform sampler2D  surface_color2_buffer;

uniform sampler2D wireframe_buffer;
uniform sampler2D wireframe_depth_buffer;

uniform sampler2D  control_points_buffer;
uniform sampler2D  control_points_depth_buffer;
uniform usampler2D control_points_id_buffer;

uniform sampler2D control_net_buffer;
uniform sampler2D control_net_depth_buffer;

uniform sampler2D normals_buffer;
uniform sampler2D normals_depth_buffer;
uniform usampler2D normals_id_buffer;

uniform sampler2D  edges_buffer;
uniform sampler2D  edges_depth_buffer;
uniform usampler2D edges_id_buffer;

uniform sampler2D  nodes_buffer;
uniform sampler2D  nodes_depth_buffer;
uniform usampler2D nodes_id_buffer;

uniform sampler2D contour_buffer;
uniform sampler2D contour_depth_buffer;

uniform sampler2D gaussian_v_buffer;
uniform sampler2D gaussian_v_color2_buffer;

uniform sampler2D contour_v_buffer;
uniform sampler2D painting_buffer;

uniform int draw_surface;
uniform int draw_wireframe;
uniform int draw_control_points;
uniform int draw_control_net;
uniform int draw_normals;
uniform int draw_edges;
uniform int draw_nodes;
uniform int draw_painting;

uniform int draw_cursor;
uniform int draw_flat;

uniform int draw_gaussian_v;

uniform vec2 cursor;
uniform vec3 bg_color;
uniform int render_counter;

uniform float cursor_radius;

uniform float line_width;

uniform int H;
uniform int W;

uniform float zoom;

uniform float screen_scale_x;
uniform float screen_scale_y;

uniform float gaussian_scale;
uniform float contrast_scale;

uniform int mousetool;
uniform int mouse_left;
uniform int mouse_right;


in vec2 v_position;
in vec4 gl_FragCoord;

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


// Gold Noise ©2017-2018 dcerisano@standard3d.com
//  - based on the Golden Ratio, PI and Square Root of Two
//  - fastest noise generator function
//  - works with all chipsets (including low precision)
//precision lowp    float;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed){
    return fract(sin(dot(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 rgb_noise(vec2 coord, int seed) {
    float r = gold_noise(coord, seed), g = gold_noise(coord,r), b = gold_noise(coord,g);
    return vec3(r,g,b);
}

float c_dist(vec4 c) {
    //vec4 w = vec4(0.21267, 0.71516, 0.07217, 1);
    vec4 w = vec4(1);
    vec4 wc = w*c;
    return dot(wc,wc);
}

float vsin(vec2 v1, vec2 v2) {
    return v1.x*v2.y - v1.y*v2.x;
}

void main() {

    vec2 viewport_coord = (v_position + vec2(1.0,1.0))  / 2.0;

    bool halfplane_h = viewport_coord.x > 0.5;
    float h = 1.0-viewport_coord.y;
    float v = 1.0-viewport_coord.x;
    float h_a = 1.0 / (1.0 + exp(-40*(h - 0.5)));
    float v_a = 1.0 / (1.0 + exp(-40*(v - 0.5)));

    vec4 bgcolora = vec4(vec3(1),1);//vec4(bg_color, 1);
    f_color = bgcolora;

    vec2 p = vec2(viewport_coord - vec2(0.5, 0.5)) * 2.0;
    float r = gold_noise(p,render_counter+100), g = gold_noise(p,r), b = gold_noise(p,g);
    float al = 0.1;
    //f_color = vec4(r,g,b, 1)*al + vec4(vec3(0.5),1)*(1.0-al);



    if(draw_surface == 1)
    {
        f_id = texture(surface_id_buffer, viewport_coord).r;
        if(f_id != 0) {
            vec4 col;
            if(draw_gaussian_v ==  0)  col = texture(surface_buffer, viewport_coord);
            //else if(draw_painting == 1) col = texture(painting_buffer, viewport_coord);
            else col = texture(gaussian_v_buffer, viewport_coord);
            if(draw_painting == 1)
                f_color = col*h_a + f_color*(1.0-h_a);
            else
                f_color = col;
        }
        //f_color = texture(gaussian_v_buffer, viewport_coord); f_color.a = 1;
    }

    if(draw_control_net == 1)
    {
        vec4 c0 = texture(control_net_buffer, viewport_coord);
        float q = 0.8;
        vec4 c = f_color*q + inv(f_color)*(1.0-q);//inv(f_color);
        c.a = c0.a;
        float al = c.a;
        //f_color = semi(f_color)*al +  f_color*(1.0-al);//semi(f_color)*c.a + f_color*(1.0-c.a);
        f_color = c*al + f_color*(1.0-al);
    }

    vec4 before_edges = f_color;

    if(draw_edges == 1)
    {
        vec4 c = texture(edges_buffer, viewport_coord);
        f_id = texture(edges_id_buffer, viewport_coord).r;
        vec4 col = vec4(vec3(0),1);//semi(f_color);//
        if(draw_painting == 1)
            col = col*(1.0-h_a) + f_color*h_a;
        f_color = col*c.a + f_color*(1.0-c.a);
    }

    if(draw_nodes == 1) {
        vec4 col = texture(nodes_buffer, viewport_coord);
        //f_color = h120((f_color))*col.a + f_color*(1.0-col.a);
        f_color = vec4(vec3(0),1)*col.a + f_color*(1.0-col.a);

        //float al = col.a;
        //f_color = col*al + f_color*(1.0-al);
        if(col.a != 0) {
            f_id = texture(nodes_id_buffer, viewport_coord).r;
        }
    }

    if(draw_cursor == 1) {
        vec2 p = vec2(vec2(viewport_coord.xy) - vec2(0.5, 0.5)) * 2.0;
        vec2 cur = cursor;// / 2 + vec2(0.5,0.5);
        float len = length(p - cur);
        float r = cursor_radius;
        float r2 = 0.002;
        float w = 1.0/((W+H)/2); //pixel width
        int draw = 0;
        draw |= int(len < r+w && len > r-w);
        draw |= int(len < r2+w && len > r2-w);
        float draw_al = 1;

        if(mousetool == 1) {
            if(mouse_left == 1 && mouse_right == 1)
                draw |= int(abs(length(p) - length(cur)) < w);
            else if(mouse_right == 1) {
                //draw |= int(abs(vsin(normalize(cur),p)) < w && dot(normalize(cur), normalize(p)) >= 0 && length(p) <= length(cur));
                draw |= int(abs(vsin(normalize(cur),p)) < w && dot(normalize(cur), normalize(p)) >= 0 && length(p) <= length(cur));
            }

        }

        if(draw == 1) {
            float r = gold_noise(p,render_counter), g = gold_noise(p,r), b = gold_noise(p,g);
            f_color = vec4(r,g,b,1)*draw_al + f_color*(1-draw_al);
        }
    }
}
