#pragma once

#include <vector>
#include <set>

#include "common/vector.h"
#include "common/vector3.h"
struct Tri;
struct Edge;

struct Contour {
    void add(Edge* e);
    Edge* operator[](int i);
    int sz();
    int count();
    std::vector<Edge*> cntr;
    double sur = 0.0;
};


struct Concomp {
    void add(Tri*);
    std::vector<Tri*> ts;
    int64_t area;
    Vec2 mid;
    double cos;
    std::set<Concomp*> neighbors;
    Vec3 covar;
    Vec2 eigv;
    Vec2 eigvec1;
    Vec2 eigvec2;
    double r;
    int contours = 0;
    double sur = 0.0;
};

