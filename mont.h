#ifndef MONT_H_
#define MONT_H_
#include "bi.h"

extern unsigned mont_size;
extern item_t *mont_n;
extern item_t *mont_n_comp;
extern bi_size mont_detail;
extern item_t *mont_one;
extern item_t *mont_ione;

// set the base of mont is 2^(2^k)
void mont_init(item_t *, unsigned);

// 蒙哥马利约简, 长度为2 * mont_size + 1
void mont_redc(item_t *);
// 蒙哥马利模乘, res 长为 mont_size + 2
void mont_mul(item_t *restrict l, item_t *restrict r, item_t *restrict res);

// len(a) = mont_n_size
void mont_square(item_t *a);

// len(a) = mont_n_size, 传入len(b)
void mont_pow(item_t *restrict a, item_t *restrict b, unsigned size, unsigned shift);

void mont_clean(void);

#endif
