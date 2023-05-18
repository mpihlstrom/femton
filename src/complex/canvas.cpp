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
    //purge_small_trigons();
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

void populate_neighbors(Tri* t) {
    for(auto e : *t) {
        auto neighbor_cc = e->j->t->cc;
        if(neighbor_cc == nullptr)
            continue;
        //if(t->cc->neighbors.find(neighbor_cc) == t->cc->neighbors.end()) {
        t->cc->neighbors.insert(neighbor_cc);
        //}
    }
}

struct Link
{
    Edge* e;
    Link* nxt;
    Link* prv;
};

bool contour_recursive(Concomp* cc, Node* n, Edge* start, Col rc) {
    if(cc->cnt.size() > 250) {
        //return false;
    }
    if(cc->ts.size()*3 < cc->cnt.size()) {
        myDebug() << "bad size";
        return false;
    }
    for(auto e : *n) {
        if(e->t->cc == cc && e->j->t->cc != cc) {
            if(e == start) {
                //e->t->color = Col::Red;
                return true;
            }
            cc->cnt.push_back(e);
            //e->t->color = rc;
            return contour_recursive(cc, e->nxt->n, start, rc*1.01);
        }
    }
    return false;
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
        //auto rc = Col::random();
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
    }
    for(auto cc : com->ccs) {

        Edge* start = nullptr;
        for(auto t : cc->ts) {
            for(auto e : *t) {
                if(e->t->cc == cc && e->j->t->cc != cc) {
                    start = e;
                    goto end;
                }
            }
        }
        end:
        if(start != nullptr) {
            auto rc = start->t->color*0.1;
            cc->cnt.push_back(start);
            auto ret = contour_recursive(cc, start->nxt->n, start, rc);
            if(!ret) {
                cc->cnt.clear();
                myDebug() << "bad contour";
            } else {
                //myDebug() << "good contour";
            }
        } else {
            myDebug() << "no start";
        }



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



        int a = 4;




        /*
        if(!(e->t->cc == cc && e->j->t->cc != cc))
            continue;
        auto end = Node::iterator(e);
        Edge* g = nullptr;
        for(auto f = Node::iterator(e); f != end; ++f) {
            if(((*f)->t->cc != cc && (*f)->j->t->cc == cc)) {
               g = *f;
               break;
            }
        }
        if(g == nullptr)
            continue;
        auto ww = abs(e->v().unit0() ^ g->v().unit0());
        v += g->v() * w;
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


    if(true) {
        for(auto cc : com->ccs) {


            int cnt_sz = cc->cnt.size();
            for(int i = 0; i < cnt_sz; ++i) {
                Vec2 v;
                Edge* e = cc->cntr(i);
                Vec2 v1 = e->v();
                Vec2 v2 = cc->cntr(i-1)->v() * -1;

                int ngh = cnt_sz*0.5;
                Vec2 vn;
                auto wns = 0.0;
                for(int k = 0; k < ngh; ++k) {
                    auto vn1 = cc->cntr(i+k)->v();
                    auto vn2 = cc->cntr(i-1-k)->v() * -1;

                    auto wn1 = pow(abs((v1) ^ (vn1)), 2);
                    auto wn2 = pow(abs((v2) ^ (vn2)), 2);

                    vn += vn1 * wn1;
                    vn += vn2 * wn2;
                    wns += wn1 + wn2;
                }
                if(wns <= 0)
                    continue;
                vn /= wns;

                Vec2 v0;
                auto w1 = 1.0;//pow(abs((v1) ^ (v3)), 1);
                auto w2 = 1.0;//pow(abs((v2) ^ (v3)), 1);
                v0 += v1 * w1;
                v0 += v2 * w2;
                auto w0s = w1+w2;
                if(w0s <= 0)
                    continue;
                v0 /= w0s;

                v = v0 - vn*0.5;
                v *= 0.5;

                move(*e->n, e->n->cp + v);

            }


            /*
            for(auto t : cc->ts) {
                for(auto n : *t) {
                    Vec2 v;
                    double ws = 0.0;
                    int count = 0;
                    for(auto e : *n->n) {
                        if(!e->line())
                            continue;

                        if(!((e->t->cc == cc && e->j->t->cc != cc) || (e->t->cc != cc && e->j->t->cc == cc)))
                            continue;

                        auto w = 0.0;
                        auto v2 = e->n->p() - cc->mid;

                        w = pow(abs((v2) ^ (e->v())), 2);
                        v += e->v() * w;
                        v += v2.unit0() * com->ev_quant * w * 5;
                        ws += w;
                        ++count;
                    }

                    if(ws <= 0)
                        continue;
                    if(v.dot() <= 0)
                        continue;

                    v = v / ws * 0.2;
                    //v = v.unit() * com->ev_quant * 0.5;
                    move(*n->n, n->n->cp + v);
                }
            }*/
        }
        move_nodes();
        delaunify();
    }

/*
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
    */


    /*
    for(auto cc : com->ccs) {
        double neigh_area = 0.0;
        int count = 0;
        Vec2 midv = 0;
        double midvl = 0.0;
        for(auto neigh : cc->neighbors) {
            neigh_area += neigh->area;
            midv += neigh->mid - cc->mid;
            midvl += (neigh->mid - cc->mid).l2();
            ++count;
        }
        midv /= midvl;
        midv.norm();
        neigh_area += cc->area;

        for(auto t : cc->ts) {
            for(auto e : *t) {
                if(e->line())
                {
                    double jccar = e->j->t->cc ? e->j->t->cc->area : 0.0;
                    Vec2 v = (e->v().unit() + midvl).unit() * com->ev_quant * 0.5;// + midvl;
                    //else v -= (m - n->cp);
                    //v += e->v()*e->t->cc->area;
                    move(*e->n, e->n->cp + v);
                }
            }
        }
        move_nodes();
    }
    */


    if(false) {
        for(auto n : ns) {
            if(n->type != Node::Floating) continue;
            Vec2 v;
            int count = 0;
            double wsum = 0.0;

            count = 0;
            for(auto e : *n) {
                if(e->line())
                {
                    auto vv1 = e->v();
                    //auto vv2 = e->j->t->cc->mid - e->t->cc->mid;
                    auto w = pow(abs(e->j->t->cc->r * e->t->cc->r),2.0);//abs(vv1.unit().dot(vv2.unit()));
                    if(w <= 0.0) continue;// || w > 1.0) continue;
                    vv1 *= w;
                    //vv *= abs(e->t->cc->r);

                    //if(e->t->cc == com->ccs[5])
                    {
                        v += vv1*w;// + (e->t->cc->mid - n->p()) * -w;
                        wsum += w;
                        ++count;
                    }
                }
            }

            if(count <= 0) continue;

            if(v.dot() <= 0) continue;

            v.norm(); v *= com->ev_quant * 0.6;
            //v = v / wsum * 0.5;

            //move(*n, n->cp + n->v);
            move(*n, n->cp + v);
        }

        move_nodes();
        delaunify();
    }











/*

  //gravity

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
            auto mag = v.dot() * 0.0005;
            //double a = 20*com->ev_quant / pow(v.dot(),2);
            double acceleration = 10*mag*exp(-e->w*10);//(mag > com->ev_quant)? a : 0;
            if(!e->line()) {
                //acceleration *= 0.01;//acc *= 0.85;
                continue;
            }

            acc += v.unit() * acceleration;
            ++count;
        }
        if(!count) continue;//n->v -= acc / count;
        n->v += acc / count;

    }

    for(auto n : ns) move(*n, n->cp + n->v);
*/



    //move_nodes();

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
    if(epadding(e) || jpadding(e) || (constrained && this->constrained(e)) || !jt_inside_circumcircle(e))
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
    if(jpadding(e) || epadding(e)) return false;

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

