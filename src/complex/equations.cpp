#include "trigon.h"
#include "mpir/mpir.h"
#include "globals.h"

mpz_t Root::t, Root::t1, Root::t2, Root::t3, Root::t4, Root::t5, Root::t6, Root::t7, Root::t8;

void Root::init_mpzs()
{
    mpz_init2(t, 512);
    mpz_init2(t1, 512);
    mpz_init2(t2, 512);
    mpz_init2(t3, 512);
    mpz_init2(t4, 512);
    mpz_init2(t5, 512);
    mpz_init2(t6, 512);
    mpz_init2(t7, 512);
    mpz_init2(t8, 512);
}

void Root::clear_mpzs()
{
    mpz_clears(t, t1, t2, t3, t4, t5, t6, t7, t8, nullptr);
}

int Root::sqsq_sgn_rh1_big(Root const& f, Root const& s)
{
    //sgn(s.sqrtsgn*s.B*s.B*f.A*f.A - 4*s.C*s.A*f.A*f.A) - f.sqrtsgn*(f.B*f.B*s.A*s.A - 4*f.C*f.A*s.A*s.A))
    mpz_mul(t1, s.B_, s.B_); mpz_mul(t1, t1, f.A_); mpz_mul(t1, t1, f.A_);
    mpz_mul_si(t2, s.C_, 4); mpz_mul(t2, t2, s.A_); mpz_mul(t2, t2, f.A_); mpz_mul(t2, t2, f.A_);
    mpz_mul(t3, f.B_, f.B_); mpz_mul(t3, t3, s.A_); mpz_mul(t3, t3, s.A_);
    mpz_mul_si(t4, f.C_, 4); mpz_mul(t4, t4, f.A_); mpz_mul(t4, t4, s.A_); mpz_mul(t4, t4, s.A_);
    mpz_sub(t5, t1, t2); mpz_mul_si(t5, t5, (int)s.sqrtsgn);
    mpz_sub(t6, t3, t4); mpz_mul_si(t6, t6, (int)f.sqrtsgn);
    mpz_sub(t, t5, t6);
    return mpz_sgn(t);
}

int Root::sqsq_sgn_lh3_big(Root const& f, Root const& s)
{
    //sgn(f.C*f.C*s.A*s.A + s.C*s.C*f.A*f.A - 2*f.C*s.C*f.A*s.A - f.B*s.B*f.C*s.A - f.B*s.B*s.C*f.A + f.B*f.B*s.C*s.A + s.B*s.B*f.C*f.A);
    mpz_mul(t1, f.C_, f.C_); mpz_mul(t1, t1, s.A_); mpz_mul(t1, t1, s.A_);
    mpz_mul(t2, s.C_, s.C_); mpz_mul(t2, t2, f.A_); mpz_mul(t2, t2, f.A_);
    mpz_mul(t3, f.C_, s.C_); mpz_mul(t3, t3, f.A_); mpz_mul(t3, t3, s.A_);
    mpz_mul(t4, f.C_, s.C_); mpz_mul(t4, t4, f.A_); mpz_mul(t4, t4, s.A_);
    mpz_mul(t5, f.B_, s.B_); mpz_mul(t5, t5, f.C_); mpz_mul(t5, t5, s.A_);
    mpz_mul(t6, f.B_, s.B_); mpz_mul(t6, t6, s.C_); mpz_mul(t6, t6, f.A_);
    mpz_mul(t7, f.B_, f.B_); mpz_mul(t7, t7, s.C_); mpz_mul(t7, t7, s.A_);
    mpz_mul(t8, s.B_, s.B_); mpz_mul(t8, t8, f.C_); mpz_mul(t8, t8, f.A_);
    mpz_add(t, t1, t2); mpz_sub(t, t, t3); mpz_sub(t, t, t4); mpz_sub(t, t, t5); mpz_sub(t, t, t6); mpz_add(t, t, t7); mpz_add(t, t, t8);
    return mpz_sgn(t);
}

int Cross::SqL2::operator()() const
{
    if(t->kind == Cross::Root::Square) {
        //int64_t V = A*(t->B*t->B - 2*t->A*t->C) - t->A*t->B*B + 2*t->A*t->A*C;
        mpz_mul(Root::t1, t->B_, t->B_); mpz_mul_si(Root::t2, t->A_, 2);mpz_mul(Root::t2, Root::t2, t->C_);
        mpz_sub(Root::t1, Root::t1, Root::t2); mpz_mul(Root::t1, Root::t1, A_);
        mpz_mul(Root::t2, t->A_, t->B_); mpz_mul(Root::t2, Root::t2, B_); mpz_sub(Root::t1, Root::t1, Root::t2);
        mpz_mul_si(Root::t2, t->A_, 2); mpz_mul(Root::t2, Root::t2, t->A_); mpz_mul(Root::t2, Root::t2, C_); mpz_add(Root::t1, Root::t1, Root::t2);
        int sgnV = mpz_sgn(Root::t1);

        //int64_t sgn_R = sgn(t->B*t->B - 4*t->A*t->C);
        /*mpz_mul(Root::t1, t->B_, t->B_); mpz_mul_si(Root::t2, t->A_, 4); mpz_mul(Root::t2, Root::t2, t->C_); mpz_sub(Root::t1, Root::t1, Root::t2);
        int sgnR = mpz_sgn(Root::t1);*/
        int sgnR = sgn(t->B*t->B - 4*t->A*t->C);

        //int64_t H = (int64_t)sgn(t->A)*-(int64_t)t->sqrtsgn*sgn_R*(t->A*B - A*t->B);
        /*mpz_mul(Root::t1, t->A_, B_); mpz_mul(Root::t2, A_, t->B_); mpz_sub(Root::t1, Root::t1, Root::t2); mpz_mul_si(Root::t1, Root::t1, -sgn(t->A)*t->sqrtsgn*sgnR);
        int sgnH = mpz_sgn(Root::t1);*/
        int sgnH = sgn(sgn(t->A)*-t->sqrtsgn*sgnR*(t->A*B - A*t->B));

        if(sgnV*sgnH <= 0) {
            return sgnV - sgnH;
        } else {
            mpz_mul_si(Root::t1, A_, (2*(t->B*t->B) - 4*t->A*t->C));
            mpz_mul_si(Root::t2, C_, (4*t->A*t->A));
            mpz_mul_si(Root::t3, B_, (2*t->A)*-(t->B));
            mpz_add(Root::t4, Root::t1, Root::t2); mpz_add(Root::t4, Root::t4, Root::t3);
            //int64_t V2 = A*(2*(t->B*t->B) - 4*t->A*t->C) + (4*t->A*t->A)*C + (2*t->A)*-(t->B)*B;
            mpz_mul(Root::t4, Root::t4, Root::t4); //V2*V2

            mpz_init_set_si(Root::t1, (t->B*t->B - 4*t->A*t->C));
            mpz_init_set_si(Root::t2, (2*t->A*B - 2*A*(t->B)));
            mpz_mul(Root::t3, Root::t1, Root::t2); mpz_mul(Root::t3, Root::t3, Root::t2);
            //int64_t H2 = (t->B*t->B - 4*t->A*t->C) * (2*t->A*B - 2*A*(t->B)) * (2*t->A*B - 2*A*(t->B));

            mpz_mul_si(Root::t4, Root::t4, sgnV);
            mpz_mul_si(Root::t3, Root::t3, sgnH);
            mpz_sub(Root::t5, Root::t4, Root::t3);
            int s = mpz_sgn(Root::t5);

            return s;
        }

    } else if(t->kind == Cross::Root::Linear) {
        //sgn(A*t->C*t->C - B*t->C*t->B + C*t->B*t->B);
        mpz_mul(Root::t1, A_, t->C_); mpz_mul(Root::t1, Root::t1, t->C_);
        mpz_mul(Root::t2, B_, t->C_); mpz_mul(Root::t2, Root::t2, t->B_); mpz_sub(Root::t1, Root::t1, Root::t2);
        mpz_mul(Root::t2, C_, t->B_); mpz_mul(Root::t2, Root::t2, t->B_); mpz_add(Root::t1, Root::t1, Root::t2);
        int s = mpz_sgn(Root::t1);
        return s;
    }

    return 0;
}


int Root::sqsq_sgn_lh2_big(Root const& f, Root const& s)
{
    //sgn(-f.B*s.B + 2*f.C*s.A + 2*s.C*f.A)
    mpz_mul(t1, f.B_, s.B_);
    mpz_mul_si(t2, f.C_, 2); mpz_mul(t2, t2, s.A_);
    mpz_mul_si(t3, s.C_, 2); mpz_mul(t3, t3, f.A_);
    mpz_sub(t, t2, t1); mpz_add(t, t, t3);
    return mpz_sgn(t);
}

int Root::sqsq_sgn_rh2_big_2(const Root &f, const Root &s)
{
    //(B_s^2 - 4*A_s*C_s)*(B_f^2 - 4*A_f*C_f)
    mpz_mul(t1, s.B_, s.B_);
    mpz_mul(t2, f.B_, f.B_);
    mpz_mul_si(t3, s.A_, 4); mpz_mul(t3, t3, s.C_);
    mpz_mul_si(t4, f.A_, 4); mpz_mul(t4, t4, f.C_);
    mpz_sub(t1, t1, t3);
    mpz_sub(t2, t2, t4);
    mpz_mul(t3, t1, t2);
    return mpz_sgn(t3);
}

int Root::init_sq_sgn_lh1_big()
{
    //sgn(B*B - 4*C*A)
    mpz_mul(t1, B_, B_);
    mpz_mul_si(t2, C_, 4); mpz_mul(t2, t2, A_);
    mpz_sub(t, t1, t2);
    return mpz_sgn(t);
}

int Root::linlin_sgn_lh1_big(Root const& f, Root const& s)
{
    //sgn(s.C*f.B - f.C*s.B)
    mpz_mul(t1, s.C_, f.B_);
    mpz_mul(t2, f.C_, s.B_);
    mpz_sub(t, t1, t2);
    return mpz_sgn(t);
}

int Root::linsq_sgn_lh1_big(Root const& f, Root const& s)
{
    //sgn(f.B*s.B - 2*f.C*s.A)
    mpz_mul(t1, f.B_, s.B_);
    mpz_mul_si(t2, f.C_, 2); mpz_mul(t2, t2, s.A_);
    mpz_sub(t, t1, t2);
    return mpz_sgn(t);
}

int Root::linsq_sgn_rh1_big(Root const& s)
{
    //sgn(s.B*s.B - 4*s.C*s.A)
    mpz_mul(t1, s.B_, s.B_);
    mpz_mul_si(t2, s.C_, 4); mpz_mul(t2, t2, s.A_);
    mpz_sub(t, t1, t2);
    return mpz_sgn(t);
}

int Root::linsq_sgn_lh2_big(Root const& f, Root const& s)
{
    //sgn(f.C*f.C*s.A - f.B*s.B*f.C + f.B*f.B*s.C)
    mpz_mul(t1, f.C_, f.C_); mpz_mul(t1, t1, s.A_);
    mpz_mul(t2, f.B_, s.B_); mpz_mul(t2, t2, f.C_);
    mpz_mul(t3, f.B_, f.B_); mpz_mul(t3, t3, s.C_);
    mpz_sub(t, t1, t2); mpz_add(t, t, t3);
    return mpz_sgn(t);
}

int Root::sqsq_sgn_lh1_big(Root const& f, Root const& s)
{
    //sgn(s.B*f.A - f.B*s.A)
    //mpz_mul(t1, f.A_, s.A_);
    mpz_mul(t2, s.B_, f.A_);
    mpz_mul(t3, f.B_, s.A_);
    mpz_sub(t, t2, t3);
    return mpz_sgn(t);
}

