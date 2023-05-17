#include "trigon.h"
#include "node.h"

#include "globals.h"

#include <QDebug>
#include <cassert>
#include <sstream>

//#pragma warning(disable: 4355) //passing 'this' in initializer list.

Tri::Cross::Root root_t0(1, 0, 0, nullptr, -1);
//Tri::Cross::Polyroot root_t1(Tri::Cross::Cross::Polyroot::T1);
Tri::Cross::Root root_t1(1, -2, 1, nullptr, -1);

Tri::Tri(Node* an, Edge* aj, Node* bn, Edge* bj, Node* cn, Edge* cj, Type t, Col const& col) : cross(nullptr),
    type(t), status(Live), a(an, aj, &b, &c, this), b(bn, bj, &c, &a, this), c(cn, cj, &a, &b, this), _area(1), color(col), entry(&a) {}

Tri::Tri(Node* an, Node* bn, Node* cn, Type t, Col const& col) : cross(nullptr),
    type(t), status(Live), a(an, 0, &b, &c, this), b(bn, 0, &c, &a, this), c(cn, 0, &a, &b, this), _area(1), color(col), entry(&a) {}

Tri::Label Tri::label(Node const& n) const { return label(n.entry()); }

Vec2 Tri::ab() const { return b.n->p_() - a.n->p_(); }
Vec2 Tri::bc() const { return c.n->p_() - b.n->p_(); }
Vec2 Tri::ca() const { return a.n->p_() - c.n->p_(); }

int64_t Tri::area() const { return (b.n->p_() - a.n->p_())^(c.n->p_() - a.n->p_()); }

Vec3 Tri::p2u(Vec2 const& a, Vec2 const& b, Vec2 const& c, Vec2 const& q) {
    double ar_pbc = (c - b)^(q - b),
           ar_apc = (a - c)^(q - c),
           ar_abp = (b - a)^(q - a),
           ar_abc = (b - a)^(c - a);
    return Vec3(ar_pbc,ar_apc,ar_abp)/ar_abc;
}

Root::Root(Root const& s) : A(s.A), B(s.B), C(s.C), kind(s.kind), sqrtsgn(s.sqrtsgn), /*colin(s.colin),*/ root1(s.root1), root2(s.root2) {
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);
}

Root::Root() : A(0), B(0), C(0), kind(None), /*colin(false),*/ root1(-INFINITY), root2(-INFINITY) {
    mpz_init(A_);
    mpz_init(B_);
    mpz_init(C_);
}

void Root::operator=(Root const& s) {
    A = s.A; B = s.B; C = s.C; kind = s.kind; sqrtsgn = s.sqrtsgn; /*colin = s.colin;*/ root1 = s.root1; root2 = s.root2;
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);
}


Tri::Cross::Root::Root(int64_t Ap, int64_t Bp, int64_t Cp, Tri::Cross::Root const* cur, int sqrtsgn) : A(Ap), B(Bp), C(Cp), sqrtsgn(sqrtsgn)
{
    init(cur);
}

Tri::Cross::Root::Root(Tri const& t, Tri::Cross::Root const* cur, int sqrtsgn) : sqrtsgn(sqrtsgn)
{
    auto a0 = t.a.n->pp, a1 = t.a.n->cp, b0 = t.b.n->pp, b1 = t.b.n->cp, c0 = t.c.n->pp, c1 = t.c.n->cp;
    auto ab0 = b0 - a0, ab1 = b1 - a1, ac0 = c0 - a0, ac1 = c1 - a1;
    auto ab0ac0 = ab0^ac0, ab1ac1 = ab1^ac1, ab0ac1 = ab0^ac1, ab1ac0 = ab1^ac0;
    A = ab0ac0 - ab0ac1 - ab1ac0 + ab1ac1;
    B = -2*ab0ac0 + ab0ac1 + ab1ac0;
    C = ab0ac0;

    init(cur);
}


void Root::init(Tri::Cross::Root const* cur)
{
    //colin = false;
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);

    if(A == 0) {
        if(B == 0) {
            if(C == 0) { //degenerate case, colinear trigon at t = 0 has been created due to previous flip at current time step.
                *this = *cur; //inherit the current non-degenerate case
                //colin = true;
                return;

            } else {
                kind = Imaginary;
            }
        } else {
            root1 = root2 = -(double)C/(double)B;
            kind = Linear;
        }
    } else {
        if(init_sq_sgn_lh1_big() >= 0) {
            root1 = -(double)B/((double)2*A) - sqrt((double)B*(double)B/((double)4*A*(double)A) - (double)C/(double)A);
            root2 = -(double)B/((double)2*A) + sqrt((double)B*(double)B/((double)4*A*(double)A) - (double)C/(double)A);
            kind = Square;
        } else {
            kind = Imaginary;
        }
    }
}

//operator<, biggest numbers on the top of the heap
//returns true if f should come after s in queue
bool Cross::lt(Tri::Cross* f, Tri::Cross* s)
{
    auto d = f->crossroot - s->crossroot;

    if(d == 0) {
        if(f->overlap && !s->overlap) return true;
        else if(!f->overlap && s->overlap) return false;
        if(f->overlap && s->overlap) {
            if(f->overlap != s->overlap)
                return f->overlap < s->overlap;
            if(f->iter != s->iter)
                return f->iter > s->iter;
        }

        return f->count > s->count;
    }

    return d > 0;
}

int Cross::Root::operator-(Root const& s) const
{
    Root const& f = *this;

    int idiff = 0;
    auto froot = f.sqrtsgn <= 0 ? f.root1 : f.root2;
    auto sroot = s.sqrtsgn <= 0 ? s.root1 : s.root2;
    auto fdiff = froot - sroot;

    //if(abs(fdiff) > 1e-13) return sgn(fdiff);

    switch(f.kind) {
        case Linear:
            switch(s.kind) {
                case Linear:
                    idiff = sgn(f.B)*sgn(s.B)*sgn(s.C*f.B - f.C*s.B); //sgn(f.B)*sgn(s.B)*linlin_sgn_lh1_big(f,s);
                    break;
                case Square: {
                    auto sgnL = sgn(s.A)*sgn(f.B)*sgn(f.B*s.B - 2*f.C*s.A); //sgn(s.A)*sgn(f.B)*linsq_sgn_lh1_big(f,s);
                    auto sgnR = s.sqrtsgn*sgn(s.B*s.B - 4*s.C*s.A); //s.sqrtsgn*linsq_sgn_rh1_big(s);
                    if(sgnL*sgnR <= 0)
                        idiff = sgnL - sgnR;
                    else
                        idiff = sgn(s.A)*sgnL*linsq_sgn_lh2_big(f, s);
                    break;
                }
            }
            break;
        case Square: {
            switch(s.kind) {
                case Linear:
                    idiff = -(s - f);
                    break;
                case Square: {
                    auto sgnL1 = sgn(f.A)*sgn(s.A)*sgn(s.B*f.A - f.B*s.A); //sgn(f.A)*sgn(s.A)*sqsq_sgn_lh1_big(f,s);
                    auto sgnR1 = sqsq_sgn_rh1_big(f,s);
                    if(sgnL1 * sgnR1 <= 0)
                        return sgnL1 - sgnR1;
                    auto sgnL2 = sgnL1*sgn(f.A)*sgn(s.A)*sgn(-f.B*s.B + 2*f.C*s.A + 2*s.C*f.A); //sgn_lh1*sgn(f.A)*sgn(s.A)*sqsq_sgn_lh2_big(f,s);
                    auto sgnR2 = sgnR1*-f.sqrtsgn*s.sqrtsgn*sqsq_sgn_rh2_big_2(f,s); //sgn_rh1*-f.sqrtsgn*s.sqrtsgn*sgn((s.B*s.B - 4*s.A*s.C)*(f.B*f.B - 4*f.A*f.C));
                    if(sgnL2 * sgnR2 <= 0)
                        idiff = sgnL2 - sgnR2;
                    else
                        idiff = sgnR2*sgn(sqsq_sgn_lh3_big(f, s));
                    break;
                }
            }
        }
    }



    if(sgn(fdiff) != sgn(idiff)) {
        //qDebug() << "not equal sgn";
        //qDebug() << fdiff;
        //qDebug() << idiff;
    }

    return idiff;
}


Cross::SqL2::SqL2(Edge* e, Root* t) : t(t)
{
    Vec2 a0 = e->n->pp, a1 = e->n->cp, b0 = e->nxt->n->pp, b1 = e->nxt->n->cp;
    Vec2 ab0 = b0 - a0, ab1 = b1 - a1;
    double ab0ab0 = ab0&ab0;
    double ab1ab1 = ab1&ab1;
    double ab0ab1 = ab0&ab1;
    A = ab0ab0 - 2*ab0ab1 + ab1ab1;
    B = -2*ab0ab0 + 2*ab0ab1;
    C = ab0ab0;
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);
}


Cross::SqL2::SqL2(int64_t A, int64_t B, int64_t C, Root* t) : A(A), B(B), C(C), t(t)
{
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);
}

Cross::SqL2 Cross::SqL2::operator-(SqL2 const& s) const {
    return Cross::SqL2(this->A - s.A, this->B - s.B, this->C - s.C, this->t);
}

void Cross::SqL2::operator=(SqL2 const& s) {
    A = s.A;
    B = s.B;
    C = s.C;
    t = s.t;
    mpz_init_set_si(A_,A);
    mpz_init_set_si(B_,B);
    mpz_init_set_si(C_,C);
}


Tri::Cross* Tri::create_cross(Tri::Cross* curcr)
{
    Cross::Root* curcrossroot = curcr != nullptr ? &curcr->crossroot : &root_t0;
    Cross::Root root(*this, curcrossroot, 1);

    if(root.kind == Root::Imaginary)
        return nullptr;

    auto rootneg_root_t1 = root(-1) - root_t1;
    auto rootpos_root_t1 = root( 1) - root_t1;

    // if neither root is in [0,1] return null
    if((root(-1) - root_t0 < 0 || rootneg_root_t1 > 0) &&
       (root( 1) - root_t0 < 0 || rootpos_root_t1 > 0))
        return nullptr;

    // if min root is in [t, 1], choose this root, else
    // if max root is in [t, 1], choose that root, else return null
    if((root(-1) - *curcrossroot >= 0) && (rootneg_root_t1 <= 0))
        root(-1); else
    if((root( 1) - *curcrossroot >= 0) && (rootpos_root_t1 <= 0))
        root(1);
    else
        return nullptr;

    Edge* flip_e = nullptr;

    auto sql2a = Cross::SqL2(&a, &root);
    auto sql2b = Cross::SqL2(&b, &root);
    auto sql2c = Cross::SqL2(&c, &root);

    int overlap = 0;
    if(sql2a() == 0) { ++overlap; flip_e = &a; }
    if(sql2b() == 0) { ++overlap; flip_e = &b; }
    if(sql2c() == 0) { ++overlap; flip_e = &c; }

    if(!overlap) {
        int sgn_ab = (sql2a - sql2b)();
        int sgn_bc = (sql2b - sql2c)();
        int sgn_ca = (sql2c - sql2a)();

        if(sgn_ab > 0 && sgn_ca < 0) flip_e = &a;
        if(sgn_bc > 0 && sgn_ab < 0) flip_e = &b;
        if(sgn_ca > 0 && sgn_bc < 0) flip_e = &c;
    }

    return new Tri::Cross(this, flip_e, ++com->cross_counter, root, overlap);
}

double ecost(Edge const& e) {
    Edge const& j = *e.j;
    return e.t->area() / j.t->area();
}

Vec2 Tri::centroid() const {
    return Vec2(a.n->p_() + b.n->p_() + c.n->p_()) / 3.;
}

Vec2 Tri::clamp(Vec2 o) {
    Vec2 ab(this->ab()), bc(this->bc()), ca(this->ca()), ao(o-a.n->p_());
    if((ab^ao) < 0) o = a.n->p_() + ab.norm()*(ab.norm().dot(ao));
    Vec2 bo(o - b.n->p_());
    if((bc^bo) < 0) o = b.n->p_() + bc.norm()*(bc.norm().dot(bo));
    Vec2 co(o - c.n->p_());
    if((ca^co) < 0) o = c.n->p_() + ca.norm()*(ca.norm().dot(co));
    if((ab^ao) < 0 && (bc^bo) < 0) return b.n->p_();
    if((bc^bo) < 0 && (ca^co) < 0) return c.n->p_();
    if((ca^co) < 0 && (ab^ao) < 0) return a.n->p_();
    return o;
}

Edge* Tri::edge(Tri::Label l)
{
    switch(l) {
    case A:	return &a;
    case B: return &b;
    case C:	return &c;
    default: throw;
    }
}

Tri::Label Tri::label(Edge const* e) const {
    if(e == &a) return A;
    else if(e == &b) return B;
    else if(e == &c) return C;
    return None;
}

Vec3 Tri::p2u(Vec2 const& p) const {
    double ar_pbc = (b.n->p_() - p)^(c.n->p_() - p),
           ar_apc = (p - a.n->p_())^(c.n->p_() - a.n->p_()),
           ar_abp = (b.n->p_() - a.n->p_())^(p - a.n->p_()),
           ar_abc = (b.n->p_() - a.n->p_())^(c.n->p_() - a.n->p_());
    return Vec3(ar_pbc,ar_apc,ar_abp)/ar_abc;
}

void Tri::u_2_e_q(Vec2 const& u2, Edge const** e, double* q) const {
    Vec3 u(u2, 1-u2.u-u2.v);
    if(u.w < u.u && u.w < u.v) {
        *e = &a;
        *q = u.u/(u.u+u.v);
    } else if(u.u < u.v && u.u < u.w) {
        *e = &b;
        *q = u.v/(u.v+u.w);
    } else {
        *e = &c;
        *q = u.w/(u.w+u.u);
    }
}

int64_t Tri::deter(Node& a, Node& b, Node& c) {
    return	a.p_().x*b.p_().y - b.p_().x*a.p_().y +
            b.p_().x*c.p_().y - c.p_().x*b.p_().y +
            c.p_().x*a.p_().y - a.p_().x*c.p_().y;
}

bool Tri::inside(Vec2i const& o) const {
    Vec2i ab(b.n->cp - a.n->cp), ao(o - a.n->cp),
          bc(c.n->cp - b.n->cp), bo(o - b.n->cp),
          ca(a.n->cp - c.n->cp), co(o - c.n->cp);
    return (ab^ao) >= 0 && (bc^bo) >= 0 && (ca^co) >= 0;
}

Vec2 Tri::operator*(Vec3 const& s) const {
    return a.n->p()*s.x + b.n->p()*s.y + c.n->p()*s.z;
}
Vec3 Tri::cntrst() {
    return Vec3(a.n->contrast, b.n->contrast, c.n->contrast);
}

void Tri::retire() {
    status = Dead;
    if(cross != nullptr) cross->invalid = true;
}

void Tri::retire_w_ns() {
    retire();
    a.n->status = b.n->status = c.n->status = Node::Dead;
}






