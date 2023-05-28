#include "complex/complex.h"
#include "graphics/graphics.h"
#include "init.h"
#include "common/color.h"
#include "state.h"
#include "globals.h"
#include "window.h"

#include "json/json.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <math.h>
#include <time.h>

#include "util.h"
#include "globals.h"

bool Tool::left = false;
bool Tool::right = false;

void pre_init() {
    //srand((unsigned int)time(0));
    srand(0);
    Root::init_mpzs();
    view = new Camera();
    com = new Complex(true);
    brush_color = Col::random();
}

void post_init() {
    window->connect_toggle_buttons();
    window->reset_view();

    Json::Value root;
    Json::Reader reader;
    std::string config(get_file_contents("config.json"));
    if(!config.empty() && !reader.parse(config, root))
        std::cout << "Json failure: \n" << reader.getFormattedErrorMessages();

    line_width = root.get("line width", .001).asFloat();
    point_size = root.get("point size", 4.).asFloat();
    window->set_surface(root.get("surface", true).asBool());
    window->set_gaussian(root.get("gaussian", true).asBool());
    window->set_contours(root.get("painting", true).asBool());
    window->set_nodes(root.get("nodes", false).asBool());
    window->set_edges(root.get("edges", false).asBool());
    window->set_control_net(root.get("control net", false).asBool());
    gaussian_size = root.get("gaussian_size", 10.).asFloat();
    min_gaussian_q = root.get("min_gaussian_q", 10.).asFloat();
    contrast_scale = root.get("contrast_scale", 200.).asFloat();

    view->zrotzoom = Vec2(root.get("camera_zoom", 1.).asFloat(), 0.0);
    std::string loadfile = root.get("load", "").asString();
    if(!loadfile.empty()) {
        load_file(loadfile);
    }

    com->move_mode = com->str_to_move(root.get("move_mode", "conseq").asString());

    //view->pos = Vec2(-100, -100);
    //view->zrotzoom = Vec2(0.99,0);

    brush_color = Col::White;
    window->set_colorpicker_button_color();
}

void destroy() {
    Root::clear_mpzs();
    delete com;
}
