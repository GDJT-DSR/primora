#ifndef MONT_H_
#define MONT_H_
#include "bi.h"

extern unsigned mont_size;
extern u64 *mont_n;
extern u64 *mont_n_comp;
extern bi_size mont_detail;
extern u64 *mont_one;
extern u64 *mont_ione;

// set the base of mont is 2^(2^k)
void mont_init(u64 *, unsigned);

// 蒙哥马利约简, 长度为2 * mont_size + 1
void mont_redc(u64 *);
// 蒙哥马利模乘, res 长为 mont_size + 2
void mont_mul(u64 *restrict l, u64 *restrict r, u64 *restrict res);

// len(a) = mont_n_size
void mont_square(u64 *a);

// len(a) = mont_n_size, 传入len(b)
void mont_pow(u64 *restrict a, u64 *restrict b, unsigned size, unsigned shift);

void mont_clean(void);

#endif
