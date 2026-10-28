#ifndef MONT_H_
#define MONT_H_
#include "bi.h"

extern unsigned mont_size;
extern unsigned *mont_n;
extern unsigned *mont_n_comp;
extern bi_size mont_detail;
extern unsigned *mont_one;
extern unsigned *mont_ione;

// set the base of mont is 2^(2^k)
void mont_init(unsigned *, unsigned);

// 蒙哥马利约简, 长度为2 * mont_size + 1
void mont_redc(unsigned *);
// 蒙哥马利模乘, res 长为2 * mont_size + 1
void mont_mul(unsigned *l, unsigned *r, unsigned *res);

// len(a) = mont_n_size
void mont_square(unsigned *a);

// len(a) = mont_n_size, 传入len(b)
void mont_pow(unsigned *a, unsigned *b, unsigned size);

void mont_clean(void);

#endif
