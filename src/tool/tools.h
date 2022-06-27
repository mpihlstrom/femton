#pragma once
#include "common/entity.h"
#include "camera.h"
#include "complex/node.h"

struct Tool {
    Tool() { radius = 0.008; }
    virtual void reset() {}
    virtual void timer_update() {}
    virtual void right_down(Vec2) {}
    virtual void motion(Vec2) {}
    virtual void left_down(Vec2) {}
    virtual void left_up(Vec2) {}
    virtual void right_up(Vec2) {}
    virtual void wheel_rot(double) {}

public:
    static bool left;
    static bool right;
    double radius;
};

struct Brush : Tool {
    static constexpr double default_brush_radius = 0.15;
    static constexpr double min_split_len_factor = 3;
    double blend_dist_th = 1;

    enum Actions {
        Drop = 1<<1,
        Split_line = 1<<2,
        Push = 1<<3,
        Rub = 1<<4,
        Contrast = 1<<5,
        Blur = 1<<6,
        Repelpinch = 1<<7,
        Smooth = 1<<8,
        Blend = 1<<9,
        Timer = 1<<10
    };

    Brush() : actions(0) { radius = default_brush_radius; }
    Brush(int64_t actions) : actions(actions) { radius = default_brush_radius; }
    virtual void timer_update();
    virtual void right_down(Vec2) { action(); }
    virtual void motion(Vec2);
    virtual void left_down(Vec2);
    virtual void left_up(Vec2) {}
    virtual void right_up(Vec2) {}
    virtual void wheel_rot(double);
    virtual void action(Vec2 const* scr_v = nullptr);
    void toggle(Actions a) { actions ^= (unsigned int)a; }

    void split_radius();
    void split_line(Vec2 rel);
    void rub(Vec2 rel);
    void blend();
    void change_contrast();
    void change_blur();
    void push();
    void smooth();

    void treat_canvas();

private:
    Node* split_intersection(Edge* e);
    int64_t actions;
    bool perturb;
};

struct Paintbrush : Brush { Paintbrush() : Brush(Brush::Drop| Brush::Split_line | Brush::Push) {} };
struct Rubbrush : Brush { Rubbrush() : Brush(Brush::Rub | Brush::Blend | Brush::Split_line) {} };
struct Contrastbrush : Brush { Contrastbrush() : Brush(Brush::Contrast | Brush::Timer) {} };
struct Blurbrush : Brush { Blurbrush() : Brush(Brush::Blur | Brush::Timer) {} };
struct Repelpinchbrush : Brush { Repelpinchbrush() : Brush(Brush::Repelpinch | Brush::Split_line | Brush::Timer) {} };
struct Smoothbrush : Brush { Smoothbrush() : Brush(Brush::Smooth | Brush::Split_line | Brush::Timer) {} };

struct Cameratool : Tool {
    Cameratool() {}
    void motion(Vec2 rel);
    void left_down(Vec2 p);
    void left_up(Vec2 p);
    void right_down(Vec2 p);
    void right_up(Vec2 p);
    void wheel_rot(double delta);
    Vec2 o_mouse;
    Vec2 mouse;
    Vec2 o_zrotz;
};

struct Selecttool : Tool  {
    Selecttool() : selected(0) {}
    void motion(Vec2 rel);
    void left_down(Vec2 p);
    void right_down(Vec2 p);
    void wheel_rot(double delta);
    int id() { return selected? selected->id : 0; }
    Class* selected_() { return selected? selected : 0; }
    void reset() { selected = 0; }
private:
    Class* selected;
};

struct Colorpicker : Tool {
    Colorpicker() {}
    void left_down(Vec2 p_scr);
    void right_down(Vec2 p_scr);
    void motion(Vec2 v_scr);
    void pick(Vec2i p_scr);
    Col o_color;
};
