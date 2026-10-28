#include "utils.h"

unsigned roundup_pow(unsigned x)
{
    if (x == 0)
        return 1;

    if ((x & (x - 1)) == 0)
        return x;

#define gen(a) x |= x >> a
    gen(1);
    gen(2);
    gen(4);
    gen(8);
    gen(16);
#undef gen

    return x + 1;
}

unsigned log2_floor(unsigned x)
{
    if (x == 0)
        return 0;
    int result = 0;
#define X(i)                                                                                                           \
    if ((x) >= (1 << (i)))                                                                                             \
    {                                                                                                                  \
        (x) >>= (i);                                                                                                   \
        result += (i);                                                                                                 \
    }

    X(16)
    X(8)
    X(4)
    X(2)
    X(1)
#undef X
    return result;
}
int str_eq(const char *l, const char *r)
{
    while (*l && (*l == *r))
    {
        ++l;
        ++r;
    }

    return *l == '\0' && *r == '\0';
}
char to_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

unsigned str_len(const char *s)
{
    unsigned len = 0;
    while (s[len++])
        ;
    return len - 1;
}
