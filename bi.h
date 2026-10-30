#ifndef BI_H_
#define BI_H_
#include "mt19937.h"
#include "uints.h"
#define ITEM_SIZE 64

typedef struct bi_size
{
    unsigned limb; // 全部有效的limb数
    unsigned bits; // 部分有效的字节数，in [0,31]
} bi_size;

extern mt19937_t bi_rand;

u64 *bi_create(unsigned size);
u64 *bi_clone(const u64 *origin, unsigned size);
u64 *bi_from_string(const char *s, unsigned *size);

void bi_copy(u64 *restrict dest, const u64 *restrict src, unsigned size);
void bi_zero(u64 *dest, unsigned size);

void bi_random(u64 *val, bi_size size);

// 得到大整数在二进制下的实际大小
bi_size bi_get_size(const u64 *val, unsigned size);

// 得到数的补码
void bi_comp(u64 *, unsigned);

// 比较大小
int bi_cmp(const u64 *restrict l, const u64 *restrict r, unsigned size);

// 加在一起，返回溢出 l += r
int bi_add(u64 *restrict l, const u64 *restrict r, unsigned size);

// l -= r，返回最高位是否借位
int bi_sub(u64 *restrict l, const u64 *restrict r, unsigned size);
// 加一个低精度数，返回溢出
int bi_add_small(u64 *restrict l, u64 r, unsigned size);

void bi_mul(const u64 *restrict, unsigned, const u64 *restrict, unsigned, u64 *restrict, unsigned);
void bi_square(const u64 *restrict, unsigned, u64 *restrict, unsigned);

// 求余数 l r l_size
unsigned bi_remainder(u64 *, unsigned, unsigned);

// val size size
void bi_rshift(u64 *, unsigned, unsigned);

void bi_print_hex(const u64 *val, bi_size size);

// 末尾0的个数，返回bit
bi_size bi_end0(const u64 *val, unsigned size);

void bi_double_mod(u64 *restrict val, const u64 *restrict mod, const u64 *restrict nmod, unsigned size);

#endif
