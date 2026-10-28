#ifndef BI_H_
#define BI_H_
#include "mt19937.h"
#define ITEM_SIZE 32

typedef struct bi_size
{
    unsigned limb; // 全部有效的limb数
    unsigned bits; // 部分有效的字节数，in [0,31]
} bi_size;

extern mt19937_t bi_rand;

unsigned *bi_create(unsigned size);
unsigned *bi_clone(const unsigned *origin, unsigned size);
unsigned *bi_from_string(const char *s, unsigned *size);

void bi_copy(unsigned *dest, const unsigned *src, unsigned size);
void bi_zero(unsigned *dest, unsigned size);

void bi_random(unsigned *val, bi_size size);

// 得到大整数在二进制下的实际大小
bi_size bi_get_size(const unsigned *val, unsigned size);

// 得到某一位
unsigned bi_bit(const unsigned *val, unsigned bit);

// 得到数的补码
void bi_comp(unsigned *, unsigned);

// 比较大小
int bi_cmp(const unsigned *l, const unsigned *r, unsigned size);

// 加在一起，返回溢出 l += r
int bi_add(unsigned *l, const unsigned *r, unsigned size);

// l -= r，返回最高位是否借位
int bi_sub(unsigned *l, const unsigned *r, unsigned size);
// 加一个低精度数，返回溢出
int bi_add_small(unsigned *l, unsigned r, unsigned size);

void bi_mul(const unsigned *, unsigned, const unsigned *, unsigned, unsigned *, unsigned);
void bi_square(const unsigned *, unsigned, unsigned *, unsigned);

// 求余数 l r l_size
unsigned bi_remainder(unsigned *, unsigned, unsigned);

// val size size
void bi_rshift(unsigned *, bi_size, unsigned);

void bi_print_hex(const unsigned *val, bi_size size);

// 末尾0的个数，返回bit
bi_size bi_end0(const unsigned *val, unsigned size);

void bi_double_mod(unsigned *val, const unsigned *mod, const unsigned *nmod, unsigned size);

#endif
