#ifndef MT19937_H_
#define MT19937_H_

#define MT_N 624
typedef struct
{
    unsigned mt[MT_N];
    int mti;
} mt19937_t;
void mt19937_init(mt19937_t *, unsigned);
unsigned mt19937_gen(mt19937_t *);

#endif
