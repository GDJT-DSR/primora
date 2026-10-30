#include "utils.h"

int str_eq(const char *l, const char *r)
{
    while (*l && (*l == *r))
    {
        ++l;
        ++r;
    }

    return *l == '\0' && *r == '\0';
}

unsigned str_len(const char *s)
{
    unsigned len = 0;
    while (s[len++])
        ;
    return len - 1;
}
