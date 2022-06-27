#pragma once
#include "common/vector.h"
#include "common/vector3.h"
#include "common/common.h"

struct Angle {
    Angle() : angle_(0.) {}
    Angle(Angle const& a) : angle_(a()) {}
    Angle(double angle) : angle_(angle) {}

    static double rad_deg(double r) {
        return ((r / (2*M_PI)) * 360.0);
    }

    static double deg_rad(double d) {
        return ((d / 360.0) * (2*M_PI));
    }

    static double fmod360(double angle) {
        if(angle > 360.) angle -= 360.*((int)angle % 360);
        else if(angle < 0.) angle += 360.*((int)-angle % 360 + 1);
        return angle;
    }

    Angle const& operator=(double angle) {
        angle_ = fmod360(angle);
        return *this;
    }

    Angle const& operator+=(double angle) {
        angle_ = fmod360(angle_ + angle);
        return *this;
    }

    Angle const& operator-=(double angle) { return operator+=(-angle); }

    double operator()() const { return angle_; }

private:
    double angle_;
};

struct Camera {
    Camera(Vec2 p);
	Camera();
    void reset();
    double uni_can_s(double s);
    Vec2 uni_scr(Vec2 p);
    Vec2 uni_can_v(Vec2 v);
    Vec2 scr_uni_v(Vec2 v);
    Vec2 scr_can_v(Vec2 v);
    Vec2 scr_uni_p(Vec2 p);
    Vec2 scr_can_p(Vec2 p);

    Vec2 can_scr_p(Vec2 p);

    double zoom() const { return zrotzoom.length(); }
    double rotz() const;

    Vec2 zrotzoom;
    Vec2 pos;
};


