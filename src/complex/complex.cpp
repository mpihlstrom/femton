#include "complex.h"
#include "common/color.h"
#include "util.h"
#include "common/common.h"

#include <math.h>
#include <algorithm>

#include <QDebug>

#include <vector>

#include "globals.h"

#define ortsin(a,b) Vector3d::ort_sin((a),(b))

bool Complex::clip(Vec2i& c, Vec2i& p) {
    bool clipped = false;
    if(p.x <= -c.x) { p.x = -c.x; clipped = true; }
    else if(p.x >= c.x) { p.x = c.x; clipped = true; }
    if(p.y <= -c.y) { p.y = -c.y; clipped = true; }
    else if(p.y >= c.y) { p.y = c.y; clipped = true; }
    return clipped;
}

void Complex::clipd(Vec2i c, Vec2& p)
{
    if(p.x < -c.x) p.x = -c.x;
    else if(p.x > c.x) p.x = c.x;
    if(p.y < -c.y) p.y = -c.y;
    else if(p.y > c.y) p.y = c.y;
}

void Complex::move_nodes()
{
    if(move_mode == Concur) move_nodes_concur();
    else if(move_mode == Consec) move_nodes_consec();
}

double intersect(Vec2 a, Vec2 b, Vec2 p, Vec2 q) {
    double n = (b-a)^(a-p);
    double d = (b-a)^(q-p);
    return n / d;
}

Node* Complex::line_split_node(Node* n, Vec2 p1)
{
    if(n->type == Node::Padding)
        return n;
    clipd(_canvas - Vec2(1), p1);
    if(n->cp == p1)
        return n;

    for(auto f : *n) {
        if(f->t->type != Tri::Regular)
            continue;
        auto e = f->nxt;
        double t = intersect(e->n->cp, e->nxt->n->cp, f->n->cp, p1);
        double s = intersect(p1, f->n->cp, e->n->cp, e->nxt->n->cp);

        if(s >= 0.0 && s <= 1.0 && t > 0) {
            if(t < 1.0) {
                Node* m = split_edge(e, (1-s));
                if(m != nullptr) {
                    return line_split_node(m, p1);
                }
                auto np = (Vec2(n->cp) + (p1 - Vec2(n->cp)) * t).round() + ((p1 - Vec2(n->cp)).unit()).round();
                auto u = f->t->p2u(f->n->cp, e->n->cp, e->nxt->n->cp, np);
                if(u.x < u.y && u.x < u.z) {
                    if(n->type == Node::Floating) {
                        move_consec(*n, np);
                        return line_split_node(n, p1);
                    }
                    return n;
                }
                return (u.y < u.x && u.y < u.z)? e->nxt->n : e->n;
            }
            Node* m = split(*e->t, Vec3(1.0/3.0));
            if(m == nullptr)
                return n;
            move_consec(*m, p1);
            return m;
        }
    }
    return nullptr;
}




Node* Complex::split(Tri& t, Vec2 coord) {
    return split(t, Vec3(coord, 1.0-coord.x-coord.y));
}

/*       c1 o                            a3 o b2
           / \                             /|\
          /   \                           / | \
         /     \                         /  |  \
        /       \                       /   |   \
       /         \                     /    |c2  \
      /           \       ----->      /  c3.o.    \
     /             \                 /   .´ c1`.   \
    /               \               /  .´       `.  \
   /                 \             / .´           `. \
  /                   \        b3 /.´               `.\ a2
 o---------------------o b1      o´-------------------`o
a1                               a1                  b1      */
Node* Complex::split(Tri &t, Vec3 u) {
    Node &na = *t.a.n, &nb = *t.b.n, &nc = *t.c.n;

    auto p = Vec2(t.a.n->p()*u.x + t.b.n->p()*u.y + t.c.n->p()*u.z);
    Vec2i p2 = p.round();

    const int Min_ar = 0;
    int64_t ar1, ar2, ar3;
    if((ar1 = Vec2i::area(na.cp, nb.cp, p2)) <= Min_ar || (ar2 = Vec2i::area(nb.cp, nc.cp, p2)) <= Min_ar || (ar3 = Vec2i::area(nc.cp, na.cp, p2)) <= Min_ar)
        return nullptr;

    Edge* e = &t.a;

    double nmw =  t.a.n->w_ * u.x + t.b.n->w_ * u.y + t.c.n->w_ * u.z;
    double nmcontrast =  t.a.n->contrast * u.x + t.b.n->contrast * u.y + t.c.n->contrast * u.z;
    Vec2 nmv =  t.a.n->v * u.x + t.b.n->v * u.y + t.c.n->v * u.z;

    Node &nm = *(new Node(p2, e->prv, Col(na.color*u[0] + nb.color*u[1] + nc.color*u[2])));
    e->prv->n = &nm; //re-assign node c to the created node
    Tri &t1 = t,
           &t2 = *(new Tri(&nb, e->nxt->j, &nc, nullptr, &nm, e->nxt, e->t->type, t1.color)),
           &t3 = *(new Tri(&nc, e->prv->j, &na, e->prv, &nm, &t2.b, e->t->type, t1.color));
    t1.id = ++gid; //though only a transmutation of trigon t, the trigon t1 should take on a new identity.
    if(nb.entry() ==  e->nxt) nb.setentry(&t2.a); //this will ensure entry nodes on
    if(nc.entry() ==  e->prv) nc.setentry(&t3.a); //border edges for border nodes.
    nm.w_ = nmw;
    nm.contrast = nmcontrast;
    nm.v = nmv;

    Col t1ac = t1.a.c;
    Col t1bc = t1.b.c;
    Col t1cc = t1.c.c;

    t2.a.c = t1bc;
    t2.b.c = t1cc;
    //Color mix = t1ac*(u.x+u.y-u.z) + t1bc*(u.y+u.z-u.x) + t1cc*(u.z+u.x-u.y);
    Col mix = t1ac*u.x + t1bc*u.y + t1cc*u.z;
    mix.a = 0;
    t2.c.c = mix;

    t3.a.c = t1cc;
    t3.b.c = t1ac;
    t3.c.c = mix;

    t1.c.c = mix;

    t2.a.w = t1.b.w;
    t3.a.w = t1.c.w;
    t1.b.w = t1.c.w = t2.b.w = t3.b.w = 0;

    t1._area = ar1; t2._area = ar2; t3._area = ar3; //set as a bonus.

    trigons.push_back(&t2);
    trigons.push_back(&t3);
    nodes.push_front(&nm);
    return &nm;
}

Node* Complex::split_edge(Tri &t, Vec2 u2) {
    double f;
    Edge const* e;
    t.u_2_e_q(Vec3(u2, 1-u2.x-u2.y), &e, &f);
    return split_edge(e, f);
}

/*     s                     s
      / \                   /|\
     /   \                 / | \
    /     \               / c2  b2
   /       \             /   |   \
  /         \           /    |    \
 p-----e---->r  ---->  p--e--o-a2--r
  \         /           \    | c4 /
   \       /             \   |   /
    \     /               \ a4  b4
     \   /                 \ | /
      \ /                   \|/
       q                     q

Splits a pair along a given edge.
calculates and stores trigon areas.
maintains entry edges within the trigon pair that is split. */
Node* Complex::split_edge(Edge const* e, double f) {
    if(e->nxt->n->type == Node::Padding)
        return nullptr;
    if(isnan(f) || isinf(f))
        return nullptr;

    Edge* j = e->j;

    Vec2 op = (e->n->p() * f + e->nxt->n->p() * (1-f)).round();
    Vec2i opi = op;

    Node *p = e->n, *q = j->prv->n, *r = j->n, *s = e->prv->n;

    const int64_t ma = 0;
    //Pre-calculate areas (successful split).
    int64_t ar1, ar2, ar3, ar4;
    if((ar1 = Vec2i::area(p->p_(), opi, s->p_())) <= ma || (ar2 = Vec2i::area(opi, r->p_(), s->p_())) <= ma || (ar3 = Vec2i::area(p->p_(), q->p_(), opi)) <= ma || (ar4 = Vec2i::area(opi, q->p_(), r->p_())) <= ma)
        return nullptr;

    double ow = e->n->w_ * f + e->nxt->n->w_ * (1-f);
    double ocontrast = e->n->contrast * f + e->nxt->n->contrast * (1-f);

    Vec2 pv = p->v*f;
    Vec2 rv = r->v*(1-f);
    Vec2 ov = pv + rv;

    Node* o = new Node(opi, e->nxt, p->color*(1-f) + r->color*f);
    Tri *t1 = e->t,
           *t2 = new Tri(o, nullptr, r, e->nxt->j, e->prv->n, e->nxt, t1->type, t1->color);
    e->nxt->n = o;
    if(r->entry() ==  e->nxt) r->setentry(&t2->b);
    o->setentry(&t2->a);
    if(epadding(e) || jpadding(e)) o->type = Node::Border;
    j->n = o;
    Tri *t3 = j->t,
        *t4 = new Tri(o, j->prv, q, j->prv->j, r, &t2->a, t3->type, t3->color);
    if(r->entry() == j) r->setentry(&t4->c);
    if(q->entry() == j->prv) q->setentry(&t4->b);
    o->w_ = ow;
    o->contrast = ocontrast;

    o->v = ov;

    t2->a.w = e->w;
    t2->b.w = e->nxt->w;
    t4->c.w = j->w;
    t4->b.w = j->prv->w;
    e->nxt->w = j->prv->w = 0;

    t3->_area = ar3;
    t4->_area = ar4;
    t1->_area = ar1;
    t2->_area = ar2;

    trigons.push_back(t2);
    trigons.push_back(t4);
    nodes.push_front(o);
    return o;
}

void Complex::randomize()
{
    for(auto t : ts)
        t->color = Col::random();
}

Tri* Complex::inside(Vec2i const& p) const {
    for(Trigons::const_iterator i = trigons.begin(); i != trigons.end(); ++i) {
        Tri& t = **i;
        if(t.invalid()) continue;
        if(t.inside(p)) return &t;
    }
    return nullptr;
}

void Complex::waste() {
    for(Trigons::iterator it = trigons.begin(); it != trigons.end();) {
        if((*it)->status == Tri::Dead) {
            delete *it;
            it = trigons.erase(it);
            continue;
        }
        ++it;
    }

    for(Nodes::iterator it = nodes.begin(); it != nodes.end();) {
        if((*it)->status == Node::Dead) {
            delete *it;
            it = nodes.erase(it);
            continue;
        }
        ++it;
    }
}

Complex::~Complex() {
    for(Nodes::iterator i = nodes.begin(); i != nodes.end(); ++i) delete (*i);
    for(Trigons::iterator i = trigons.begin(); i != trigons.end(); ++i) delete (*i);
}

