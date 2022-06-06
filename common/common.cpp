#include "common.h"

//the counter is used for keeping track of the random seed.
int r_count = 0;
int rand_count()
{
    ++r_count;
    return rand();
}

//pseudo-normal distribution
double rand_binom(int n)
{
    double acc = 0.0;
    for(auto i = 0; i < n*2; ++i) {
        acc += rand_uni();
    }
    return acc  - (double)n;
}
