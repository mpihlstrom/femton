#include "globals.h"
#include "complex/complex.h"

void Complex::color_to_line() {
    for(auto t : ts) {
        for(auto e : *t) {
            auto cdist = e->t->color.dist(e->j->t->color);
            if(cdist  < color_dist_th)
                e->w = e->j->w = 0;
            else if(jpadding(e))
                e->w = e->j->w = 1.0;
            else
                e->w = e->j->w = cdist;
        }
    }
}

void Complex::purge_nonlines()
{
    for(auto n : ns) {
        for(auto e :*n)
            if(e->line())
                goto no_merge;
        {
            //myDebug() << "nonline merge";
            if(n->type == Node::Border) merge(n->border());
            else merge(n->entry());
        }
        no_merge:;
    }
    move_nodes();
}

void Complex::purge_small_trigons()
{
    for(auto n : ns) {
        for(auto e : *n) {
            if(e->t->area() < ar_quant && e->v().l2() < ev_quant) {
                merge(e);
                break;
            }
        }
    }
    move_nodes();
}

void Complex::purge_stars()
{
    Nodes mv_ns;

    uint32_t* buffer;
    if(draw->renders["painting"]->active)
        buffer = ((Surface_render*)draw->renders["painting"])->fbo_buffer(Base_render::FBO_buffer::Color, false);
    else
        buffer = ((Surface_render*)draw->renders["gaussian_v"])->fbo_buffer(Base_render::FBO_buffer::Color, false);

    auto r = Rect(scr_sz);
    for(auto n : ns) {
        double min_vdist = _canvas.x, max_cdist = 0, min_cdist = 1;
        Edge* merge_e = n->entry();
        auto np1 = n->p();
        Vec2i p_scr = view->can_scr_p(np1);
        if(!r.contains(p_scr))
            continue;
        Col col1(*(buffer + (r.h-1-p_scr.y)*r.w + p_scr.x));

        for(auto e : *n) {
            if(e->nxt->n->type != Node::Floating)
                continue;
            auto np2 = e->nxt->n->p();
            p_scr = view->can_scr_p(np2);
            if(!r.contains(p_scr))
                continue;
            auto col2 = Col(*(buffer + (r.h-1-p_scr.y)*r.w + p_scr.x));

            double vdist = e->v().l2();
            double cdist = col1.dist(col2);// * (np1 - np2).l2() / ev_quant;
            if(cdist > max_cdist)
                max_cdist = cdist;
            if(cdist < min_cdist)
                min_cdist = cdist;
            if(vdist < min_vdist) {
                min_vdist = vdist;
                merge_e = e;
            }
        }

        if(max_cdist < color_dist_th*5)// && min_vdist < ev_quant)
        {
            merge(merge_e);
        }
    }

    move_nodes();
}

void Complex::purge() {
    //purge_spikes();
    //purge_straight_lines();

    purge_nonlines();
    purge_small_trigons();
    //purge_stars();
}



void Complex::purge_straight_lines()
{
    for(auto n : ns) {
        Edge *pri = nullptr, *sec = nullptr, *ter = nullptr;
        for(auto e : *n) {
            if(!e->line())
                continue;
            if(pri == nullptr) pri = e;
            else if(pri != nullptr) sec = e;
            else {
                ter = e;
                break;
            }
        }
        if(sec != nullptr && ter == nullptr && (pri->v().unit() & sec->v().unit()) * (ev_quant/pri->v().l2()) < purge_cos_thresh) {
            merge(pri);
        }
    }
    move_nodes();
}

void Complex::purge_spikes()
{
    double sin60 = sqrt(3.)/2.;
    Nodes mv_ns;
    for(auto n : ns) {
        Edge* purge = nullptr;
        for(auto e : *n) {
            if(e->line() && e->sin() < sin60*0.01)
                purge = e;
        }
        if(purge != nullptr)
            merge(purge);
    }
    move_nodes();
}




void Complex::refract()
{
    for(auto t : ts) {
        if(t->type != Tri::Regular)
            continue;
        for(auto e : *t) {
            if(e->line()) {
                if(e->v().l2() > ev_quant*10) {
                    split_edge(e, 0.5);
                }
            }
        }
    }
}


void color_to_line(Node* n) {
    for(auto e : *n) {
        auto cdist = e->t->color.dist(e->j->t->color);
        if(cdist  < com->color_dist_th)
            e->w = e->j->w = 0;
        else if (jpadding(e))
            e->w = e->j->w = 1.0;
        else
            e->w = e->j->w = cdist;
    }
}

Node* checkremove_nonline(Node* n) {
    color_to_line(n);
    for(auto e :*n) {
        if(e->line()) return nullptr;
    }
    Edge* merge_e = (n->type == Node::Border)? n->border() : n->entry();
    com->move_consec(*n, merge_e->nxt->n->cp);
    return checkremove_nonline(n);
}

void purge_nonlines_recursive() {
    for(auto n : com->ns) {
        checkremove_nonline(n);
    }
}

void recursive_fill(Concomp* cc, Edge* e) {
    if(e->t->type != Tri::Regular) { return; }
    if(e->line())  { return; }
    if(e->t->cc != nullptr)  { return; }
    cc->ts.push_back(e->t);
    e->t->cc = cc;
    recursive_fill(cc, e->nxt->j);
    recursive_fill(cc, e->prv->j);
}

void create_ccs() {
    for(auto cc : com->ccs)
        delete cc;
    com->ccs.clear();
    for(auto t : com->trigons)
        t->cc = nullptr;
    for(auto t : com->ts) {
        if(t->cc != nullptr)
            continue;
        auto cc = new Concomp();
        cc->ts.push_back(t);
        t->cc = cc;
        for(auto e : *t)
            recursive_fill(cc, e->j);
        com->ccs.push_back(cc);
    }
    for(auto cc : com->ccs) {
        auto col = Col::random();
        for(auto t : cc->ts) {
            cc->area += t->area();
            cc->mid += t->centroid();//*t->area();
            for(auto e : *t) {
                if(e->line())  {
                    cc->peri += e->v().l2();
                }
            }
        }
        cc->mid /= cc->ts.size();





        /*
        double l2acc = 0.0;
        double cos = 0.0;

        int count = 0;
        Edge* h, *p;
        col = Col::random();
        Edge* e = &cc->ts.front()->a;
        Edge* start = e;
        do {
            auto broke = false;
            for(auto f : *e->n) { h = f; if(h->t->cc == cc && h->j->t->cc != cc) { broke = true; break;  } }
            if(!broke) break;
            if(count == 0) start = h;
            else {
                //cos += (p->v().unit() & e->v().unit() + 1.0) / 2.0;
                cos += p->v().unit() ^ e->v().unit();
            }

            l2acc += h->v().l2();

            ++count;
            if(count > cc->ts.size()*2 + 1) break;
            p = h;
            e = h->nxt;
        } while(!((h == start) && count != 1));

        cc->cos = cos/count;
        cc->peri = l2acc;
        */
    }
}


bool Complex::automata()
{
    purge_nonlines_recursive();
    color_to_line();
    refract();
    color_to_line();
    create_ccs();


    for(auto n : ns) {
        if(n->type != Node::Floating) continue;
        Vec2 v;
        int count = 0;
        double wsum = 0.0;

        count = 0;
        for(auto e : *n) {
            if(e->line()) ++count;
        }

        if(count <= 1) continue;

        for(auto e : *n) {
            if(!e->line()) continue;
            Edge* f = e->snxt();
            for(; f != e; f = f->snxt()) {
                if(f->line()) continue;

                double w = (1.0 - (e->v().unit() & f->v().unit()))*0.5;
                //if(!e->line()) w = pow(w,0.21);
                wsum += w;//
                v += (f->v() + e->v())*w;
                break;
            }
        }

        //v.norm(); v *= com->ev_quant * 0.25;
        v /= wsum; v *= 0.5;

        move(*n, n->cp + v);
    }

/*
    for(auto cc : com->ccs) {
        auto col = Col::random();
        for(auto t : cc->ts) {
            for(auto e : *t) {
                if(e->line())
                {
                    Vec2 v = e->v()*0.5;
                    //else v -= (m - n->cp);
                    //v += e->v()*e->t->cc->area;
                    move(*e->n, e->n->cp + v);
                }
            }
        }
        move_nodes();
    }
*/

    /*
    for(auto n : ns) {
        if(n->type != Node::Floating) continue;
        Vec2 v;
        int count = 0;
        double wsum = 0.0;

        count = 0;
        for(auto e : *n) {
            if(e->line())
            {
                double aa = abs(e->j->t->cc->area - e->t->cc->area);
                //double aa = (e->j->t->cc->peri - e->t->cc->peri);
                if(aa <= 0) continue;
                auto w = pow(aa, -2);
                wsum += w;//
                v += Vec2(e->t->cc->mid + e->j->t->cc->mid - e->n->cp - e->n->cp).round()*-w;
                ++count;
            }
        }

        if(count <= 0) continue;

        //v.norm(); v *= com->ev_quant * 0.5;
        v /= wsum; v *= 0.005;

        //n->v += v*0.01;
        move(*n, n->cp + v);
    }

    for(auto n : ns) move(*n, n->cp + n->v);
    */


/*
    for(auto n : ns) {
        //if(n->type == Node::Padding) n->v = 0;//continue;
        if(n->type == Node::Corner || n->type == Node::Padding) n->v = 0;//continue;
        if(n->type == Node::Border && (n->cp.x == com->_canvas.x || n->cp.x == -com->_canvas.x)) n->v.x = 0;
        if(n->type == Node::Border && (n->cp.y == com->_canvas.y || n->cp.y == -com->_canvas.y)) n->v.y = 0;

        Vec2 acc;

        int count = 0;
        for(auto e : *n) {
            //if(e->nxt->n->type == Node::Padding) continue;
            //if(!e->line()) continue;

            auto v = Vec2(e->nxt->n->cp - n->cp);
            auto mag = v.l2();
            double a = 100035*com->ev_quant / pow(v.dot(),2);
            double acceleration = (mag > 2.0*com->ev_quant)? a : 0;
            if(!e->line()) acceleration *= 0;//1.5;

            acc += v.unit() * acceleration;
            ++count;
        }
        if(count) n->v += acc / count;
        n->v += acc;

    }

    for(auto n : ns) move(*n, n->cp + n->v);
*/



    move_nodes();

    color_to_line();
    //purge_nonlines();

    //delaunify();

    return true;
}

/*Delaunay stuff*/

bool Complex::jt_inside_circumcircle(Edge* e) {
    Vec2i a(e->t->a.n->cp), b(e->t->b.n->cp), c(e->t->c.n->cp), d(e->j->prv->n->cp), d2(d*d), a2(a*a), b2(b*b), c2(c*c);
    return (a.x-d.x)*(b.y-d.y)*((c2.x-d2.x)+(c2.y-d2.y)) +
           (a.y-d.y)*((b2.x-d2.x)+(b2.y-d2.y))*(c.x-d.x) +
           ((a2.x-d2.x)+(a2.y-d2.y))*(b.x-d.x)*(c.y-d.y) -
           (a.x-d.x)*((b2.x-d2.x)+(b2.y-d2.y))*(c.y-d.y) -
           (a.y-d.y)*(b.x-d.x)*((c2.x-d2.x)+(c2.y-d2.y)) -
           ((a2.x-d2.x)+(a2.y-d2.y))*(b.y-d.y)*(c.x-d.x) > 0;
}

bool Complex::constrained(Edge* e) {
    return e->line();
}

bool Complex::flip_delaunay(Edge* e, bool constrained) {
    if(padding(e) || jpadding(e) || (constrained && this->constrained(e)) || !jt_inside_circumcircle(e))
        return false;
    Core::oflip(e);
    return true;
}

void Complex::delaunify(bool constrained) {
    bool f;
    do {
        f = false;
        for(auto t : ts) {
            f |= flip_delaunay(&t->a, constrained);
            f |= flip_delaunay(&t->b, constrained);
            f |= flip_delaunay(&t->c, constrained);
        }
    } while(f);
}


bool Complex::redelaunay(Edge* e)
{
    if(jpadding(e) || padding(e)) return false;

    if(constrained(e)) return false;

    if(jt_inside_circumcircle(e)) {
        Core::oflip(e);
        redelaunay(e);
        redelaunay(e->prv);
        redelaunay(e->nxt->j->prv);
        redelaunay(e->nxt->j->nxt);
        return true;
    }

    return false;
}

void Complex::delaunay(Node& n)
{
    if(n.invalid()) return;
    for(Node::iterator j = n.begin(); j != n.end(); ++j) {
        if(redelaunay(*j)) j = n.begin();
    }
}

