#include "mont.h"
#include "bi.h"
#include "utils.h"
#include <stdlib.h>

unsigned *mont_n;
unsigned *mont_n_comp; // 2^s - n
unsigned *mont_one;
unsigned *mont_ione;
unsigned mont_n0inv; // -N^{-1} mod 2^32
unsigned mont_size;  // R = 2^(32*mont_size)
bi_size mont_detail;
unsigned *mont_square_tmp;

void mont_init(unsigned *n, unsigned size)
{
    if (!size)
        return;

    mont_clean();
    mont_detail = bi_get_size(n, size);

    mont_n = n;
    mont_size = mont_detail.limb + (mont_detail.bits != 0);

    mont_n_comp = bi_clone(mont_n, mont_size);
    bi_comp(mont_n_comp, mont_size);

    mont_n0inv = 1;
    mont_n0inv *= 2u + n[0] * mont_n0inv;
    mont_n0inv *= 2u + n[0] * mont_n0inv;
    mont_n0inv *= 2u + n[0] * mont_n0inv;
    mont_n0inv *= 2u + n[0] * mont_n0inv;
    mont_n0inv *= 2u + n[0] * mont_n0inv;

    // 计算mont_one, mont_ione
    /*
    mont_detail = {l,b}
    2^{32l+b} mod N = 2^{32l+b} - N
    32-b次double_mod
    */

    mont_one = bi_clone(mont_n_comp, mont_size);
    if (mont_detail.bits != 0)
    {
        mont_one[mont_detail.limb] &= (1u << mont_detail.bits) - 1;
        for (unsigned j = 0; j < 32 - mont_detail.bits; ++j)
        {
            bi_double_mod(mont_one, mont_n, mont_n_comp, mont_size);
        }
    }
    mont_ione = bi_clone(mont_n, mont_size);
    bi_sub(mont_ione, mont_one, mont_size);
    // mont_n[mont_detail.limb] |= 1u << (mont_detail.bits);
    mont_square_tmp = bi_create(2 * mont_size + 1);
}

void mont_redc(unsigned *val)
{

    for (unsigned i = 0; i < mont_size; ++i)
    {
        // m = T[i] * (-n^{-1}) mod 2^32
        unsigned m = val[i] * mont_n0inv;
        unsigned long long car = 0;

        for (unsigned j = 0; j < mont_size; ++j)
        {
            car += (unsigned long long)m * mont_n[j] + val[i + j];
            val[i + j] = (unsigned)car;
            car >>= ITEM_SIZE;
        }

        unsigned k = i + mont_size;
        for (; car && k < mont_size * 2 + 1; ++k)
        {
            unsigned long long cur = (unsigned long long)val[k] + car;
            val[k] = (unsigned)cur;
            car = cur >> ITEM_SIZE;
        }
    }
    bi_rshift(val, (bi_size){.limb = mont_size, .bits = 0}, 2 * mont_size + 1);
    if (val[mont_size] || bi_cmp(val, mont_n, mont_size) >= 0)
    {
        // val[mont_size] = !bi_sub(val, mont_n, mont_size);
        bi_add(val, mont_n_comp, mont_size);
        val[mont_size] = 0;
    }
}

void mont_mul(unsigned *l, unsigned *r, unsigned *res)
{
    bi_mul(l, mont_size, r, mont_size, res, 2 * mont_size + 1);
    mont_redc(res);
}

void mont_square(unsigned *a)
{
    unsigned *tmp = mont_square_tmp;
    bi_zero(tmp, 2 * mont_size + 1);
    bi_square(a, mont_size, tmp, 2 * mont_size + 1);
    mont_redc(tmp);
    for (unsigned i = 0; i < mont_size; ++i)
    {
        a[i] = tmp[i];
    }
}
void mont_pow(unsigned *a, unsigned *b, unsigned size)
{
    size = bi_get_size(b, size).limb + 1;
    size *= ITEM_SIZE;
    unsigned *tmp = bi_clone(a, mont_size);
    unsigned *res = bi_create(2 * mont_size + 1);
    // 处理a > n的情况
    if (bi_cmp(a, mont_n, mont_size) >= 0)
    {
        mont_mul(a, mont_one, res);
        bi_copy(a, res, mont_size);
    }
    bi_copy(a, mont_one, mont_size);
    for (unsigned i = 0; i < size; ++i, mont_square(tmp))
    {
        if (!bi_bit(b, i))
            continue;
        bi_zero(res, 2 * mont_size + 1);
        bi_mul(a, mont_size, tmp, mont_size, res, 2 * mont_size);
        mont_redc(res);
        bi_copy(a, res, mont_size);
    }

    free(res);
    free(tmp);
}

void mont_clean(void)
{
#define FREE_AND_CLEAR(v)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        free((v));                                                                                                     \
        (v) = NULL;                                                                                                    \
    } while (0)
    FREE_AND_CLEAR(mont_n_comp);
    FREE_AND_CLEAR(mont_one);
    FREE_AND_CLEAR(mont_ione);
    FREE_AND_CLEAR(mont_square_tmp);
}
