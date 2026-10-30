#ifndef BI_H_
#define BI_H_
#include "mt19937.h"

#if defined(__SIZEOF_INT128__) && !defined(NO_U128)
typedef unsigned long long item_t;
typedef unsigned __int128 ditem_t;
#define ITEM_SIZE 64
#else
typedef unsigned item_t;
typedef unsigned long long ditem_t;
#define ITEM_SIZE 32
#endif

typedef char item_size_is_correct[sizeof(item_t) * 8 == ITEM_SIZE ? 1 : -1];
typedef char ditem_size_is_correct[sizeof(ditem_t) * 4 == ITEM_SIZE ? 1 : -1];

typedef struct bi_size
{
    unsigned limb; // 全部有效的limb数
    unsigned bits; // 部分有效的字节数，in [0,31]
} bi_size;

extern mt19937_t bi_rand;

item_t *bi_create(unsigned size);
item_t *bi_clone(const item_t *origin, unsigned size);
item_t *bi_from_string(const char *s, unsigned *size);

void bi_copy(item_t *restrict dest, const item_t *restrict src, unsigned size);
void bi_zero(item_t *dest, unsigned size);

void bi_random(item_t *val, bi_size size);

// 得到大整数在二进制下的实际大小
bi_size bi_get_size(const item_t *val, unsigned size);

// 得到数的补码
void bi_comp(item_t *, unsigned);

// 比较大小
int bi_cmp(const item_t *restrict l, const item_t *restrict r, unsigned size);

// 加在一起，返回溢出 l += r
int bi_add(item_t *restrict l, const item_t *restrict r, unsigned size);

// l -= r，返回最高位是否借位
int bi_sub(item_t *restrict l, const item_t *restrict r, unsigned size);
// 加一个低精度数，返回溢出
int bi_add_small(item_t *restrict l, item_t r, unsigned size);

void bi_square(const item_t *restrict, unsigned, item_t *restrict, unsigned);

// 求余数 l r l_size
unsigned bi_remainder(item_t *, unsigned, unsigned);

// val size size
void bi_rshift(item_t *, unsigned, unsigned);

void bi_print_hex(const item_t *val, bi_size size);

// 末尾0的个数，返回bit
bi_size bi_end0(const item_t *val, unsigned size);

void bi_double_mod(item_t *restrict val, const item_t *restrict mod, const item_t *restrict nmod, unsigned size);

#endif
