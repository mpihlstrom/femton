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




void Complex::refract(int iter)
{
    for(auto i = 0; i < iter; ++i) {
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

//pre-condition: e->j is in cc
void recursive_fill(Concomp* cc, Edge* e) {
    if(e->t->cc != nullptr)  { return; }
    if(e->line()) return;
    if(e->t->type != Tri::Regular) return;
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


void create_ccs() {
    for(auto c : com->cntrs)
        delete c;
    com->cntrs.clear();
    for(auto cc : com->ccs)
        delete cc;
    com->ccs.clear();
    for(auto t : com->trigons) {
        for(auto e : *t)
            e->cntr = nullptr;
        t->cc = nullptr;
    }
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

    for(auto t : com->ts) {
        for(auto e : *t) {
            if(e->cntr == nullptr && e->t->cc != e->j->t->cc) {
                auto cntr = new Contour;
                com->cntrs.push_back(cntr);
                cntr->add(e);
                auto ret = contour_recursive(cntr, e, e);
                if(!ret) {
                    myDebug() << "bad contour";
                }
            }
        }
    }
}

Edge* idx(std::vector<Edge*> *cnt, int i) {
    return (*cnt)[(i + cnt->size() - 1) % cnt->size()];
}


bool Complex::automata()
{
    purge_nonlines_recursive();
    color_to_line();
    refract();
    color_to_line();
    create_ccs();

    for(auto cntr_p : com->cntrs) {
        Contour &cntr = *cntr_p;
        int cnt_sz = cntr.count();
        for(int i = 0; i < cnt_sz; ++i) {
            Edge* e = cntr[i];
            Vec2 v1 = e->v();
            Vec2 v2 = cntr[i-1]->v() * -1;

            //auto pd = cc->cntr(i+1)->n->p() - cc->cntr(i-1)->n->p();

            if(e->t->type != Tri::Regular)
                continue;

            double we = 1.0;
            double wj = 1.0;
            if(e->j->t->cc != nullptr) {
                we = e->j->t->cc->area;
                wj = (double)e->j->t->cc->area;
            } else continue;

            Vec2 v0;
            auto w1 = 1.0;//pow(abs((v1.unit0()) ^ (v3.unit0())), 1);
            auto w2 = 1.0;//pow(abs((v2.unit0()) ^ (v3.unit0())), 1);
            v0 += v1 * w1;
            v0 += v2 * w2;
            auto w0s = w1+w2;
            if(w0s <= 0)
                continue;
            v0 /= w0s;

            double C = 8.0;

            //int ngh = fmin(cnt_sz/2-1,fmax(2, (1.0-exp(-ew/jw*C))*cnt_sz));
            //int ngh = fmin(ew/jw,1.0)*(cnt_sz/2-1);
            int ngh = 3;//fmin(fmax(3, 1500.0/cnt_sz), cnt_sz*0.5-1);
            //int ngh = (cnt_sz/2-1)*(1.0-exp(-jw/ew*50));
            Vec2 vn;
            auto vnws = 0.0;
            Vec2 pn;
            auto pws = 0.0;
            auto dws = 0.0;

            int ngh2 = 100;

            Vec2 mid;
            auto midws = 0.0;
            for(int k = 0; k < fmin(cntr.sz(), ngh2); ++k) {//cntr.sz()/4+1; ++k) {
                auto w = 1.0;// / ((cntr(i+k)->n->p() - e->n->p()).dot() + 1);
                mid += cntr[i+k]->n->p() * w;
                mid += cntr[i-k]->n->p() * w;
                midws += w*2;
            }
            mid /= midws;

            Vec2 midj1;
            auto midj1ws = 0.0;
            auto j1 = cntr[i]->j->relcntr(1);
            if(j1->cntr == nullptr) {
                j1->t->color = Col::random();
                continue;
            }
            for(int k = 0; k < fmin(j1->cntr->sz(), ngh2); ++k) {//j1->contour->sz()/4+1; ++k) {
                auto w = 1.0;
                midj1 += j1->relcntr(k)->n->p() * w;
                midj1ws += w;
            }
            midj1 /= midj1ws;


            Vec2 midj2;
            auto midj2ws = 0.0;
            auto j2 = cntr[i-1]->j;
            if(j2->cntr == nullptr) {
                j2->t->color = Col::random();
                continue;
            }
            for(int k = 0; k < fmin(j2->cntr->sz(), ngh2); ++k) {//j2->contour->sz()/4+1; ++k) {
                auto w = 1.0;
                midj2 += j2->relcntr(-k)->n->p() * w;
                midj2ws += w;
            }
            midj2 /= midj2ws;

            for(int k = 0; k < ngh; ++k) {
                if(cntr[i+k+0]->j->t->cc == nullptr) {
                    cntr[i+k+0]->j->t->color = Col::random();
                    continue;
                }
                if(cntr[i-k-1]->j->t->cc == nullptr) {
                    cntr[i-k-1]->j->t->color = Col::random();
                    continue;
                }
                //double wj1 = cc->cntr(i+k)->j->t->cc->area;
                //double wj2 = cc->cntr(i-k-1)->j->t->cc->area;
                //double abc1 = fmax(0.0,fmin(we/wj1,1.0));//(1.0-exp(-jfw/ew*C));
                //double abc2 = fmax(0.0,fmin(we/wj2,1.0));//(1.0-exp(-jbw/ew*C));

                //auto vn1 = cc->cntr(i+k)->v();
                //auto vn2 = cc->cntr(i-k-1)->v() * -1;
                auto pn1 = cntr[i+k+1]->n->p();
                auto pn2 = cntr[i-k-1]->n->p();
                auto p_pn1 = pn1 - e->n->p();
                auto p_pn2 = pn2 - e->n->p();
                auto mj_pn1 = pn1 - midj1;//cntr[i+0]->j->t->cc->mid;//cntr[i+k+0]->j->t->cc->mid;//
                auto mj_pn2 = pn2 - midj2;//cntr[i-1]->j->t->cc->mid;//cntr[i-k-1]->j->t->cc->mid;//
                auto m_pn1 = pn1 - mid;//e->t->cc->mid;//mid;//;
                auto m_pn2 = pn2 - mid;//e->t->cc->mid;//mid;//;
                //auto m_mn1 = cc->cntr(i+k)->j->t->cc->mid - cc->mid;
                //auto m_mn2 = cc->cntr(i-k-1)->j->t->cc->mid - cc->mid;

                //auto dd1 = (1.0 + (cc->cntr(i+k+0)->v().unit0() ^ cc->cntr(i+k+1)->v().unit0())) / 2.0;
                //auto dd2 = (1.0 + (cc->cntr(i-k-2)->v().unit0() ^ cc->cntr(i-k-1)->v().unit0())) / 2.0;
                //dws += dd1 + dd2;

                //auto pmn1 = cc->cntr(i+k)->j->t->cc->mid - e->n->p();
                //auto pmn2 = cc->cntr(i-k-1)->j->t->cc->mid - e->n->p();
                //auto mm1 = cc->mid - cc->cntr(i+k)->j->t->cc->mid;
                //auto mm2 = cc->mid - cc->cntr(i-k-1)->j->t->cc->mid;
                //auto bb1 = 1 + (vn1.unit0() ^ mm1.unit0());
                //auto bb2 = 1 + ((vn2.unit0()*-1) ^ mm2.unit0());
                //auto cc1 = 1 + (m_pn1.unit0() ^ vn1.unit0());
                //auto cc2 = 1 + (m_pn2.unit0() ^ (vn2*-1).unit0());

                //auto ee1 = fmax(0, 1 + (pd.unit0() ^ p_pn1.unit0()));
                //auto ee2 = fmax(0, 1 + (pd.unit0() ^ p_pn2.unit0()));

                auto pw1 = (1.0 / (p_pn1.dot()+1.0)) * (mj_pn1.dot()) * (m_pn1.dot());
                auto pw2 = (1.0 / (p_pn2.dot()+1.0)) * (mj_pn2.dot()) * (m_pn2.dot());
                pn += pn1*pw1 + pn2*pw2;
                pws += pw1+pw2;

                //auto vnw1 = abs(mm1.unit0() ^ cc->cntr(i+k)->v().unit0())   / (ppn1.dot() + 1.0);
                //auto vnw2 = abs(mm2.unit0() ^ cc->cntr(i-k-1)->v().unit0()) / (ppn2.dot() + 1.0);
                auto vnw1 = pw1;
                auto vnw2 = pw2;
                vn += p_pn1 * vnw1;
                vn += p_pn2 * vnw2;
                vnws += vnw1 + vnw2;
            }

            if(pws <= 0) {
                myDebug() << "(pws <= 0)";
                continue;
            }
            pn /= pws;

            /*
            if(vnws <= 0)
               continue;
            vn /= vnws;
            */

            auto v3 = (pn - e->n->p()).unit0() * com->ev_quant * 2.0;//vn.l2();//
            //auto abc = fabs(vn.unit0() & (e->t->cc->mid - e->j->t->cc->mid).unit0());
            //auto abc = fabs(v1.unit0() & v2.unit0());//fabs(vn.unit0() & v3.unit0());
            Vec2 v = v0 - v3;//*fmax(fmin(we/wj,1),0.5);//v0 = v0.unit0() * com->ev_quant * 0.5;
            //Vec2 v = v0 - vn;//fmax(fmin(pow(wj/we,2), 1.0),0.1);//(1.0-exp(-jw/ew*C));//
            v *= 0.75;

            move(*e->n, e->n->cp + v);

        }
    }
    move_nodes();
    color_to_line();
    delaunify();
    color_to_line();
    purge_nonlines();
    //purge_straight_lines();
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

