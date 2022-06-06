#include "edge.h"
#include "node.h"

Edge::Edge() : n(nullptr), j(nullptr), nxt(nullptr), prv(nullptr), t(nullptr), w(0) {}
Edge::Edge(Edge const& e) : n(e.n), j(e.j), nxt(e.nxt), prv(e.prv), t(nullptr), w(0) {}

Edge::Edge(Node* en, Edge* ej, Edge* nxt, Edge* prv, Tri* t)
    : n(en), j(ej), nxt(nxt), prv(prv), t(t), w(0) {
	if(j) j->j = this; //This is always true and the operation must always be done.
    c = Col(1,0,0,1);//random();
}

void Edge::link(Edge*e) {
	j = e;
	if(j) j->j = this;
}

Vec2 Edge::v() const {
    return Vec2(nxt->n->cp - n->cp);
}

double Edge::ang() const {
    Vec2 a(nxt->n->p_()-n->p_()),
             b(prv->n->p_()-n->p_());
    return asin(a.unit()^b.unit()) / (2*M_PI);
}

double Edge::cos() const {
    Vec2 a(nxt->n->p_()-n->p_()),
             b(prv->n->p_()-n->p_());
    return a.unit().dot(b.unit());
}

double Edge::sin() const {
    Vec2 a(nxt->n->p_()-n->p_()),
         b(prv->n->p_()-n->p_());
    return a.unit()^b.unit();
}

/*Vec3 Edge::p2u(Vec2 const& q) const {
    double ar_pbc = (prv->n->p_() - nxt->n->p_()) ^(q - nxt->n->p_()),
           ar_apc = (n->p_() - prv->n->p_())^(q - prv->n->p_()),
           ar_abp = (nxt->n->p_() - n->p_())^(q - n->p_()),
           ar_abc = (nxt->n->p_() - n->p_())^(prv->n->p_() - n->p_());
    return Vec3(ar_pbc,ar_apc,ar_abp)/ar_abc;
}*/

void Edge::operator^=(double bw) {
    if(this->w < 1 && this->w < bw) this->w = bw;
    else if(this->w > 0 && bw > 0) this->w = 0;
}

void Edge::operator|=(double bw) {
    if(this->w > 0 || bw > 0) this->w = 1;
    //else if(this->w > 0 && bw > 0) this->w = 0;
}

bool Edge::line() const {
    return w > 0;
}




bool Edge_iterator::operator!=(Edge_iterator const& i) const {
    return  e != i.e || first == i.first;
}

bool Edge_iterator::operator==(Edge_iterator const& i) const {
    return e == i.e && first != i.first;
}


Edge_iterator const&  Node_iterator::operator++() {
    e = e->prv->j;
    first = false;
    return *this;
}

Edge_iterator const&  Node_iterator::operator--() {
    e = e->j->nxt;
    first = false;
    return *this;
}

Edge_iterator const&  Trigon_iterator::operator++() {
    e = e->nxt;
    first = false;
    return *this;
}

Edge_iterator const&  Trigon_iterator::operator--() {
    e = e->prv;
    first = false;
    return *this;
}





