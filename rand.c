#include "rand.h"
static rand_t s[4]; // 内部状态

// 用于初始化种子
static rand_t splitmix64(rand_t *state)
{
    *state += 0x9e3779b97f4a7c15ull;
    rand_t z = *state;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
// 循环左移
static inline rand_t rotl(const rand_t x, int k) { return (x << k) | (x >> (64 - k)); }

void my_srand(rand_t seed)
{
    rand_t sm_state = seed;
    for (int i = 0; i < 4; i++)
    {
        s[i] = splitmix64(&sm_state);
    }
    if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0)
    {
        s[0] = 31241;
        s[1] = 2415131;
        s[2] = 431241312;
        s[3] = 1212123;
    }
}

rand_t my_rand(void)
{
    const rand_t result = rotl(s[1] * 5, 7) * 9;
    const rand_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);

    return result;
}
