#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include "graphics/graphics.h"
#include "common/entity.h"
#include "tool/tools.h"
#include "globals.h"

const double NormalPitchSpeed = 0.05;
const double NormalYawSpeed = 1.;

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE

class GLWidget;
class MainWindow;

class Window : public QWidget {
    Q_OBJECT

public:
    Window();

    void dump_canvas2(std::string name = "");

public slots:
    void set_brush_color();

    void new_canvas();
    void reset_view();
    void dump_canvas();
    void connect_toggle_buttons();

    void set_surface(bool b);
    void set_control_net(bool b);
    void set_edges(bool b);
    void set_nodes(bool b);
    void set_contours(bool b);
    void set_gaussian(bool b);

signals:
    void surface_changed(bool b);
    void control_net_changed(bool b);
    void edges_changed(bool b);
    void nodes_changed(bool b);
    void contours_changed(bool b);
    void gaussian_changed(bool b);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

public:
    bool space_down;

    void set_colorpicker_button_color();

private:
    void wasd_press(int key);
    std::string get_QLineEdit_string(QLineEdit* le);

    QMainWindow *main_window;
    QLineEdit* save_name;

    QCheckBox *surface_button, *gaussian_button, *control_net_button, *edges_button, *nodes_button, *contours_button;
    QCheckBox *drop_button, *blend_button, *split_button, *rub_button, *push_button, *contrast_button, *blur_button, *smooth_button, *timer_button;

    const int timer_interval = 10; //ms
    unsigned int timer_modulo = 1;
    unsigned int timer_counter = 0;
};
