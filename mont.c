#include "mont.h"
#include "bi.h"
#include "uints.h"
#include "utils.h"
#include <stdlib.h>

u64 *mont_n;
u64 *mont_n_comp; // 2^s - n
u64 *mont_one;
u64 *mont_ione;
u64 mont_n0inv;     // -N^{-1} mod 2^64
unsigned mont_size; // R = 2^(64*mont_size)
bi_size mont_detail;
u64 *mont_square_tmp;
#define WINDOW_SIZE 8
typedef unsigned long long uli;

void mont_init(u64 *restrict n, unsigned size)
{
    if (!size)
        return;

    mont_clean();
    mont_detail = bi_get_size(n, size);

    mont_n = n;
    unsigned ms = mont_detail.limb + (mont_detail.bits != 0);
    mont_size = ms;

    mont_n_comp = bi_clone(mont_n, ms);
    bi_comp(mont_n_comp, ms);

    mont_n0inv = 1;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;
    mont_n0inv *= (u64)2 + n[0] * mont_n0inv;

    // 计算mont_one, mont_ione
    /*
    mont_detail = {l,b}
    2^{64l+b} mod N = 2^{64l+b} - N
    64-b次double_mod
    */

    mont_one = bi_clone(mont_n_comp, ms);
    if (mont_detail.bits != 0)
    {
        mont_one[mont_detail.limb] &= ((u64)1 << mont_detail.bits) - 1;
        for (unsigned j = 0; j < ITEM_SIZE - mont_detail.bits; ++j)
        {
            bi_double_mod(mont_one, mont_n, mont_n_comp, ms);
        }
    }
    mont_ione = bi_clone(n, ms);
    bi_sub(mont_ione, mont_one, ms);
    // mont_n[mont_detail.limb] |= 1u << (mont_detail.bits);
    mont_square_tmp = bi_create(2 * ms + 1);
}

void mont_redc(u64 *restrict val)
{
    const u64 *restrict n = mont_n;
    unsigned ms = mont_size;
    for (unsigned i = 0; i < ms; ++i)
    {
        u64 *v = val + i;
        // m = T[i] * (-n^{-1}) mod 2^64
        u64 m = val[i] * mont_n0inv;
        uli car = 0;

        for (unsigned j = 0; j < ms; ++j)
        {
            u128 acc = u128_mul(m, n[j]);
            u128_self_add(acc, v[j]);
            u128_self_add(acc, car);
            v[j] = u128_low(acc);
            car = u128_high(acc);
        }

        unsigned k = i + ms;
        for (; car && k < ms * 2 + 1; ++k)
        {
            val[k] += car;
            car = val[k] < car;
        }
    }
    bi_rshift(val, ms, 2 * ms + 1);
    if (val[ms] || bi_cmp(val, mont_n, ms) >= 0)
    {
        // val[ms] = !bi_sub(val, mont_n, ms);
        bi_add(val, mont_n_comp, ms);
        val[ms] = 0;
    }
}

void mont_mul(u64 *restrict l, u64 *restrict r, u64 *restrict res)
{
    const u64 inv = mont_n0inv;
    const unsigned ms = mont_size;
    const u64 *restrict n = mont_n;
    // bi_mul(l, mont_size, r, mont_size, res, 2 * mont_size + 1);
    // mont_redc(res);
    uli car = 0;

    for (unsigned i = 0; i < ms; ++i)
    {
        // res += l * r[i];
        car = 0;
        for (unsigned j = 0; j < ms; ++j)
        {
            u128 acc = u128_mul(l[j], r[i]);
            u128_self_add(acc, res[j]);
            u128_self_add(acc, car);
            res[j] = u128_low(acc);
            car = u128_high(acc);
        }
        res[ms] += car;
        res[ms + 1] = res[ms] < car;

        // REDC
        u64 m = res[0] * inv;
        u128 acc = u128_mul(m, n[0]);
        u128_self_add(acc, res[0]);
        car = u128_high(acc);
        for (unsigned j = 1; j < ms; ++j)
        {
            u128 acc = u128_mul(m, n[j]);
            u128_self_add(acc, res[j]);
            u128_self_add(acc, car);
            res[j - 1] = u128_low(acc);
            car = u128_high(acc);
        }
        res[ms - 1] = car + res[ms];
        res[ms] = res[ms + 1] + (res[ms - 1] < car);

        if (res[ms] || bi_cmp(res, n, ms) >= 0)
        {
            bi_add(res, mont_n_comp, ms);
            res[ms] = 0;
        }
        res[ms + 1] = 0;
    }
}

void mont_square(u64 *restrict a)
{
    u64 *restrict tmp = mont_square_tmp;
    bi_zero(tmp, 2 * mont_size + 1);
    bi_square(a, mont_size, tmp, 2 * mont_size + 1);
    mont_redc(tmp);
    for (unsigned i = 0; i < mont_size; ++i)
    {
        a[i] = tmp[i];
    }
}

// 单个limb的滑动窗口
inline static void mont_single(u64 *restrict a, u64 *restrict b, u64 *restrict res, u64 *restrict table, unsigned l,
                               unsigned ms, int end)
{
    u64 current = b[l];
    for (int high = ITEM_SIZE - 1; high >= end;)
    {
        if ((current & ((u64)1 << high)) == 0)
        {
            mont_square(a);
            --high;
            continue;
        }
        int low = high - WINDOW_SIZE + 1;
        if (low < 0)
            low = 0;
        while ((current & ((u64)1 << low)) == 0)
            ++low;
        mont_square(a);
        unsigned val = 1;
        for (int k = high - 1; k >= low; --k)
        {
            val <<= 1;
            val |= (current >> k) & 1;
            mont_square(a);
        }
        bi_zero(res, ms + 2);
        mont_mul(a, table + (val >> 1) * ms, res);
        bi_copy(a, res, ms);
        high = low - 1;
    }
}
void mont_pow(u64 *a, u64 *b, unsigned size, unsigned shift)
{
    unsigned ms = mont_size;
    bi_size bs = bi_get_size(b, size);
    if (bs.limb * ITEM_SIZE + bs.bits <= shift)
    {
        bi_copy(a, mont_one, ms);
        return;
    }
    // unsigned nbits = bs.limb * ITEM_SIZE + bs.bits - shift;
    const unsigned table_count = 1u << (WINDOW_SIZE - 1);
    u64 *table = bi_create(table_count * ms);
    // size = bi_get_size(b, size).limb + 1;
    // size *= ITEM_SIZE;
    u64 *res = bi_create(ms + 2);
    u64 *a2 = bi_create(2 * ms + 1);
    // 处理a > n的情况
    if (bi_cmp(a, mont_n, ms) >= 0)
    {
        mont_mul(a, mont_one, res);
        bi_copy(table, res, ms);
    }
    else
    {
        bi_copy(table, a, ms);
    }

    // 滑动窗口预计算 a a^3 a^5 ...
    bi_copy(a2, table, ms);
    mont_square(a2);
    for (unsigned i = 1; i < table_count; ++i)
    {
        u64 *dst = table + i * ms;
        bi_zero(res, ms + 2);
        mont_mul(dst - ms, a2, res);
        bi_copy(dst, res, ms);
    }

    unsigned limb = shift / ITEM_SIZE;
    unsigned rem = shift - ITEM_SIZE * limb;
    bi_copy(a, mont_one, ms);
    for (unsigned l = ms - 1; l > limb; l--)
    {
        mont_single(a, b, res, table, l, ms, 0);
    }
    mont_single(a, b, res, table, limb, ms, (int)rem);
    // for (int high = (int)nbits - 1; high >= 0;)
    // {
    //     unsigned pos = (unsigned)high + shift;
    //     if (!bi_bit(b, pos))
    //     {
    //         mont_square(a);
    //         --high;
    //         continue;
    //     }
    //
    //     int low = high - WINDOW_SIZE + 1;
    //     if (low < 0)
    //         low = 0;
    //
    //     // 缩短窗口使得最后一位是1
    //     while (!bi_bit(b, (unsigned)low + shift))
    //         ++low;
    //
    //     int len = high - low + 1;
    //     unsigned val = 1;
    //     for (int k = high - 1; k >= low; --k)
    //     {
    //         val <<= 1;
    //         val |= bi_bit(b, (unsigned)k + shift);
    //     }
    //     for (int k = 0; k < len; ++k)
    //     {
    //         mont_square(a);
    //     }
    //     bi_zero(res, ms + 2);
    //     mont_mul(a, table + (val >> 1) * ms, res);
    //     bi_copy(a, res, ms);
    //
    //     high = low - 1;
    // }
    //
    // for (unsigned i = 0; i < nbits; ++i, mont_square(tmp))
    // {
    //     if (!bi_bit(b, i + shift))
    //         continue;
    //     bi_zero(res, 2 * ms + 1);
    //     bi_mul(a, ms, tmp, ms, res, 2 * ms);
    //     mont_redc(res);
    //     bi_copy(a, res, ms);
    // }
    free(res);
    free(table);
    free(a2);
    // free(tmp);
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
