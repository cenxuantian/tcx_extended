#pragma once

#include <stdlib.h>
#include <time.h>
#include <math.h>

namespace tcx{
namespace {
// call srand and return the seed
inline int __start_rand(int res){
    srand(res);
    return res;
}

static const int __original_seed = __start_rand(time(NULL));

}

inline int reset_seed(int seed){
    __start_rand(seed);
    return seed;
}

int randint(int start=INT_MIN, int stop_not_included = INT_MAX){
    return (int)((long long)rand() % ((long long)stop_not_included-(long long)start+1)+ start);
}

double rand_percent(int precision = 2){
    double res =  (double)rand() / (double)RAND_MAX;
    int dec = pow(10,precision);
    res = (double)(round(res*dec)) / dec;
    return res;
}



}