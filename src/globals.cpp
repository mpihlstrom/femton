#include "globals.h"
#include "window.h"
#include "state.h"

class Graphics;

Complex* com;
Camera* view;
Graphics* draw;

Void void_selection;

Size init_scr_sz(640, 640);
Size scr_sz(init_scr_sz);
double pixelratio;

Window* window;

Tool* mousetool;

Brush brush;
Paintbrush paintbrush;
Contrastbrush contrastbrush;
Blurbrush blurbrush;
Repelpinchbrush repelpinchbrush;
Smoothbrush smoothbrush;
Rubbrush rubbrush;
Cameratool cameratool;
Selecttool selecttool;
Colorpicker colorpicker;

std::unordered_map<unsigned int, Class const*> selectable;

unsigned int gid = 0;

bool interpolate_move = true;
double manual_curt = 1.0;
bool set_buffers = true;

Edge* flipped_e;
Edge* flipped_j;
Node* flipped_en;
Node* flipped_jn;

Col brush_color;

std::stringstream debug_strstr;



bool load_file(std::string filename0) {
    auto filename = filename0 + ".qua";
    if(Complex* new_com = State::load_from(filename)) {
        delete com;
        com = new_com;
        selecttool.reset();
        return true;
    }
    return false;
}

bool save_file(std::string filename0) {
    auto filename = filename0 + ".qua";
    return State::export_to(*com, filename);
}


void set_tool(Tool* mt) {
    if(mousetool) mousetool->reset();
    mousetool = mt;
    draw->update();
}
