#include "iostream"
#include "iomanip"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "fixed_random.h"


uint32_t GenRand32(void)//generate 32-bit random fixed number
{
    uint32_t r=0;
    srand((uint32_t)time(NULL));
    while(!r)   r=fix_rn32((double)rand()/rand()); //random float number

    return r;

}

uint64_t GenRand64(void)//generate 64-bit random fixed number
{
    uint64_t r=0;
    srand((uint64_t)time(NULL));
    while(!r)   r=fix_rn64((double)rand()/rand()); //random float number

    return r;

}