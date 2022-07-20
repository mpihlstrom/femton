#include "globals.h"
#include "window.h"
#include "globals.h"

#include <math.h>

double fmod_360(double angle) {
    while(angle > 360.) angle -= 360.;
    while(angle < 0.) angle += 360.;
    return angle;
}

void Brush::left_down(Vec2 rel) {
    if(actions & Drop) split_radius();
    if(actions & Split_line) split_line(rel);

    draw->update();//treat_canvas();

    //if(action_at_left_down)
    //    action();
}

void Brush::timer_update() {
    if(actions & Timer) action();
}

void Brush::action(Vec2 const* scr_v)
{
    if(!left && !right)
        return;
    //different order here may have big consequences
    if(actions & Blend)
        blend();
    //com->color_to_line();
    if(actions & Contrast) change_contrast();
    if(actions & Blur) change_blur();

    if(actions & Push) push();
    if(actions & Smooth) smooth();

    perturb = right;
    if(scr_v != nullptr && actions & Rub) rub(*scr_v);

    //if(actions & Split_line) split_line();
    split_line(scr_v == nullptr? Vec2() : *scr_v);
    //split_radius();

    treat_canvas();
}

void Brush::motion(Vec2 v_scr)
{
    if(!left && !right)
        return;
/*
    auto steps = (int)ceil(view->scr_can_v(v_scr).l2() / com->ev_quant*100.0);
    Vec2 step = v_scr / steps;

    for(int i = 0; i < steps; ++i) {
        action(&step);
    }
    */
    action(&v_scr);
}

void Brush::treat_canvas()
{
    com->color_to_line();
    com->delaunify();
    com->purge();
    draw->update();
}

void Brush::wheel_rot(double delta) {
    radius *= pow(1.0005, delta);
    draw->update();
}

void Brush::blend()
{
    uint32_t* buffer;
    if(draw->renders["painting"]->active)
        buffer = ((Surface_render*)draw->renders["painting"])->fbo_buffer(Base_render::FBO_buffer::Color, false);
    else
        buffer = ((Surface_render*)draw->renders["gaussian_v"])->fbo_buffer(Base_render::FBO_buffer::Color, false);

    //uint32_t* sur_buffer = ((Surface_render*)draw->renders["surface"])->fbo_buffer(Base_render::FBO_buffer::Color, false);
    auto r = Rect(scr_sz);

    double r_can = view->uni_can_s(radius);
    auto m_can = view->scr_can_p(cursor);

    for(auto n : com->ns) {
        if((n->p() - m_can).l2() > r_can)
            continue;
        for(auto e : *n) {
            auto t = e->t;
            Vec2i p_scr = view->can_scr_p(*t * Vec3(1)/3.0);
            if(!r.contains(p_scr))
                continue;

            double q = 0.01;
            n->contrast = n->contrast*(1.0-q) + blend_dist_th*q;
            n->w_ = 1.0 - n->contrast;

            Col col(*(buffer + (r.h-1-p_scr.y)*r.w + p_scr.x));

            if(col.dist(t->color) >= blend_dist_th)
            {
                float a = t->color.a;
                t->color = col;
                t->color.a = a;
            }
        }
    }

    return;
}

void Brush::rub(Vec2 scr_v)
{
    auto mp_can = view->scr_can_p(cursor);
    auto r_can = view->uni_can_s(radius);

    double pert_intensity = 0.15;

    for(auto n : com->ns) {
        if(n->type_() != Node::Floating) continue;
        auto diff = (n->p() - mp_can).length();
        double r = diff / r_can;
        if(r < 1.0) {
            auto v = view->scr_can_v(scr_v);//*pow((1.0-r),0.25);
            if(perturb) {
                //double ang = M_PI*(rand_uni()*2.0-1.0)*pert_intensity;
                double ang = M_PI*rand_pn1()*pert_intensity;
                double len = 1.0 + rand_pn1()*0.1;
                v = v.scalerotate(Vec2(cos(ang), sin(ang))*len);
            }
            auto newpos = (n->p() + v).round();
            double q = (newpos - mp_can).l2() / r_can;
            if(q > 1.0) {
                newpos = mp_can + (newpos - mp_can)/q;
            }

            com->move(*n, newpos);
        }
    }

    com->move_nodes();
}

void Brush::push()
{
    const double intensity = 0.1;
    auto m = view->scr_can_p(cursor);
    auto r = view->uni_can_s(radius);

    for(auto n : com->ns) {
        Vec2 v(n->p() - m);
        if(v.l2() < r) {
            double fringe_dist = r - v.l2();
            Vec2 dir;
            if(left) {
                dir = v.unit() * fringe_dist*intensity;
                com->move(*n, n->p() + dir);
            } else {
                dir = v.scalerotate(Vec2(1,0.1).unit());// * intensity * 0.3;
                com->move(*n, m + dir.round());
            }
        }
    }

    com->move_nodes();
}


void Brush::split_line(Vec2 scr_v)
{
    const double split_radius = radius;
    const double min_split_length = com->ev_quant * min_split_len_factor;
    const double radius_canvas = (view->uni_can_s(split_radius) + view->scr_can_v(scr_v).l2()*2.1);

    Vec2 mp = view->scr_can_p(cursor);
    double r = radius_canvas;
    Node* new_n = nullptr;

    auto candidates = com->trigons;
    for(auto t : Eutris(&candidates)) {
        for(auto e : *t) {
            if(!e->line())
                continue;

            //if(e->n->type_() != Node::Floating || e->nxt->n->type_() != Node::Floating) continue;

            if(com->flip_delaunay(e)) { //some delaunay aid for better splits
                candidates.push_back(e->t);
                candidates.push_back(e->j->t);
            }

            Vec2 em = mp - e->n->p();
            double eml = em.length();
            Vec2 ev = e->v();
            double evl = ev.length();

            //start, split edge case
            double b = fabs(ev ^ em / evl); //sinus = b
            if(b < r) { //is the line e->n to e->nxt->n intersecting the circle?
                double a2 = r*r - b*b; //cosinus = a; r = hypotenuse
                double d = ev & em / evl;
                double f = (eml < r)? d + sqrt(a2) : d - sqrt(a2); //e->n inside or not

                double orto = 1 - fabs(d / eml); //todo maybe: orto = b
                if(f > orto*min_split_length && evl-f > orto*min_split_length) {
                    double w = 1 - f / evl;
                    if(0 < w && w < 1) { //the posisition of e->nxt->n (on the line) has not benn considered until now
                        if((new_n = com->split_edge(e, w)) != nullptr)
                            for(auto i : *new_n)
                                candidates.push_back(i->t);
                    }
                }
            } //split edge case, end

            //split trigon case
            if (eml > r || (mp - e->nxt->n->p()).l2() >  r || evl < min_split_length*2)
                continue;
            if((new_n = com->split_edge(e, 0.5)) != nullptr)
                for(auto i : *new_n)
                    candidates.push_back(i->t);
        }
    }
}

void Brush::split_radius()
{
    double r = view->uni_can_s(radius);
    int g = (2*M_PI*r) / (com->ev_quant*10);

    Vec2 mp = view->scr_can_p(cursor);
    std::vector<Vec2> ps;


    for(int i = 0; i < g; ++i) {
        ps.push_back((mp + Vec2(cos(((double)i*2.0*M_PI)/(double)g), sin((double)i*2.0*M_PI/(double)g))*r).round());
    }


    /*
    g = 20;
    for(int i = 0; i < g; ++i) {
        auto s = (mp + Vec2(cos(((double)i*2.0*M_PI)/(double)g), sin((double)i*2.0*M_PI/(double)g))*r).round();
        auto alpha = rand_pn1()*M_PI;
        auto rad = rand_uni()*r*0.5;
        Vec2 p = Vec2(cos(alpha), sin(alpha)) * rad;
        ps.push_back(s + p);
    }*/

    Tri *t;
    int start = 0;
    for(; (t = com->inside(ps[start])) == nullptr && start < g; ++start);

    Node* n = com->split(*t, t->p2u(ps[start]));

    for(int i = start; i < start+g && n != nullptr; ++i) {
        n = com->line_split_node(n, ps[i % g]);
    }

    for(auto t : com->ts) {
        auto p = t->centroid();
        int h = 0;
        for(int i = 0; i < ps.size(); ++i) {
            auto p0 = ps[i];
            auto p1 = ps[(i+1) % ps.size()];
            auto ysgn = sgn(p1.y - p0.y);
            if(ysgn*(p.y - p0.y) >= 0 &&  ysgn*(p1.y - p.y) > 0) {
                h += sgn((p1 - p0) ^ (p - p0));
            }
        }
        if(h > 0) {
            t->color = brush_color;
        }
    }



    /*for(auto t : com->ts) {
        for(auto e : *t)
            if((mp - e->n->p()).l2() > r * 1.01)
                goto no_color;
        t->color = brush_color;
        no_color:;
    }*/

}




void Brush::change_contrast()
{
    const double can_radius = view->uni_can_s(radius);
    const double fid = 0.01;
    const double eps = 0.000000001;

    auto can_cur = view->scr_can_p(::cursor);
    std::list<Tri*> candidates;
    for(auto t : com->trigons)
        if(!t->invalid())
            candidates.push_back(t);

    for(auto t : com->ts) {
        for(auto e : *t) {
            if((e->n->p() - can_cur).length() > can_radius)
                continue;
            if(left)  e->n->contrast = e->n->contrast <= 1 ? e->n->contrast += fid : 1;
            if(right) e->n->contrast = e->n->contrast >= 0 ? e->n->contrast -= fid : 0;

            if(e->line()) {
                double al = 0.5;
                e->n->contrast = e->n->contrast*al + e->nxt->n->contrast*(1-al);
                //(e->nxt->n->p() - can_cur).length() > can_radius
            }

        }
    }
}

void Brush::change_blur()
{
    const double can_radius = view->uni_can_s(radius);
    const double fid = 0.01;
    const double eps = 0.000000001;

    auto p = view->scr_can_p(::cursor);
    std::list<Tri*> candidates;
    for(auto t : com->ts)
        candidates.push_back(t);

    for(auto t : com->ts) {
        for(auto e : *t) {
            if((e->n->p() - p).length() > can_radius)
                continue;
            if(left)  e->n->w_ = e->n->w_ <= 1 ? e->n->w_ += fid : 1;
            if(right) e->n->w_ = e->n->w_ >= 0 ? e->n->w_ -= fid : 0;

            if(e->line()) {
                double al = 0.5;
                e->n->w_ = e->n->w_*al + e->nxt->n->w_*(1-al);
            }
        }
    }
    draw->update();
}



void Brush::smooth() {
    double canvas_radius = view->uni_can_s(radius);

    for(auto n : com->ns) {
        Vec2 n_cur(view->scr_can_p(cursor) - Vec2(n->cp));
        if(n_cur.length() < canvas_radius) {

            Vec2 v;
            int count = 0;

            for(auto e : *n) {
                Vec2 n_nxt(view->scr_can_p(cursor) - Vec2(e->nxt->n->cp));
                if(n_nxt.length() > canvas_radius)
                    continue;
                if(!e->line())
                    continue;
                v +=  e->v();
                ++count;
            }
            if(count <= 0)
                continue;
            v /= (double)count;
            v.norm();
            v *= com->ev_quant;

            com->move(*n, (Vec2(n->cp)+v).round());
        }
    }

    com->move_nodes();
}

void Colorpicker::left_down(Vec2 p_scr) {
    o_color = brush_color;
    pick(p_scr);
}

void Colorpicker::right_down(Vec2) {
    if(left) {
        brush_color = o_color;
        window->set_colorpicker_button_color();
    }
}

void Colorpicker::motion(Vec2) {
    if(!left || right)
        return;
    pick(cursor);
}

void Colorpicker::pick(Vec2i p_scr) {
    auto r = Rect(scr_sz);
    if(!r.contains(p_scr.x))
        return;
    uint32_t* buffer = ((Painting_render*)draw->renders["painting"])->fbo_buffer(Base_render::FBO_buffer::Color, false);
    uint32_t color = *(buffer + (r.h-1-p_scr.y)*r.w + p_scr.x);
    brush_color = Col(color);
    window->set_colorpicker_button_color();
    draw->update();
    return;
}

void Selecttool::wheel_rot(double) {
    if(!selected)
        return;
    if(selected->name() == Class::Class_node) {
    }
}

int e_count(Node* n);

void Selecttool::left_down(Vec2 p) {
    uint32_t id = draw->composition_id_at(p.x, p.y);
    myDebug() << "class id" << id;
    if(selectable.find(id) == selectable.end()) {
        selected = 0;
        return;
    }
    selected = (Class*)selectable[id];
    Class::Name class_name = selected->name();

    switch(class_name) {
    case Class::Class_trigon: {
            Tri& t = *(Tri*)selected;
            Vec2 coord = draw->surface_coordinate_at(p.x, p.y);
            Node* n = com->split(t, coord);
            selected = n; //may be null
            break;
        }
    case Class::Class_node:{
        Node* n = (Node*)selected;
        myDebug() << "e count" << e_count(n);
        break;
    }
    case Class::Class_void: break;
    default: return;
    }

    draw->update();
}

void Selecttool::right_down(Vec2 p) {
    uint32_t id = draw->composition_id_at(p.x, p.y);
    if(selectable.find(id) == selectable.end()) {
        selected = 0;
        return;
    }

    selected = (Class*)selectable[id];
    Class::Name class_name = selected->name();
    switch(class_name) {
    case Class::Class_trigon: {
            Tri& t = *(Tri*)selected;
            Vec2 coord = draw->surface_coordinate_at(p.x, p.y);
            Node* n = com->split_edge(t, coord);
            selected = n; //may be null
            break;
        }
    case Class::Class_node:{
        Node* n = (Node*)selected;
        myDebug() << "e count" << e_count(n);
        break;
    }
    case Class::Class_void: break;
    default: return;
    }

    draw->update();
}

void Selecttool::motion(Vec2 rel) {
    if(!selected) {
        cameratool.motion(rel);
        return;
    }
    if(left) {
        Class::Name sname = selected->name();
        switch(sname) {
        case Class::Class_node: {
                Node& n = *(Node*)selected;
                Vec2 mv = view->scr_can_v(rel);
                com->move(n, n.cp + mv);
                com->move_nodes();
                //n.cp += mv;
                break;
            }
        case Class::Class_trigon:
        case Class::Class_void:
            cameratool.motion(rel);
            break;
        default:
            return;
        }
    }
    else if(right) {
        Class::Name sname = selected->name();
        switch(sname) {
        case Class::Class_node: {
                Node& n = *(Node*)selected;
                Vec2 mv = view->scr_can_v(rel);
                com->move(n, n.cp + mv);
                //n.pp += mv;
                com->clip(com->_canvas, n.cp);
                break;
            }
        case Class::Class_trigon:
        case Class::Class_void:
            cameratool.motion(rel);
            break;
        default:
            return;
        }
    }
    draw->update();
}


std::list<Tri*> sample_trigons(Vec2 origin0, double r_inner0, double r_outer0, int num_samples) {
    std::list<Tri*> candidates;

    uint32_t* buffer = ((Surface_render*)draw->renders["surface"])->fbo_buffer(Base_render::FBO_buffer::Id, true);

    int w = scr_sz.w;
    int h = scr_sz.h;
    Vec2 origin = view->uni_scr(origin0);
    Vec2i p = Vec2i(origin.x, h - origin.y);

    double r_inner = r_inner0 * (w+h)*0.5 * 0.5;
    double r_outer = r_outer0 * (w+h)*0.5 * 0.5;

    int samples = num_samples;
    double alpha = 2*M_PI / (double)samples;

    for(int k = 0; k < samples; ++k)
    {
        double kt = (double)k / (double)samples;
        double r_inter = r_inner*(1-kt) + r_outer*kt;
        Vec2i r = Vec2(cos(alpha*k), sin(alpha*k)) * r_inter;
        Vec2i s(p.x + r.x, p.y + r.y);
        if(0 > s.x || s.x >= w || 0 > s.y || s.y >= h)
            continue;

        uint32_t id = *(buffer + s.y*w + s.x);

        Class* cls = (Class*)selectable[id];
        if(cls->name() != Class::Name::Class_trigon)
            continue;
        Tri* t = (Tri*)cls;
        if(t->id != id || t->invalid())
            continue;
        candidates.push_back(t);
    }

    return candidates;
}

std::list<Tri*> scan_trigons(Vec2 origin_uni, double r_uni) {
    std::list<Tri*> candidates;

    int w = scr_sz.w;
    int h = scr_sz.h;
    Vec2 origin = view->uni_scr(origin_uni);
    Vec2i p = Vec2i(origin.x, h - origin.y);
    double r = r_uni * (w+h)*0.5 * 0.5;

    uint32_t* buffer = ((Surface_render*)draw->renders["surface"])->fbo_buffer(Base_render::FBO_buffer::Id, true);
    for(int x = (int)max(p.x-r, 0); x < (int)min(p.x+r, w); ++x) {
        for(int y = (int)max(p.y-r, 0); y < (int)min(p.y+r, h); ++y) {
            if((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) > r*r)
                continue;
            uint32_t id = *(buffer + y*w + x);
            Class* cls = (Class*)selectable[id];
            if(cls->name() != Class::Name::Class_trigon)
                continue;
            Tri* t = (Tri*)cls;
            if(t->id != id || t->invalid())
                continue;
            candidates.push_back(t);
        }
    }
    return candidates;
}
