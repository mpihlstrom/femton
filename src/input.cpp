#include <sstream>
#include <qpainter.h>
#include <QDateTime>
#include <QLineEdit>
#include <QCoreApplication>
#include <QGraphicsScene>
#include <QKeyEvent>
#include "window.h"
#include "state.h"

#include <QThread>

#ifdef _WIN32
#include <winuser.h>
#endif

#include <algorithm>

Vec2 cursor;

unsigned long prev_pointer_time;


void Graphics::hover_mouse_event(QHoverEvent* me) {

#ifdef TEST_WIN32
    POINT point = {me->pos().x(), me->pos().y()};
    HWND hWnd = (HWND)effectiveWinId();

    ClientToScreen(hWnd, &point);

    MOUSEMOVEPOINT mmpoint = {point.x, point.y, (unsigned long)GetMessageTime()};
    MOUSEMOVEPOINT mmpoints[64];
    int num_points = GetMouseMovePointsEx(sizeof(mmpoint), &mmpoint, mmpoints, 64, GMMP_USE_DISPLAY_POINTS);

    POINT point0 = {(LONG)::cursor.x, (LONG)::cursor.y};
    POINT point0scr = point0;
    ClientToScreen(hWnd, &point0scr);

    int i = 0;
    while(i < num_points && !(mmpoints[i].time < prev_pointer_time) && !(mmpoints[i].time == prev_pointer_time && mmpoints[i].x == point0scr.x && mmpoints[i].y == point0scr.y))
       ++i;
    while(i-- > 0) {
        POINT point1 = { mmpoints[i].x, mmpoints[i].y };
        ScreenToClient(hWnd, &point1);

        Vec2 rel(point1.x - point0.x, point1.y - point0.y);
        ::cursor = Vec2(point1.x, point1.y);
        mousetool->motion(Vec2(rel.x, rel.y));
        point0 = point1;
    }

    prev_pointer_time = (unsigned long)GetMessageTime();
#else
    ::cursor = Vec2(me->position().x(), me->position().y()) * pixelratio;
    mousetool->motion(Vec2(me->position().x() - me->oldPos().x(), me->position().y() - me->oldPos().y()) * pixelratio);
#endif

    update();
}


void Window::timerEvent(QTimerEvent*)
{
    ++timer_counter;
    if(timer_counter % timer_modulo == 0)
        mousetool->timer_update();
}

void Window::dump_canvas2(std::string name) {
    //QImage img = draw->grab_render(Size(2048, 2048));
    QImage img = draw->grab_render2(Size(draw->render_sz.w, draw->render_sz.h));

    QGraphicsScene* scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 1, 1);
    QPainter painter(&img);
    scene->render(&painter);

    std::stringstream filename;

    filename << "series/" << name << ".png";
    if(img.save(filename.str().c_str())) {
        std::cout << "'" << filename.str().c_str() << "' saved." << std::endl;
    } else {
        std::cout << "Failed to save '" << filename.str().c_str() << "'." << std::endl;
    }
}

void Window::dump_canvas() {
    //QImage img = draw->grab_render(Size(2048, 2048));
    QImage img = draw->grab_render(Size(draw->render_sz.w, draw->render_sz.h));

    QGraphicsScene* scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 2, 2);
    QPainter painter(&img);
    scene->render(&painter);

    std::stringstream filename;

    QString time_str = QDateTime::currentDateTime().toString(QString("yyMMdd-hhmmss"));

    filename << "out/" << time_str.toStdString() << ".png";
    if(img.save(filename.str().c_str())) {
        std::cout << "'" << filename.str().c_str() << "' saved." << std::endl;
    } else {
        std::cout << "Failed to save '" << filename.str().c_str() << "'." << std::endl;
    }
}

void Window::wasd_press(int key) {
    if(mousetool == &cameratool) {
        const double MoveSpeed = 0.03;
        Vec2 v;
        if(key == Qt::Key_W) v.y -= MoveSpeed;
        if(key == Qt::Key_S) v.y += MoveSpeed;
        if(key == Qt::Key_A) v.x += MoveSpeed;
        if(key == Qt::Key_D) v.x -= MoveSpeed;
        view->pos += view->uni_can_v(v);
    }
}

std::string Window::get_QLineEdit_string(QLineEdit* le) {
    return le->text().isEmpty() ? le->placeholderText().toStdString() : le->text().toStdString();
}

void Window::reset_view() {
    view->reset();
    draw->update();
}

void Window::keyReleaseEvent(QKeyEvent *e) {
    switch(e->key()) {
    case Qt::Key_Control:
        space_down = false;
        break;
    }
}

bool stop = false;
int frame = 1;

void Window::keyPressEvent(QKeyEvent *e) {
    switch(e->key()) {

    case Qt::Key_Escape:
        close();
        QCoreApplication::quit();
        break;

    case Qt::Key_Control:
        space_down = true;
        break;

    case Qt::Key_0: {
        reset_view();
        break;
    }

        /*
    case Qt::Key_B:
        set_buffers = !set_buffers;
        myDebug() << set_buffers;
        break;
        */

        /*
    case Qt::Key_W:
    case Qt::Key_S:
    case Qt::Key_A:
    case Qt::Key_D:
        wasd_press(e->key());
        break;
    */

    case Qt::Key_A:
        if(com->move_mode == Complex::Concur)  {
            com->move_mode = Complex::Consec;
            myDebug() << "consecutive mode";
        }
        else {
            com->move_mode = Complex::Concur;
            myDebug() << "concurrent mode";
        }
        break;

    case Qt::Key_G:
        com->waste();
        break;

    case Qt::Key_E:
    {
        int i = 0;
        dump_canvas2(std::to_string(++i));
        while(true) {
            com->automata();
            dump_canvas2(std::to_string(++i));
            Sleep(10);
        }
        dump_canvas2(std::to_string(++i));


        /*
        int i = 0;
        for(int k = 0; k < 10; ++k) {
            new_canvas();
            com->delaunify(false);
            com->randomize();
            com->move_random();
            //dump_canvas2(std::to_string(++i));
            com->move_nodes_populate();

            dump_canvas2(std::to_string(++i));
            for(; !com->move_nodes();) {
                dump_canvas2(std::to_string(++i));
                Sleep(10);
            }
            dump_canvas2(std::to_string(++i));
        }
        */

        break;
    }

    case Qt::Key_Q:
    {
        com->automata();
        break;
    }

    case Qt::Key_T:
        com->delaunify(false);
    case Qt::Key_Y:
        com->delaunify();
        break;
    case Qt::Key_U:
        com->randomize();
        break;
    case Qt::Key_H:
        load_file(get_QLineEdit_string(save_name));
        break;

    /*case Qt::Key_K: {
            myDebug() << "\n ::mv_tris:: \n";

            std::list<Cross*> mv;
            for(auto i = com->mv_tris.begin(); i != com->mv_tris.end(); ++i) {
                mv.push_back(*i);
            }

            mv.sort(cross_comp_lt);
            for(auto i = mv.begin(); i != mv.end(); ++i) {
                auto c = *i;
                if(!c->invalid) {
                    myDebug() << "current cross case:" << c->g;
                    myDebug() << "c->crossroot.kind:" << c->crossroot.kind;
                    myDebug() << "c->overlap:" << c->overlap;
                    myDebug() << "curt" << com->curt;
                    myDebug() << "t" << c->t;
                    myDebug() << "";
                }
            }
        }
        break;*/

    case Qt::Key_I:
        interpolate_move = !interpolate_move;
        break;

    case Qt::Key_N:
        new_canvas();
        break;

    case Qt::Key_Z:
        myDebug() << "set_buffers" << set_buffers;
        set_buffers = !set_buffers;
        break;

    case Qt::Key_9:
        contrast_scale *= 1.05f;
        myDebug() << "contrast_scale";
        myDebug() << contrast_scale;
        break;
    case Qt::Key_8:
        contrast_scale /= 1.05f;
        myDebug() << "contrast_scale";
        myDebug() << contrast_scale;
        break;

    case Qt::Key_Plus:
        gaussian_size *= 1.05f;
        myDebug() << "gaussian_size";
        myDebug() << gaussian_size;
        break;
    case Qt::Key_Minus:
        gaussian_size /= 1.05f;
        myDebug() << "gaussian_size";
        myDebug() << gaussian_size;
        break;


/*
    case Qt::Key_Plus:
        manual_curt = fmin(1.0, manual_curt + 0.01);
        myDebug() << "manual_curt" << manual_curt;
        break;
    case Qt::Key_Minus:
        manual_curt = fmax(0.0, manual_curt - 0.01);
        myDebug() << "manual_curt" << manual_curt;
        break;
        */


    case Qt::Key_F3:
        draw->toggle_render("surface");
        break;
    case Qt::Key_F4:
        draw->toggle_render("gaussian_v");
        break;
    case Qt::Key_F5:
        draw->toggle_render("painting");
        break;
    case Qt::Key_F6:
        draw->toggle_render("nodes");
        break;
    case Qt::Key_F7:
        draw->toggle_render("edges");
        break;
    case Qt::Key_F8:
        draw->toggle_render("control_net");
        break;

    case Qt::Key_F9:
        save_file(get_QLineEdit_string(save_name));
        break;

    case Qt::Key_F10:
        frame = 1;
        load_file(get_QLineEdit_string(save_name));
        break;

    default:
        QWidget::keyPressEvent(e);
        return;
    }

    draw->update();
}



bool Graphics::event(QEvent *event) {
    switch(event->type()) {
    case QEvent::Wheel: {
            QWheelEvent* we = (QWheelEvent*)event;
            //if(we->orientation() == Qt::Vertical) {
            mousetool->wheel_rot((double)we->angleDelta().y());
            //}
            break;
        }
    case QEvent::MouseButtonPress: {
            QMouseEvent* me = (QMouseEvent*)event;
            if(me->button() == Qt::MouseButton::LeftButton) {
                Brush::left = true;
                mousetool->left_down(::cursor);
            } else if(me->button() == Qt::MouseButton::RightButton) {
                Brush::right = true;
                mousetool->right_down(::cursor);
            }
            break;
        }
    case QEvent::MouseButtonRelease: {
            QMouseEvent* me = (QMouseEvent*)event;
            if(me->button() == Qt::MouseButton::LeftButton) {
                Brush::left = false;
                mousetool->left_up(::cursor);
            } else if(me->button() == Qt::MouseButton::RightButton) {
                Brush::right = false;
                mousetool->right_up(::cursor);
            }
            break;
        }
    case QEvent::HoverEnter:
        setCursor(Qt::BlankCursor);
        draw_cursor = true;
        update();
        break;
    case QEvent::HoverLeave:
        unsetCursor();
        draw_cursor = false;
        update();
        break;
    case QEvent::HoverMove: {
            hover_mouse_event((QHoverEvent*)event);
            if(false)//(Brush::left || Brush::right)
            {
                com->automata();
                ::window->dump_canvas2(std::to_string(frame++));
            }
            break;
        }
    default:
        break;
    }
    return QWidget::event(event);
}
