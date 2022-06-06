#pragma once

#include "common/common.h"
#include "vector3.h"

#include <iostream>

struct Col : Vec3 {

    static const Col Red, Yellow, Green, Cyan, Blue, Magenta, Black, White, Orange, Gray;

    Col() : a(1.0) {}
    Col(Vec3 const& v) : Vec3(v), a(1.0) {}
    Col(double r, double g, double b, double a) : Vec3(r, g, b), a(a) {}
    Col(double r, double g, double b) : Vec3(r, g, b), a(1.0) {}
    Col(double i);
    Col(Uint32 c);

    Col const& operator=(Col const& s) {
        r = s.r; g = s.g; b = s.b; a = s.a;
        return *this;
    }

    Col h120() const;
    Col inv() const;
    Col semi() const;
    Col mono() const;
    double dist(Col const& b) const;
    Uint32 to32() const;
    double intensity() const;
	double i() const;
	double dist() const;
	
    Col& blend(Col const& s, double q);
    Col& alpha_over(Col const& s) ;
	void setalpha(double a);

	void delta(double d);

	void clamp();
    Col mod();

    bool operator==(Col const& b) const;
    friend std::ostream& operator<<(std::ostream &out, Col &c);

    static void blend(Col &t, Col &s, double w, double f = 1.0);
    static double dist(Col const& f, Col const& s);
    static Col random();
    static Col random_a();

    double a;
};

