#include "mt19937.h"

#define MT_M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

void mt19937_init(mt19937_t *mt, unsigned seed)
{
    mt->mt[0] = seed;
    for (mt->mti = 1; mt->mti < MT_N; mt->mti++)
    {
        mt->mt[mt->mti] = (1812433253UL * (mt->mt[mt->mti - 1] ^ (mt->mt[mt->mti - 1] >> 30)) + (unsigned)mt->mti);
    }
}

unsigned mt19937_gen(mt19937_t *mt)
{
    unsigned y;
    static const unsigned mag01[2] = {0x0UL, MATRIX_A};

    if (mt->mti >= MT_N)
    {
        int kk;

        for (kk = 0; kk < MT_N - MT_M; kk++)
        {
            y = (mt->mt[kk] & UPPER_MASK) | (mt->mt[kk + 1] & LOWER_MASK);
            mt->mt[kk] = mt->mt[kk + MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < MT_N - 1; kk++)
        {
            y = (mt->mt[kk] & UPPER_MASK) | (mt->mt[kk + 1] & LOWER_MASK);
            mt->mt[kk] = mt->mt[kk + (MT_M - MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt->mt[MT_N - 1] & UPPER_MASK) | (mt->mt[0] & LOWER_MASK);
        mt->mt[MT_N - 1] = mt->mt[MT_M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mt->mti = 0;
    }

    y = mt->mt[mt->mti++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}
