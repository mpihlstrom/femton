#version 410 core
layout(location = 0) out vec4 f_color;

uniform sampler2D  surface_buffer;
uniform sampler2D  surface_color2_buffer;
uniform usampler2D surface_id_buffer;
uniform sampler2D  gaussian_v_buffer;
uniform sampler2D  gaussian_v_color2_buffer;

uniform int draw_gaussian_v;

uniform int H;
uniform int W;

uniform int dir;

uniform float zoom;
uniform float screen_scale_x;
uniform float screen_scale_y;

uniform float gaussian_size;
uniform float min_gaussian_q;
uniform float contrast_scale;

in vec2 v_position;
in vec4 gl_FragCoord;

//gaussian code from: https://dsp.stackexchange.com/questions/23460/how-to-calculate-gaussian-kernel-for-a-small-support-size


float c_dist(vec4 f, vec4 s) {
    return sqrt(((s.r*s.a-f.r*f.a)*(s.r*s.a-f.r*f.a) + (s.g*s.a-f.g*f.a)*(s.g*s.a-f.g*f.a) + (s.b*s.a-f.b*f.a)*(s.b*s.a-f.b*f.a))/3.0);
}


/*float c_dist(vec4 c) {
    //return
    //vec4 w = vec4(0.21267, 0.71516, 0.07217, 1);
    vec3 w = vec3(1);
    vec3 wc = w.rgb*c.rgb;
    return   pow(dot(wc,wc)/3.0,0.5);
}*/

void main()
{
    vec2 viewport_coord = (v_position + vec2(1.0,1.0))  / 2.0;

    uint id = texture(surface_id_buffer, viewport_coord).r;
    if(id <= 0)
        discard;

    vec4 surface_c = texture(surface_buffer, viewport_coord);

    vec4 col = surface_c;

    if(draw_gaussian_v == 1)  {
        vec4 gaussian_c = texture(gaussian_v_buffer, viewport_coord);
        //vec4 params = texture(surface_color2_buffer, viewport_coord);
        vec4 params = texture(gaussian_v_color2_buffer, viewport_coord);

        col = gaussian_c;


        float scr_scaling = (screen_scale_x + screen_scale_x) / 2.0;
        //float blur_weight = params.r;
        float blur_weight = (dir == 0)? params.r * params.b : params.r * params.a;
        //float blur_weight = (dir == 0)? params.b : params.a;
        float blur_scaling = zoom * scr_scaling;// * blur_weight;

        float al = pow(blur_weight, 1);
        float interpolated_size = gaussian_size * al + gaussian_size*min_gaussian_q*(1.0-al);
        int gaussianBlurRadius = int(ceil(blur_scaling * interpolated_size));
        float stdToRadiusFactor = 3.0;
        float gaussianKernelStd = gaussianBlurRadius * stdToRadiusFactor;

        float contrast_weight = params.g;
        float contrast = (contrast_scale);//* scr_scaling;

        if(gaussianBlurRadius > 0) {
            vec4 closest_c = surface_c;
            float min_dist = 1.0;

            vec4 goal = gaussian_c;

            float WH = float(W + H) / 2.0;
            float neigh = 1.0/WH;
            vec2 stepdirh = vec2(neigh, 0);
            vec2 stepdirv = vec2(0, neigh);

            //jump pixels when zoomed in for faster rendering but limit it when zoomed out.
            int stepsz = int(max(1.0, sqrt(zoom)));

            float cw = contrast_weight;

            vec3 c_sum = vec3(0);
            float cw_sum = 0;
            float a_sum = 0;
            float aw_sum = 0;

            float max_dist = 0;
            float dist_sum = 0;


            if(false) {
                int stepsz2 = stepsz;
                int radius2 = gaussianBlurRadius;
                int count = 0;
                for(int i = -radius2; i < radius2; i += stepsz2) {
                    for(int j = -radius2; j < radius2; j += stepsz2, ++count) {
                        vec2 step = stepdirh * i + stepdirv * j;
                        vec2 sample1_p = viewport_coord + step;
                        vec4 c1 = texture(surface_buffer, sample1_p);
                        uint id1 = texture(surface_id_buffer, sample1_p).r;
                        if(id1 > 0) {
                            float dist = c_dist(goal, c1);// * w;
                            if(dist < min_dist) {
                                closest_c = c1;
                                min_dist = dist;
                            }
                            dist_sum += dist;
                        }
                    }
                }

                al = contrast_weight;//*(1.0 - pow(c_dist(goal - closest_c),2));//contrast_weight;
                col = closest_c * al + gaussian_c * (1-al);
            } else {
                int stepsz2 = stepsz;
                int radius2 = gaussianBlurRadius;

                int count = 0;
                for(int i = -radius2; i < radius2; i += stepsz2) {
                    for(int j = -radius2; j < radius2; j += stepsz2, ++count) {
                        vec2 step = stepdirh * i + stepdirv * j;
                        vec2 sample1_p = viewport_coord + step;
                        vec4 c1 = texture(surface_buffer, sample1_p);
                        uint id1 = texture(surface_id_buffer, sample1_p).r;
                        if(id1 > 0) {
                            //float w = 1 - sqrt(i*i + j*j) / (radius2);
                            float dist = c_dist(goal, c1);

                            if(dist > max_dist) {
                                max_dist = dist;
                            }

                            if(dist < min_dist) {
                                closest_c = c1;
                                min_dist = dist;
                            }
                            dist_sum += dist;
                        }
                    }
                }
                dist_sum /= float(count);

                for(int count = 0, i = -gaussianBlurRadius; i < gaussianBlurRadius; i += stepsz) {
                    for(int j = -gaussianBlurRadius; j < gaussianBlurRadius; j += stepsz, ++count) {
                        vec2 step = stepdirh * i + stepdirv * j;
                        float exp_w = -(pow((i*j),2) / (2.0 * gaussianKernelStd * gaussianKernelStd));
                        vec2 sample1_p = viewport_coord + step;
                        vec4 c1 = texture(surface_buffer, sample1_p);
                        vec4 g1 = texture(gaussian_v_buffer, sample1_p);
                        uint id1 = texture(surface_id_buffer, sample1_p).r;
                        if(id1 > 0) {
                            //float dist = (c_dist(goal, c1) - min_dist) / (max_dist - min_dist);
                            float dist = (c_dist(goal, c1)) / (max_dist);

                            float exp_d = -dist * contrast;
                            float exp_gd = exp_d * cw + exp_w * (1-cw);
                            float w = exp(exp_gd);

                            float c1w = w*c1.a;
                            c_sum += c1.rgb*c1w;
                            cw_sum += c1w;
                            a_sum += c1.a;
                            aw_sum += w;
                        }
                    }
                }

                const float epsilon = 0.00000001;
                if(cw_sum > epsilon) {
                    col.rgb = c_sum / cw_sum;
                    col.a = a_sum / aw_sum;
                } else {
                    col = closest_c;//vec4(1,1,0,1);
                }
            }
        }
    }
    f_color = col;
}
