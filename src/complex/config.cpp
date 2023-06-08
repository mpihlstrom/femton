#include "complex.h"
#include "common/common.h"
#include "globals.h"
#include "util.h"
#include "json/json.h"

#include <iostream>
#include <string>

Complex::Move Complex::str_to_move(std::string str) {
    if (str == "Concur" || str == "concur") return Concur;
    if (str == "Pseudo" || str == "pseudo") return Pseudo;
    if (str == "Consec" || str == "consec") return Consec;
    return Consec;
}

Complex::Complex(bool configure) :
    Units(1024*4),
    color_dist_th(0.25/255.0),
    ev_quant(0.0025 * Units),
    ar_quant(ev_quant*ev_quant * 1),
    purge_cos_thresh(-0.99995),
    ns(&nodes),
    ts(&trigons),
    move_mode(Concur),
    cross_counter(0),
    curt(1),
    _canvas(Units, Units) {

    if(!configure)
        return;

    Node *a = new Node(Vec2i(-_canvas.x, -_canvas.y), Node::Corner, Col::random()),
         *b = new Node(Vec2i(_canvas.x,  -_canvas.y), Node::Corner, Col::random()),
         *c = new Node(Vec2i(-_canvas.x,  _canvas.y), Node::Corner, Col::random()),
         *d = new Node(Vec2i(_canvas.x,   _canvas.y), Node::Corner, Col::random()),
         *ab = new Node(Vec2i(0, -_canvas.y*3), Node::Padding, Col::random()),
         *bd = new Node(Vec2i(_canvas.x*3,  0), Node::Padding, Col::random()),
         *dc = new Node(Vec2i(0,  _canvas.y*3), Node::Padding, Col::random()),
         *ca = new Node(Vec2i(-_canvas.x*3, 0), Node::Padding, Col::random());

    Tri *t = new Tri(a, b, c, Tri::Regular, Col::random()),
        *u = new Tri(d, nullptr, c, &t->b, b, nullptr, Tri::Regular, Col::random()),
        *ab_t = new Tri(b, &t->a, a, nullptr, ab, nullptr, Tri::Padding),
        *bd_t = new Tri(d, &u->c, b, nullptr, bd, nullptr, Tri::Padding),
        *dc_t = new Tri(c, &u->a, d, nullptr, dc, nullptr, Tri::Padding),
        *ca_t = new Tri(a, &t->c, c, nullptr, ca, nullptr, Tri::Padding),
        *x = new Tri(ab, nullptr, bd, &bd_t->b, b, &ab_t->c, Tri::Padding),
        *y = new Tri(bd, nullptr, dc, &dc_t->b, d, &bd_t->c, Tri::Padding),
        *z = new Tri(dc, nullptr, ca, &ca_t->b, c, &dc_t->c, Tri::Padding),
        *w = new Tri(ca, nullptr, ab, &ab_t->b, a, &ca_t->c, Tri::Padding);

    a->setentry(&t->a); b->setentry(&u->c); c->setentry(&t->c); d->setentry(&u->a);
    ab->setentry(&x->a); bd->setentry(&y->a); dc->setentry(&z->a); ca->setentry(&w->a);

    t->a.c = Col(1,0,0);
    t->b.c = Col(0,1,0);
    t->c.c = Col(0,0,1);
    u->a.c = Col(1,1,0);
    u->b.c = Col(0,1,1);
    u->c.c = Col(1,0,1);

    ab_t->color = bd_t->color = dc_t->color = ca_t->color = x->color = y->color = z->color = w->color = Col(0,0,0);

    t->b.w = 1;
    u->b.w = 1;
    a->w_ = b->w_ = c->w_ = d->w_ = 0;
    a->contrast = b->contrast = c->contrast = d->contrast = ab->contrast = bd->contrast = dc->contrast = ca->contrast = 1;

    //u->color = t->color = Col::random();

    c->contrast = 1;
    u->color = Col::random();
    t->color = u->color;// * (1.+5./255.);

    nodes.push_back(a); nodes.push_back(b); nodes.push_back(c); nodes.push_back(d);
    nodes.push_back(ab); nodes.push_back(bd); nodes.push_back(dc); nodes.push_back(ca);
    trigons.push_back(t); trigons.push_back(u);
    trigons.push_back(ab_t); trigons.push_back(bd_t); trigons.push_back(dc_t); trigons.push_back(ca_t);
    trigons.push_back(x); trigons.push_back(y); trigons.push_back(z); trigons.push_back(w);

    delaunify();
    randomize();

}
