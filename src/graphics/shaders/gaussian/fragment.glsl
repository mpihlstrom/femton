#version 410 core
layout(location = 3) out vec4 f_color2;
layout(location = 2) out vec2 f_coordinate;
layout(location = 1) out uint f_id;
layout(location = 0) out vec4 f_color;

uniform sampler2D in_buffer;
uniform sampler2D in_color2_buffer;
uniform sampler2D surface_color2_buffer;
uniform usampler2D surface_id_buffer;

uniform int H;
uniform int W;

uniform int dir;

uniform float zoom;
uniform float screen_scale_x;
uniform float screen_scale_y;

uniform float gaussian_size;
uniform float min_gaussian_q;

in vec2 v_position;
in vec4 gl_FragCoord;

//gaussian code from: https://dsp.stackexchange.com/questions/23460/how-to-calculate-gaussian-kernel-for-a-small-support-size

void main() {   
    vec2 viewport_coord = (v_position + vec2(1.0,1.0))  / 2.0;

    uint id = texture(surface_id_buffer, viewport_coord).r;
    if(id <= 0)
        discard;

    vec4 params = texture(surface_color2_buffer, viewport_coord);
    vec4 c = texture(in_buffer, viewport_coord);
    vec4 p = texture(in_color2_buffer, viewport_coord);

    float scr_scaling = (screen_scale_x + screen_scale_x) / 2.0;
    //float blur_weight = params.r;
    float blur_weight = (dir == 0)? params.r * params.b : params.r * params.a;
    //float blur_weight = (dir == 0)? params.b : params.a;
    float blur_scaling = zoom * scr_scaling;// * blur_weight;

    /*{
        float al = pow(blur_weight, 1);
        float interpolated_size = gaussian_size * al + gaussian_size*min_gaussian_q*(1.0-al);
        int gaussianBlurRadius = int(ceil(blur_scaling * interpolated_size*1));
        float stdToRadiusFactor = 3.0;
        float gaussianKernelStd = gaussianBlurRadius * stdToRadiusFactor;


        float WH = float(W + H) / 2.0;
        float neigh = 1.0/WH;
        vec2 stepdir = (dir == 0)? vec2(neigh, 0) : vec2(0, neigh);

        vec4 p_sum = p;
        float pw_sum = 1;

        if(gaussianBlurRadius > 0) {
            for(int i = 0; i < gaussianBlurRadius; ++i) {
                float exponent = -(float(i*i) / (2.0 * gaussianKernelStd * gaussianKernelStd));
                float w = exp(exponent);
                vec2 step = stepdir * i;

                vec2 sample1_p = viewport_coord + step;
                vec4 p1 = texture(in_color2_buffer, sample1_p);
                uint id1 = texture(surface_id_buffer, sample1_p).r;
                if(id1 > 0) {
                    p_sum += p1;
                    pw_sum += w;
                }

                vec2 sample2_p = viewport_coord - step;
                vec4 p2 = texture(in_color2_buffer, sample2_p);
                uint id2 = texture(surface_id_buffer, sample2_p).r;
                if(id2 > 0) {
                    p_sum += p2;
                    pw_sum += w;
                }
            }
        }
        f_color2 = p_sum / pw_sum;
    }*/
    f_color2 = texture(in_color2_buffer, viewport_coord);


    float al = pow(blur_weight, 1);
    float interpolated_size = gaussian_size * al + gaussian_size*min_gaussian_q*(1.0-al);
    int gaussianBlurRadius = int(ceil(blur_scaling * interpolated_size));
    float stdToRadiusFactor = 3.0;
    float gaussianKernelStd = gaussianBlurRadius * stdToRadiusFactor;


    float WH = float(W + H) / 2.0;
    float neigh = 1.0/WH;
    vec2 stepdir = (dir == 0)? vec2(neigh, 0) : vec2(0, neigh);

    vec3 c_sum = c.rgb * c.a;
    float cw_sum = c.a;
    float a_sum = c.a;
    float aw_sum = 1;

    if(gaussianBlurRadius > 0) {
        for(int i = 0; i < gaussianBlurRadius; ++i) {
            float exponent = -(float(i*i) / (2.0 * gaussianKernelStd * gaussianKernelStd));
            float w = exp(exponent);
            vec2 step = stepdir * i;

            vec2 sample1_p = viewport_coord + step;
            vec4 c1 = texture(in_buffer, sample1_p);
            uint id1 = texture(surface_id_buffer, sample1_p).r;
            if(id1 > 0) {
                float c1w = w*c1.a;
                c_sum += c1.rgb*c1w;
                cw_sum += c1w;
                a_sum += c1.a;
                aw_sum += w;
            }

            vec2 sample2_p = viewport_coord - step;
            vec4 c2 = texture(in_buffer, sample2_p);
            uint id2 = texture(surface_id_buffer, sample2_p).r;
            if(id2 > 0) {
                float c2w = w*c2.a;
                c_sum += c2.rgb*c2w;
                cw_sum += c2w;
                a_sum += c2.a;
                aw_sum += w;
            }
        }
    }

    f_color.rgb = c_sum / cw_sum;
    f_color.a = a_sum / aw_sum;

}
