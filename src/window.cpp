#include <QLineEdit>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
//#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>
#include <QImage>
#include <QDate>
#include <QCheckBox>
#include <QButtonGroup>
#include <QSpinBox>
#include <QToolButton>

#include <QColorDialog>

#include "GL/glew.h"
#include <QOpenGLShaderProgram>

#include <sstream>
#include <iostream>
#include <string>

#include "window.h"
#include "mainwindow.h"

#include "graphics/graphics.h"
#include "camera.h"
#include "globals.h"
#include "state.h"

QPushButton* button_colorpicker;

Col qtcolor_to_color(QColor &qtc) {
    return Col(qtc.red()/255.0, qtc.green()/255.0, qtc.blue()/255.0, qtc.alpha()/255.0);
}

QColor color_to_qtcolor(Col &c) {
    return QColor((int)(c.r*255.0), (int)(c.g*255.0), (int)(c.b*255.0), (int)(c.a*255.0));
}

void Window::set_colorpicker_button_color() {
    QPalette pal = button_colorpicker->palette();
    button_colorpicker->setAutoFillBackground(true);
    pal.setColor(QPalette::Button, color_to_qtcolor(brush_color));
    button_colorpicker->setPalette(pal);
    button_colorpicker->setFlat(true);
    button_colorpicker->update();
}

void Window::set_brush_color() {
    QColor c = QColorDialog::getColor(color_to_qtcolor(brush_color));
    brush_color = qtcolor_to_color(c);
    set_colorpicker_button_color();
}

Window::Window() {

    space_down = false;

    setMouseTracking(true);

    draw = new Graphics(this);
    draw->setMouseTracking(true);
    draw->setAttribute(Qt::WA_Hover, true);

    pixelratio = devicePixelRatio();
    startTimer(timer_interval);

    QButtonGroup* group_tool = new QButtonGroup(this);
    group_tool->setExclusive(true);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    QVBoxLayout* canvas = new QVBoxLayout;
    canvas->setContentsMargins(0,0,0,0);
    canvas->setSpacing(0);
    canvas->addWidget(draw);
    QWidget* canvas_w = new QWidget;
    canvas_w->setLayout(canvas);
    canvas_w->setMouseTracking(true);
    mainLayout->addWidget(canvas_w);

    QVBoxLayout* buttons = new QVBoxLayout;
    buttons->setContentsMargins(0,0,0,0);
    buttons->setSpacing(0);
    buttons->setAlignment(Qt::AlignTop);
    QWidget* buttons_w = new QWidget;
    buttons_w->setLayout(buttons);
    mainLayout->addWidget(buttons_w);


    //QLabel *label_graphics = new QLabel(this);
    //label_graphics->setText("graphics");
    //buttons->addWidget(label_graphics);
    surface_button = new QCheckBox("surface");
    //buttons->addWidget(surface_button);
    gaussian_button = new QCheckBox("gaussian");
    //buttons->addWidget(gaussian_button);
    contours_button = new QCheckBox("contours");
    //buttons->addWidget(contours_button);
    nodes_button = new QCheckBox("nodes");
    //buttons->addWidget(nodes_button);
    edges_button = new QCheckBox("edges");
    //buttons->addWidget(edges_button);
    control_net_button = new QCheckBox("mesh");
    //buttons->addWidget(control_net_button);


    QLabel *label_camera = new QLabel(this);
    label_camera->setText("view");
    buttons->addWidget(label_camera);
    QPushButton* button_cameratool = new QPushButton("camera");
    connect(button_cameratool, &QPushButton::clicked, this, [this]{ set_tool(&cameratool); });
    button_cameratool->setShortcut(QKeySequence(Qt::Key_C));
    group_tool->addButton(button_cameratool);
    button_cameratool->setCheckable(true);
    buttons->addWidget(button_cameratool);
    QPushButton* button_reset_view = new QPushButton("reset");
    connect(button_reset_view, &QPushButton::clicked, this, &Window::reset_view);
    buttons->addWidget(button_reset_view);

    QLabel *label_color = new QLabel(this);
    label_color->setText("color");
    buttons->addWidget(label_color);
    button_colorpicker = new QPushButton("");
    connect(button_colorpicker, &QPushButton::clicked, this, &Window::set_brush_color);
    buttons->addWidget(button_colorpicker);
    set_colorpicker_button_color();
    auto button_random_color = new QPushButton("random");
    connect(button_random_color, &QPushButton::clicked,  this, [this]{ brush_color = Col::random(); set_colorpicker_button_color(); });
    button_random_color->setShortcut(QKeySequence(Qt::Key_R));
    buttons->addWidget(button_random_color);


    QLabel *brush_label = new QLabel(this);
    brush_label->setText("tools");
    buttons->addWidget(brush_label);

    QPushButton* button_colorpicker = new QPushButton("colorpicker");
    connect(button_colorpicker, &QPushButton::clicked, this, [this]{ set_tool(&colorpicker); });
    buttons->addWidget(button_colorpicker);
    group_tool->addButton(button_colorpicker);

    QPushButton* button_selecttool = new QPushButton("pincers");
    connect(button_selecttool, &QPushButton::clicked, this, [this]{ set_tool(&selecttool); });
    button_selecttool->setShortcut(QKeySequence(Qt::Key_X));
    group_tool->addButton(button_selecttool);
    button_selecttool->setCheckable(true);
    buttons->addWidget(button_selecttool);

    QPushButton* button_brush = new QPushButton("brush");
    connect(button_brush, &QPushButton::clicked, this, [this]{ set_tool(&brush); });
    button_brush->setShortcut(QKeySequence(Qt::Key_B));
    group_tool->addButton(button_brush);
    button_brush->setCheckable(true);
    buttons->addWidget(button_brush);

    QLabel *blend_label = new QLabel(this);
    blend_label->setText("blending");
    buttons->addWidget(blend_label);
    auto blend_slider = new QSlider(Qt::Orientation::Horizontal);
    blend_slider->setMaximum(1000);
    blend_slider->setMinimum(0);
    connect(blend_slider, &QSlider::valueChanged, this, [this](int t){ brush.blend_dist_th = 1.0-(double)t/1000.0; });
    blend_slider->setValue(1000);
    buttons->addWidget(blend_slider);

    blend_button = new QCheckBox("blend");
    connect(blend_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Blend); });
    buttons->addWidget(blend_button);

    drop_button = new QCheckBox("drop");
    connect(drop_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Drop); });
    buttons->addWidget(drop_button);
    //brush.toggle(Brush::Drop);
    //drop_button->setChecked(true);

    //split_button = new QCheckBox("split");
    //connect(split_button, &QPushButton::clicked, this, [this](bool b){ brush.toggle(Brush::Split_line); });
    //buttons->addWidget(split_button);
    rub_button = new QCheckBox("rub");
    connect(rub_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Rub); });
    buttons->addWidget(rub_button);
    push_button = new QCheckBox("push");
    connect(push_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Push); });
    buttons->addWidget(push_button);
    timer_button = new QCheckBox("timer");
    connect(timer_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Timer); });
    buttons->addWidget(timer_button);
    contrast_button = new QCheckBox("contrast");
    connect(contrast_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Contrast); });
    buttons->addWidget(contrast_button);

    blur_button = new QCheckBox("blur");
    connect(blur_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Blur); });
    buttons->addWidget(blur_button);
    smooth_button = new QCheckBox("smooth");
    connect(smooth_button, &QPushButton::clicked, this, [this](bool){ brush.toggle(Brush::Smooth); });
    buttons->addWidget(smooth_button);

    brush.toggle(Brush::Rub);
    rub_button->setChecked(true);
    //brush.toggle(Brush::Blend);
    //blend_button->setChecked(true);

    set_tool(&brush);

    QLabel *label_file = new QLabel(this);
    label_file->setText("painting");
    buttons->addWidget(label_file);
    QPushButton* dump_canvas_button = new QPushButton("render");
    connect(dump_canvas_button, &QPushButton::clicked, this, &Window::dump_canvas);
    dump_canvas_button->setShortcut(QKeySequence(Qt::Key_P));
    buttons->addWidget(dump_canvas_button);
    save_name = new QLineEdit;
    save_name->setPlaceholderText("out");
    save_name->setMaximumWidth(76);
    buttons->addWidget(save_name);
    QHBoxLayout* saveload_layout = new QHBoxLayout;
    saveload_layout->setContentsMargins(0,0,0,0);
    saveload_layout->setSpacing(0);
    QWidget* saveload_layout_w = new QWidget;
    saveload_layout_w->setLayout(saveload_layout);
    buttons->addWidget(saveload_layout_w);
    QPushButton* save_canvas_button = new QPushButton("save");
    save_canvas_button->setMaximumWidth(38);
    connect(save_canvas_button, &QPushButton::clicked, this, [this]{ save_file(get_QLineEdit_string(save_name)); draw->update(); });
    save_canvas_button->setShortcut(QKeySequence(Qt::Key_F11));
    saveload_layout->addWidget(save_canvas_button);
    QPushButton* load_canvas_button = new QPushButton("load");
    load_canvas_button->setMaximumWidth(38);
    connect(load_canvas_button, &QPushButton::clicked, this, [this]{ load_file(get_QLineEdit_string(save_name)); draw->update(); });
    load_canvas_button->setShortcut(QKeySequence(Qt::Key_F12));
    saveload_layout->addWidget(load_canvas_button);
    QPushButton* new_canvas_button = new QPushButton("new");
    connect(new_canvas_button, &QPushButton::clicked, this, [this]{ new_canvas(); });
    buttons->addWidget(new_canvas_button);

    //todo: keep track of all pointers and new objects and delete them nicely at destruction.
}

void Window::new_canvas()
{
    delete com;
    com = new Complex(true);
    draw->update();
    selecttool.reset();

    std::vector<Tri*> tris;
    for(auto t : com->ts)
        tris.push_back(t);

    /*
    for(int k = 0; k < 100; ++k) {
//    for(int k = 0; k < 0; ++k) {
        int i = rand() % tris.size();
        Vec3 ru3(rand_uni(),rand_uni(),rand_uni());
        Vec2 ru = Vec2(Vec3(1.0)/3.0);
        auto n = com->split(*tris[i], ru);
        if(n == nullptr) continue;
        Vec2 p = Vec2(rand_uni()*2-1,rand_uni()*2-1)*com->canvas().x;
        //Vec2 p = Vec2(normrand*2-1, ((int)(normrand+0.5))*(normrand*2-1) )*com->canvas().x;
        com->move_consec(*n, p);
        for(auto e : *n) {
            if(e->t != tris[i] && !e->t->invalid())
                tris.push_back(e->t);
        }
        com->delaunay(*n);
    }
    com->randomize();*/


    com->split_edge(&tris[0]->b, 0.5);

    /*
    for(int k = 0; k < 4; ++k) {
        int i = rand() % tris.size();
        Vec3 ru3(rand_uni(),rand_uni(),rand_uni());
        Vec2 ru = Vec2(Vec3(1.0)/3.0);
        auto n = com->split(*tris[i], ru);
        if(n == nullptr) continue;
        Vec2 p = Vec2(rand_uni()*2-1,rand_uni()*2-1)*com->canvas().x;
        //Vec2 p = Vec2(normrand*2-1, ((int)(normrand+0.5))*(normrand*2-1) )*com->canvas().x;
        com->move(*n, p);
        for(auto e : *n) {
            if(e->t != tris[i] && !e->t->invalid())
                tris.push_back(e->t);
        }
    }
    */

    com->move_nodes();
    com->delaunify();
    com->randomize();
    com->color_to_line();
    com->purge();
    draw->update();
}


void Window::set_surface(bool b){ draw->render_active("surface", b); draw->update(); emit surface_changed(b); }
void Window::set_control_net(bool b){ draw->render_active("control_net", b); draw->update(); emit control_net_changed(b); }
void Window::set_edges(bool b){ draw->render_active("edges", b); draw->update(); emit edges_changed(b); }
void Window::set_nodes(bool b){ draw->render_active("nodes", b); draw->update(); emit nodes_changed(b); }
void Window::set_contours(bool b){ draw->render_active("painting", b); draw->update(); emit contours_changed(b); }
void Window::set_gaussian(bool b){
    draw->render_active("gaussian_v", b);
    draw->render_active("gaussian_h", b);
    draw->update();
    emit gaussian_changed(b);
}

void Window::connect_toggle_buttons() {
/*
    connect(surface_button, &QCheckBox::toggled, this, &Window::set_surface);
    connect(this, &Window::surface_changed, surface_button, &QCheckBox::setChecked);
    surface_button->setShortcut(QKeySequence(Qt::Key_F3));
    emit surface_changed(draw->is_render_active("surface"));

    connect(gaussian_button, &QCheckBox::toggled, this, &Window::set_gaussian);
    connect(this, &Window::gaussian_changed, gaussian_button, &QCheckBox::setChecked);
    gaussian_button->setShortcut(QKeySequence(Qt::Key_F4));
    emit gaussian_changed(draw->is_render_active("gaussian_h"));

    connect(contours_button, &QCheckBox::toggled, this, &Window::set_contours);
    connect(this, &Window::contours_changed, contours_button, &QCheckBox::setChecked);
    contours_button->setShortcut(QKeySequence(Qt::Key_F5));
    emit contours_changed(draw->is_render_active("painting"));

    connect(nodes_button, &QCheckBox::toggled, this, &Window::set_nodes);
    connect(this, &Window::nodes_changed, nodes_button, &QCheckBox::setChecked);
    nodes_button->setShortcut(QKeySequence(Qt::Key_F6));
    emit nodes_changed(draw->is_render_active("nodes"));

    connect(edges_button, &QCheckBox::toggled, this, &Window::set_edges);
    connect(this, &Window::edges_changed, edges_button, &QCheckBox::setChecked);
    edges_button->setShortcut(QKeySequence(Qt::Key_F7));
    emit edges_changed(draw->is_render_active("edges"));

    connect(control_net_button, &QCheckBox::toggled, this, &Window::set_control_net);
    connect(this, &Window::control_net_changed, control_net_button, &QCheckBox::setChecked);
    control_net_button->setShortcut(QKeySequence(Qt::Key_F8));
    emit control_net_changed(draw->is_render_active("control_net"));
*/
    /*
    connect(, &QCheckBox::toggled, this, &Window::set_control_net);
    connect(this, &Window::control_net_changed, control_net_button, &QCheckBox::setChecked);
    emit control_net_changed(draw->is_render_active("control_net"));
    */

    /*
    paint_button = new QCheckBox("paint");
    buttons->addWidget(paint_button);
    blend_button = new QCheckBox("blend");
    buttons->addWidget(blend_button);
    split_button = new QCheckBox("split");
    buttons->addWidget(split_button);
    rub_button = new QCheckBox("rub");
    buttons->addWidget(rub_button);
    push_button = new QCheckBox("push");
    buttons->addWidget(push_button);
    contrast_button = new QCheckBox("contrast");
    buttons->addWidget(contrast_button);
    blur_button = new QCheckBox("blur");
    buttons->addWidget(blur_button);
    smooth_button = new QCheckBox("smooth");
    buttons->addWidget(smooth_button);
    timer_button = new QCheckBox("timer");
    buttons->addWidget(timer_button);
    */




}
