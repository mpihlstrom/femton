#include "contour.h"

#include "complex/edge.h"
#include "complex/trigon.h"
#include "complex/node.h"
#include "complex/complex.h"

#include "common/common.h"

Edge* Edge::relcntr(int i) { return (*cntr)[cntr_index + i]; }

void Contour::add(Edge* e) {
    cntr.push_back(e);
    e->cntr = this;
    e->cntr_index = cntr.size() - 1;
}

Edge* Contour::operator[](int i) {
    return cntr[(i + cntr.size() - 1) % cntr.size()];
}

int Contour::sz() { return count(); }
int Contour::count() { return cntr.size(); }

void Concomp::add(Tri* t) {
    ts.push_back(t);
    t->cc = this;
}

//pre-condition: e->j is in cc
void recursive_fill(Concomp* cc, Edge* e) {
    if(e->t->cc != nullptr)  return;
    if(e->line()) return;
    if(e->t->type != Tri::Regular) return;
    cc->add(e->t);
    recursive_fill(cc, e->nxt->j);
    recursive_fill(cc, e->prv->j);
}

void populate_neighbors(Tri* t) {
    for(auto e : *t) {
        auto neighbor_cc = e->j->t->cc;
        if(neighbor_cc == nullptr)
            continue;
        t->cc->neighbors.insert(neighbor_cc);
    }
}

bool contour_recursive(Contour *cntr, Edge* cur, Edge* start) {
    Node* n = cur->nxt->n;
    Edge* nxt = nullptr;
    for(auto ei = n->begin(); ei != n->end(); ++ei) {
        Edge* e = *ei;
        if(nxt == nullptr) {
            if(e->t->cc == start->t->cc && e->j->t->cc != start->t->cc) {
                if(e == start)
                    return true;
                nxt = e;
            }
        }
        else if(e->t->cc != start->t->cc) {
            if(e->j == cur)
                break;
            nxt = nullptr;
        }
    }
    if(nxt == nullptr) //should not happen
        return false;
    cntr->add(nxt);
    return contour_recursive(cntr, nxt, start);
}


void Complex::create_contours() {
    //clean up buffers and pointers
    for(auto c : cntrs)
        delete c;
    cntrs.clear();
    for(auto cc : ccs)
        delete cc;
    ccs.clear();
    for(auto t : trigons) {
        for(auto e : *t)
            e->cntr = nullptr;
        t->cc = nullptr;
    }

    for(auto t : ts) {
        if(t->cc != nullptr)
            continue;
        auto cc = new Concomp();
        cc->add(t);
        for(auto e : *t)
            recursive_fill(cc, e->j);
        ccs.push_back(cc);
    }
    for(auto cc : ccs) {
        for(auto t : cc->ts) {
            cc->area += t->area();
            cc->mid += t->centroid();
        }
        cc->mid /= cc->ts.size();

        Vec3 covar;
        for(auto t : cc->ts) {
            for(auto e : *t) {
                auto d = cc->mid - e->n->p();
                covar += Vec3(d.x*d.x, d.y*d.y, d.x*d.y);
            }

            populate_neighbors(t);
        }
        covar /= (3 * cc->ts.size());
        cc->covar = covar;
        double T = covar.x + covar.y;
        double D = covar.x*covar.y - covar.z*covar.z;
        double sq = sqrt(fabs(T*T/4.0 - D));

        cc->r = covar.z / (sqrt(covar.x)*sqrt(covar.y));
        cc->eigv = Vec2(T/2 + sq, T/2 - sq);
        cc->eigvec1 = Vec2(covar.z, cc->eigv.x - covar.x).unit();
    }

    for(auto t : ts) {
        for(auto e : *t) {
            if(e->cntr == nullptr && e->t->cc != e->j->t->cc) {
                auto cntr = new Contour;
                cntrs.push_back(cntr);
                cntr->add(e);
                auto ret = contour_recursive(cntr, e, e);
                e->t->cc->contours += 1;
                double sur = 0;
                for(auto c : cntr->cntr) {
                    sur += c->v().l2();
                }
                cntr->sur = sur;
                e->t->cc->sur += sur;
            }
        }
    }
}
