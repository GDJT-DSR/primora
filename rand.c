#include "rand.h"
static rand_t x, y;

static rand_t splitmix64(rand_t *state)
{
    *state += 0x9e3779b97f4a7c15ull;
    rand_t z = *state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

void romu_srand(rand_t seed)
{
    x = splitmix64(&seed);
    y = splitmix64(&seed);
}

rand_t romu_gen(void)
{
    rand_t xp = x;
    x = 15241094284759029579ULL * y;
    y = y - xp;
    y = (y << 27) | (y >> (64 - 27));
    return xp;
}
