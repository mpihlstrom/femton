#include "node.h"
#include "trigon.h"

#include <QDebug>

#include <math.h>

#include "globals.h"


Node::Node() : type(Floating), status(Live), w_(0), contrast(0) {}
Node::Node(Vec2i const& p, Edge* e) : cp(p), pp(p), np(Vec2(p)), entry_(e), type(Floating), status(Live), w_(0), contrast(0) {}
Node::Node(Vec2i const& p, Type t, Col const& c) : cp(p), pp(p), np(Vec2(p)), type(t), status(Live), color(c), w_(0), contrast(0) {}
Node::Node(Vec2i const& p, Edge* e, Col const& c) : cp(Vec2(p)), pp(Vec2(p)), np(Vec2(p)), entry_(e), type(Floating), status(Live), color(c), w_(0), contrast(0) {} //z(p.z) {}

Node::iterator Node::operator++() {
    iterator a;
    entry_ = iterator((Edge*)*++entry_); //resets: first = true
	return entry_;
}

Node::iterator Node::operator--() {
    entry_ = iterator((Edge*)*--entry_); //resets: first = true
	return entry_;
}

Node::iterator Node::begin() const {
    if(type == Border || type == Corner) return border();
    return entry_;
}

Node::iterator Node::end() const {
    if(type == Border || type == Corner) return border();
    return entry_;
}

Edge* Node::setentry(Edge* e) {
	entry_ = iterator(e);
	return *entry_;
}

Edge* Node::border() const {
	iterator i(entry_);
    while(!((*i)->t->type_() == Tri::Regular && (*i)->j->t->type_() == Tri::Padding)) ++i;
	return *i;
}


void Node::swap_p() {
    Vec2i t = cp;
    cp = pp;
    pp = t;
}


Vec2 Node::min_edge() const {
    Vec2 v(0);
    for(iterator i = begin(); i != end(); ++i) {
        auto v1 = Vec2((**i).nxt->n->p_() - p_());
        if(v.l2() <= 0 || v1.l2() < v.l2())
            v = v1;
    }
    return v;
}

double Node::area() const {
    double ar = 0;
    for(iterator i = begin(); i != end(); ++i) {
        ar += (**i).t->area();
    }
    return ar;
}

bool Node::operator<(Node const& s) {
    //return flip_t_acc > s.flip_t_acc;

    if(flip_t == s.flip_t)
        return (np - cp).dot() > (s.np - s.cp).dot();
    return flip_t > s.flip_t;
}

std::tuple<double,double> Node::calc_flip_t() {
    Edge* e = *entry_;
    double min_t1 = 1.0;
    double min_t2 = 1000000000.0;//DBL_MAX;
    Edge* min_e1 = e;
    Edge* min_e2 = e;
    do {
        Vec2i h1h2(e->prv->n->cp - e->nxt->n->cp);
        int64_t num = h1h2^(e->n->np - e->nxt->n->cp);
        double t = 1.0 - (double)num / (double)(h1h2^(e->n->np - e->n->cp));
        if(num <= 0) {
            if (t <= min_t1) {
                min_t1 = t;
                min_e1 = e;
            }
        }
        if(t > 0)
        {
            if (t <= min_t2) {
                min_t2 = t;
                min_e2 = e;
            }
        }
        e = e->snxt();
    } while(e != *entry_);
    return {min_t1, min_t2};//min(1.0, min_t1);
}

double Node::w() const {
    return w_;// c();

/*
    Vec2 gr;
    if(type == Padding)
        return 0.0;
    double wsum = 0.0;
    //Harris type covariance
    int count = 0;
    Vec3 co;
    for(auto e : *this) {
        if(nxtnpadding(e))
            continue;
        double w = e->w;
        wsum += w;
        gr += e->v().norm()*w;

        Vec2 pdif(e->v().norm());
        co += Vec3(pdif.x*pdif.x, pdif.y*pdif.y, pdif.x*pdif.y);
        ++count;
    }

    co /= count;
    double T = co.x + co.y;
    double D = co.x*co.y - co.z*co.z;
    double sq = sqrt(fabs(T*T/4.0 - D));
    double ev1 = T/2 + sq;
    double ev2 = T/2 - sq;

    return w_*(ev1 / (ev1 + ev2));
//	fVector e1 = fVector(co.z, ev1 - co.x).norm();
*/
}

Vec2 Node::g() const {
    return Vec2(1,1);

/*
    Vec2 gr;
    if(type == Padding)
        return gr;
    double wsum = 1.0;
    for(auto e : *this) {
        //if(e->line())
        {
            gr += e->v()*e->w;
            wsum += e->w;
        }
    }
    auto mean = gr / wsum;
    Vec2 gr_v;
    for(auto e : *this) {
        //if(e->line())
        {
            auto dif = e->v().norm()*e->w - mean;
            gr_v += dif*dif;
        }
    }
    gr_v /= wsum;

    Vec2 gr_d = Vec2(sqrt(gr_v.x), sqrt(gr_v.y));
    return gr_d.norm();
*/
}


double Node::c() const {
    return contrast;
}

