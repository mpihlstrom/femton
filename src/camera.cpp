#include "camera.h"
#include "common/common.h"
#include "complex/complex.h"
#include "globals.h"

Camera::Camera(Vec2 pos) : pos(pos), zrotzoom(Vec2(1,0)) {}
Camera::Camera() {
    reset();
}

void Camera::reset() {
    zrotzoom = Vec2(1,0);
    pos = Vec2();
}

double Camera::rotz() const {
    return deg(atan2(zrotzoom.y, zrotzoom.x));
}

Vec2 Camera::uni_can_v(Vec2 v) {
    return v.scalerotate_inv(zrotzoom) * com->Units;
}

Vec2 Camera::scr_can_v(Vec2 v) {
    return uni_can_v(scr_uni_v(v));
}

Vec2 Camera::scr_uni_v(Vec2 v) {
    return Vec2((v.x / (double)scr_sz.w), -(v.y / (double)scr_sz.h)) * 2.0;
}

Vec2 Camera::scr_uni_p(Vec2 p) {
    return Vec2(p.x/(double)scr_sz.w - 0.5, (1.0 - p.y/(double)scr_sz.h) - 0.5) * 2.0;
}

Vec2 Camera::uni_scr(Vec2 p) {
    return Vec2((1.0 + p.x)*0.5 * (double)scr_sz.w, (1.0 - p.y)*0.5 * (double)scr_sz.h);
}

Vec2 Camera::scr_can_p(Vec2 p) {
    return scr_uni_p(p).scalerotate_inv(zrotzoom) * com->Units - pos;
}

Vec2 Camera::can_scr_p(Vec2 p) {
    return uni_scr(((p + pos) / com->Units).scalerotate(zrotzoom));
}

double Camera::uni_can_s(double s) { return s*com->Units/zoom(); }


void Cameratool::left_up(Vec2 p) {
    mouse = o_mouse = p;
    o_zrotz = view->zrotzoom;
    draw->update();
}

void Cameratool::left_down(Vec2 p) {
    mouse = o_mouse = p;
    o_zrotz = view->zrotzoom;
    myDebug() << "coordinates:" << view->scr_can_p(cursor).x << view->scr_can_p(cursor).y;
    draw->update();
}

void Cameratool::right_down(Vec2 p) {
    mouse = o_mouse = p;
    o_zrotz = view->zrotzoom;
    draw->update();
}

void Cameratool::right_up(Vec2) {
    draw->update();
}

void Cameratool::motion(Vec2 rel) {
    if (left && right)
        view->zrotzoom = o_zrotz * view->scr_uni_p(cursor).length()/view->scr_uni_p(o_mouse).length();
    else if(right)
        view->zrotzoom = Vec2(o_zrotz).scalerotate(view->scr_uni_p(cursor).scalerotate_inv(view->scr_uni_p(o_mouse)));
    else if(left)
        view->pos += view->scr_can_v(rel);
    if(view->zrotzoom.length() < 0.01) view->zrotzoom = view->zrotzoom.unit() * 0.01;
    draw->update();
}

void Cameratool::wheel_rot(double delta) {
    auto o = view->scr_can_p(cursor);
    view->zrotzoom *= pow(1.001, -delta);
    view->pos += view->scr_can_p(cursor) - o;
    draw->update();
}
