#pragma once

#include <unordered_map>
#include <sstream>

#include "camera.h"
#include "graphics/graphics.h"
#include "complex/complex.h"
#include "tool/tools.h"

extern Camera* view;
extern Graphics* draw;
extern Complex* com;

extern Void void_selection;

extern Size init_scr_sz;
extern Size scr_sz;
extern double pixelratio;

class Window;
extern Window* window;

extern Vec2 cursor;
extern Tool* mousetool;
extern Brush brush;
extern Cameratool cameratool;
extern Selecttool selecttool;
extern Paintbrush paintbrush;
extern Contrastbrush contrastbrush;
extern Blurbrush blurbrush;
extern Repelpinchbrush repelpinchbrush;
extern Smoothbrush smoothbrush;
extern Rubbrush rubbrush;
extern Colorpicker colorpicker;

extern std::unordered_map<unsigned int, Class const*> selectable;

extern float contrast_scale;

extern unsigned int gid;

extern bool interpolate_move;
extern double manual_curt;

extern int r_count;

extern Edge* flipped_e;
extern Edge* flipped_j;
extern Node* flipped_en;
extern Node* flipped_jn;

extern bool draw_contours;
extern bool draw_flat;
extern float line_width;

extern float gaussian_size;
extern float min_gaussian_q;

extern bool set_buffers;

extern Col brush_color;

bool load_file(std::string filename);
bool save_file(std::string filename);

void set_tool(Tool* mt);

extern std::stringstream debug_strstr;
