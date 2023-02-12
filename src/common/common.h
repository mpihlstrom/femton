#pragma once

#include "common/vector.h"
#include <string>

struct Complex;
struct Camera;
struct Tool;
struct iPointer;
struct Col;

#define Uint32 uint32_t
#define Sint64 int64_t
#define Uint8 uint8_t

double rand_binom(int n = 10);
int rand_count();
#define rand_uni() (((double) rand_count() / (RAND_MAX)))
#define rand_int() (rand_count())
#define rand_pn1() (rand_uni()*2.0 - 1.0)

#ifndef M_PI
    #define M_PI (3.14159265359)
#endif


#define rad(a) (2.*M_PI * (double)(a) / 360.0)
#define deg(a) (360. * (double)(a) / (2.*M_PI))

#define my_max(x, y) (((x) > (y)) ? (x) : (y))
#define my_min(x, y) (((x) < (y)) ? (x) : (y))

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

#define eqv (((x) && (y)) || (^(x) && ^(y)))

struct NoDebug {
    NoDebug const& operator<<(int const&) const { return *this; }
    NoDebug const& operator<<(int64_t const&) const { return *this; }
    NoDebug const& operator<<(unsigned long long const&) const { return *this; }
    NoDebug const& operator<<(double const&) const { return *this; }
    NoDebug const& operator<<(std::string const&) const { return *this; }
};

#define MY_DEBUG
#ifdef MY_DEBUG
    #define myDebug qDebug
#else
    #define myDebug NoDebug
#endif

#define myDebugS debug_strstr

//#define _HAS_ITERATOR_DEBUGGING 0

//#define _ITERATOR_DEBUG_LEVEL 0
