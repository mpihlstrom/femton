#include "globals.h"
#include "complex/complex.h"
#include "contour.h"

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
            if(e->line() && e->j->t->type != Tri::Padding && e->t->type != Tri::Padding)
                goto no_merge;
        merge((n->type == Node::Border)? n->border() : n->entry());
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

void Complex::refract(int iters)
{
    for(auto i = 0; i < iters; ++i)
        for(auto t : ts)
            for(auto e : *t)
                if(e->line())
                    if(e->v().l2() > ev_quant*10)
                        split_edge(e, 0.5);

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



Edge* idx(std::vector<Edge*> *cnt, int i) {
    return (*cnt)[(i + cnt->size() - 1) % cnt->size()];
}


int automata_counter = 0;

bool Complex::automata()
{
    ++automata_counter;

    purge_nonlines_recursive();
    color_to_line();
    refract();
    color_to_line();
    create_contours();

    /*
    for(auto cntr_p : com->cntrs) {
        Contour &cntr = *cntr_p;
        int cnt_sz = cntr.count();

        double curv_sum = 0.0;
        for(int i = 0; i < cnt_sz; ++i) {
            double curv = (1.0 + (cntr[i]->v().unit0() & cntr[i+1]->v().unit0())) * 0.5;
            curv_sum += curv;
        }
        curv_sum /= cnt_sz;

        for(int i = 0; i < cnt_sz; ++i) {
            auto v = (cntr[i]->v() + cntr[i-1]->v()*-1)*0.5*(pow(curv_sum,0.5));
            move(*cntr[i]->n, cntr[i]->n->cp + v);
        }
    }
    */


    for(auto cntr_p : com->cntrs) {
        Contour &cntr = *cntr_p;
        int cnt_sz = cntr.count();

        for(int i = 0; i < cnt_sz; ++i) {
            Edge* e = cntr[i];
            Edge* j = e->j;
            if(e->j->t->cc == nullptr)
                continue;
            auto j1 = cntr[i]->j->relcntr(1);
            if(j1->cntr == nullptr)
                continue;
            auto j2 = cntr[i-1]->j;
            if(j2->cntr == nullptr)
                continue;

            Vec2 v1 = e->v();
            Vec2 v2 = cntr[i-1]->v() * -1;
            Vec2 v0 = (v1 + v2)/2.0;

            int ngh = 4;
            //int ngh2 = fmax(cntr.sz()*0.35, ngh*4);
            int ngh2 = 40;
            //myDebug() << ngh2;

            Vec2 mid;
            auto midws = 0.0;
            for(int k = 0; k < fmin(cnt_sz, ngh2); ++k) {//cntr.sz()/4+1; ++k) {
                auto pn1 = cntr[i+k]->n->p();
                auto pn2 = cntr[i-k]->n->p();
                auto w1 = 1;// - (vn1a.unit0() ^ vn1b.unit0());//1.0;///(l1ws);//1.0/(p_pn1.dot()+1);//
                auto w2 = 1;// - (vn2a.unit0() ^ vn2b.unit0());//1.0;///(l2ws);//1.0/(p_pn2.dot()+1);//
                mid += pn1*w1 + pn2*w2;
                midws += w1+w2;
            }
            mid /= midws;

            Vec2 midj1;
            auto midj1ws = 0.0;
            for(int k = 0; k < fmin(j1->cntr->sz(), ngh2); ++k) {//j1->contour->sz()/4+1; ++k) {
                midj1 += j1->relcntr(k)->n->p();
                midj1ws += 1;
            }
            midj1 /= midj1ws;

            Vec2 midj2;
            auto midj2ws = 0.0;
            for(int k = 0; k < fmin(j2->cntr->sz(), ngh2); ++k) {//j2->contour->sz()/4+1; ++k) {
                midj2 += j2->relcntr(-k)->n->p();
                midj2ws += 1;
            }
            midj2 /= midj2ws;

            Vec2 pn;
            auto pws = 0.0;
            for(int k = 0; k < ngh; ++k) {
                auto pn1 = cntr[i+k]->n->p();
                auto pn2 = cntr[i-k]->n->p();

                auto p_pn1 = pn1 - e->n->p();
                auto p_pn2 = pn2 - e->n->p();
                auto m_pn1_a = pn1 - mid;//e->t->cc->mid;//
                auto m_pn2_a = pn2 - mid;//e->t->cc->mid;//
                if(cntr[i+k+0]->j->t->cc == nullptr || cntr[i-k-1]->j->t->cc == nullptr) continue;

                auto pw1 = (m_pn1_a.dot()/(p_pn1.dot()+1));
                auto pw2 = (m_pn2_a.dot()/(p_pn2.dot()+1));
                pn += pn1*pw1 + pn2*pw2;
                pws += pw1+pw2;
            }

            if(pws <= 0) {
                myDebug() << "pws <= 0";
                continue;
            }
            pn /= pws;


            auto p_emid = e->t->cc->mid - e->n->p();
            auto p_jmid = j->t->cc->mid - e->n->p();
            auto emid = e->t->cc->mid;
            auto jmid = j->t->cc->mid;

            auto midw = fmin(p_jmid.dot() / (p_jmid.dot()+p_emid.dot()), 1);
            auto mmid = emid*midw + jmid*(1-midw);
            //mmid = p_emid.dot() < p_jmid.dot()? e->t->cc->mid : j->t->cc->mid;

            auto vv = 1 - 2*exp(-(mmid - pn).dot()*0.00005);

            auto v3 = (pn - e->n->p()).unit0() * com->ev_quant * 2.2;
            Vec2 v = v0 - v3;//*vv;
            v *= 0.6;

            //v = p_mid*0.16*(1.0-w) + v;

            move(*e->n, e->n->cp + v);

        }
    }


    /*
    for(auto cntr_p : com->cntrs) {
        Contour &cntr = *cntr_p;

        if(cntr[0]->j->t->cc == nullptr) continue;
        if(cntr[0]->t->cc->area > cntr[0]->j->t->cc->area) continue;

        auto vl2s = 0.0;
        Vec2 vs;
        for(int i = 0; i < cntr.count(); ++i) {
            auto v = (cntr[i]->n->np - cntr[i]->n->cp);
            vs += v;
            vl2s += v.l2();
        }
        vs /= cntr.count();
        vl2s /= cntr.count();
        //auto ww = fmin(1.0, vs.l2() / (com->ev_quant));
        //auto ww = fmin(1.0, vs.l2() / (com->ev_quant));
        if(vl2s > 0.0) {
            continue;
        }
        myDebug() << "non-moving! " << automata_counter << cntr[0];
        //cntr[0]->t->color = Col::random();

        auto q = fmin(1, fmax(0, 1.0 - (cntr.sur*cntr.sur / (4*M_PI)) / cntr[0]->t->cc->area));

        Vec2 vv;
        auto vvws = 0.0;
        for(int i = 0; i < cntr.count(); ++i) {
            auto e = cntr[i];
            vv += e->j->t->cc->mid - cntr[i]->n->p();
            vvws += vv.dot();
        }
        //if(vvws <= 0) continue;
        //vv /= cntr.count();
        vvws /= cntr.count();

        for(int i = 0; i < cntr.count(); ++i) {
            auto e = cntr[i];
            auto v = e->j->t->cc->mid - cntr[i]->n->p();
            auto mm = e->j->t->cc->mid - e->t->cc->mid;
            auto ww = fabs(v.unit0() & mm.unit0());
            if(v.dot() < mm.dot()) {
                move(*e->n, e->n->cp + v.unit0()*ww*-1*com->ev_quant*2); //(e->v().rot90().unit0())*com->ev_quant*0.5 +
            }
        }
    }*/


    move_nodes();
    color_to_line();

    /*for(auto cntr_p : com->cntrs) {
        Contour &cntr = *cntr_p;
        for(int i = 0; i < cntr.count(); ++i) {
            Edge* e = cntr[i];
            if(e->t->cc->area < com->ar_quant && e->v().l2() < com->ev_quant) {
                merge(e);
                break;
            }
        }
    }
    move_nodes();
    */
    color_to_line();

    delaunify();
    color_to_line();

    //purge_straight_lines();
    //purge_small_trigons();
    color_to_line();


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
    */

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

    //color_to_line();
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

