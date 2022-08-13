#include "complex.h"
#include "common/color.h"
#include "util.h"
#include "common/common.h"

#include <math.h>
#include <algorithm>

#include <QDebug>

#include <vector>

#include "globals.h"

bool Complex::merge(Edge* e)
{
    //todo: make sure that merge is not done across the canvas??
    return move(*e->n, e->nxt->n->cp);
}

bool Complex::move(Node &n, Vec2i np) {
    if(n.invalid()) return false;
    if(n.type == Node::Corner) return false;

    if(n.type == Node::Border) {
        Vec2 b = n.border()->nxt->n->cp - n.cp;
        //move p as projected onto the border
        np = n.cp + Vec2i((b*b.dot(Vec2(np - n.cp))/b.dot(b)));
    }

    clip(_canvas, np);
    if(np == n.cp)
        return false;

    n.np = np;
    mv_nodes.push_back(&n);
    return true;
}

#define eq_o(o, t, c) ((t)->cross != nullptr && (t)->cross->overlap == (o) && ((t)->cross->crossroot - c->crossroot == 0))

Edge* detach_o1o3(Edge* e, Edge* stop, Node* m, Cross* c)
{
    Edge *o1 = e->j, *i, *o; //1-overlap, inner, outer

    o1->t->retire();
    o1->n = o1->nxt->n = o1->prv->n = e->n; //collapse to 3-overlap polygon

    for(i = e; eq_o(3, i->t, c); i = i->snxt())
        i->t->cross->detached = true;

    for(o = i; o != o1->nxt; o = o->snxt())
        if(o->j != stop)
            o->n = m;

    o1->prv->j->link(o1->nxt->j);

    if(i->j == stop)
        return o1->prv->j;

    return detach_o1o3(i->j, stop, m, c);
}

void Complex::detach(Cross* c)
{
    c->detached = true;
    for(auto e : *c->t) {
        if(!eq_o(1, e->j->t, c))
            continue;

        Node* n = new Node(*e->nxt->n);
        auto entry = detach_o1o3(e, e, n, c);
        n->setentry(entry->sprv());
        nodes.push_front(n);
        mv_nodes.push_front(n);

        for(auto i = n->begin(); i != n->end(); ++i) {
            (*i)->nxt->n->setentry((*i)->nxt);
            cross_case((*i)->t, c);
        }

        break;
    }
}

void Complex::push_cross(Cross* c) {
    mv_tris.push_back(c);
    push_heap(mv_tris.begin(), mv_tris.end(), Cross::lt);
}

Cross* Complex::pop_cross() {
    auto c = mv_tris.front();
    pop_heap(mv_tris.begin(), mv_tris.end(), Cross::lt);
    mv_tris.pop_back();
    return c;
}

void Complex::move_nodes_concur()
{
    for(auto n : mv_nodes)
        n->cp = n->np;
    for(auto n : mv_nodes)
        for(auto e : *n)
            cross_case(e->t);

    for(Tri::Cross* c; !mv_tris.empty();) {
        c = pop_cross();

        if(!c->invalid) {
            if(c->overlap >= 2) {
                if(c->iter == 0) {
                    if(!c->detached) detach(c);
                    ++c->iter;
                    push_cross(c);
                    continue; //do not delete
                }
                c->t->retire_w_ns();
            } else if(c->overlap == 1) {
                remove(c);
            } else {
                Edge *e = c->flip_e, *j = e->j;
                flip(e);
                cross_case(e->t, c);
                cross_case(j->t, c);
            }
        }

        delete c;
    }

    set_node_type();

    for(auto *n : mv_nodes)
        n->pp = n->cp;

    mv_tris.clear();
    mv_nodes.clear();
    cross_counter = 0;
}

bool Complex::cross_case(Tri* t, Tri::Cross* c)
{
    if(t->cross != nullptr) {
        if(c == nullptr) return false; //at init, don't replace any previous cross case
        else t->cross->invalid = true; //keep it on the heap, but de-activate it
    }

    t->cross = t->create_cross(c);
    if(t->cross == nullptr)
        return false;

    push_cross(t->cross);
    return true;
}

void Complex::set_node_type()
{
    for(auto n : mv_nodes) {
        if(n->cp.x == -_canvas.x || n->cp.x == _canvas.x) n->type = Node::Border;
        if(n->cp.y == -_canvas.y || n->cp.y == _canvas.y) n->type = Node::Border;

        if(n->cp.x == -_canvas.x && n->cp.y == -_canvas.y) n->type = Node::Corner;
        if(n->cp.x == -_canvas.x && n->cp.y ==  _canvas.y) n->type = Node::Corner;
        if(n->cp.x ==  _canvas.x && n->cp.y == -_canvas.y) n->type = Node::Corner;
        if(n->cp.x ==  _canvas.x && n->cp.y ==  _canvas.y) n->type = Node::Corner;
    }
}

/*
 O = vertex

 O--------.
 |  ,---.  \
 | /     \ |
 | |  O  | |
 | |   \j| /
 | \   e\|/
 \  `----O
  `-----´
*/

bool Complex::remove_self_folded_tri(Edge* e)
{
    Edge* j = e->j;
    if (!(j == e->nxt && e->n == e->prv->n))
        return false;

    e->nxt->n->status = Node::Dead;
    e->t->retire();
    e->prv->j->t->retire();

    e->prv->j->nxt->j->link(e->prv->j->prv->j);
    e->n->setentry(e->prv->j->snxt());
    e->prv->j->prv->n->setentry(e->prv->j->prv->snxt());

    return true;
}

/*
 O = vertex

  O----.
  |\    \
  | \   |
  |  O  |
  |   \j|
  \   e\|
   `----O
*/

bool Complex::remove_double_linked_tri(Edge* e)
{
    Edge* j = e->j;
    if(e->nxt->j->nxt != j)
        return false;

    e->nxt->n->status = Node::Dead;
    e->t->retire();
    j->t->retire();

    e->prv->j->link(j->nxt->j);
    e->n->setentry(e->snxt());
    e->prv->n->setentry(e->prv->sprv());

    return true;
}

//Removes a pair of trigons sharing the edge e. The node e->n will be preserved
//while e->nxt->n is made Dead. Affected node entries are adjusted accordingly.
Node* Complex::remove(Cross* c)
{
    Edge* e = c->flip_e;
    Node* n = e->n;
    Node* s = e->nxt->n;
    auto j = e->j;
    auto et = e->t;
    auto jt = j->t;

    if(remove_self_folded_tri(e) || remove_self_folded_tri(e->nxt) || remove_self_folded_tri(e->prv))
        return nullptr;

    if(remove_double_linked_tri(e) || remove_double_linked_tri(j))
        return nullptr;

    Edge *eprv = e->prv, *enxt = e->nxt;
    Edge *jprv = j->prv, *jnxt = j->nxt;
    for(auto i = jprv->j; i != enxt; i = i->snxt()) {
        i->n = n;
        cross_case(i->t, c);
    }

    n->setentry(eprv->j);

    //Change entry nodes if they are pointing to trigons to be deleted.
    if(eprv->n->entry() == eprv) eprv->n->setentry(enxt->j);
    if(jprv->n->entry() == jprv) jprv->n->setentry(jnxt->j);

    enxt->j->link(eprv->j);
    jnxt->j->link(jprv->j);

    et->retire();
    jt->retire();

    s->status = Node::Dead;
    return n;
}

/*
    |                |
    |                |
    V                V
q  .d.  r        q--.d.--r
 .´   `.         |.´#|#j.|
a---e-->c  --->  a###|###c
|\#####/|        |\##|##/|
| \###/ |        | e#|#/ |
|  \#/  |        |  \|/  |
t---b---s        t---b---s

Flipping a trigon pair and maintains entry edges within the trigon pair. */
void Complex::flip(Edge* e) {
    Edge* j = e->j;

    double acw = e->w, caw = j->w, abw = j->nxt->w, cdw = e->nxt->w;

    e->t->type = j->t->type;
    e->t->color = j->t->color;

    if(j->n->entry() == e->nxt) ++*j->n;
    e->nxt->n = j->prv->n;
    if(e->n->entry() == j->nxt) ++*e->n;
    j->nxt->n = e->prv->n;
    e->link(j->nxt->j);
    j->link(e->nxt->j);
    e->nxt->link(j->nxt);

    *e->prv->j |= acw;
    *j->j |= acw;
    *j |= cdw;
    *e->prv |= caw;
    e->w = abw;
    e->nxt->w = j->nxt->w = 0;
}
