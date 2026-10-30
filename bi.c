#include "bi.h"
#include "mt19937.h"
#include "uints.h"
#include "utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

mt19937_t bi_rand;
u64 *bi_clone(const u64 *origin, unsigned size)
{
    u64 *tmp = malloc(sizeof(u64) * size);
    bi_copy(tmp, origin, size);
    return tmp;
}

u64 *bi_create(unsigned size)
{
    u64 *tmp = malloc(sizeof(u64) * size);
    for (unsigned i = 0; i < size; ++i)
    {
        tmp[i] = 0;
    }
    return tmp;
}
void bi_copy(u64 *dest, const u64 *src, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        dest[i] = src[i];
    }
}
void bi_random(u64 *val, bi_size size)
{
    static mt19937_t *m = &bi_rand;
    for (unsigned i = 0; i < size.limb; ++i)
    {
        // val[i] =
        // (unsigned)((rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24));
        val[i] = mt19937_gen(m);
        val[i] |= (u64)mt19937_gen(m) << 32;
    }
    if (size.bits)
    {
        // val[size.limb] = mt19937_gen(m) & ((1u << size.bits) - 1);
        val[size.limb] = mt19937_gen(m);
        val[size.limb] |= (u64)mt19937_gen(m) << 32;
        val[size.limb] &= ((u64)1 << size.bits) - 1;
    }
    else
    {
        val[size.limb] = 0;
    }
}
bi_size bi_get_size(const u64 *val, unsigned size)
{
    bi_size res = {0, 0};
    if (!size)
        return res;
    for (unsigned i = size - 1; i > 0; --i)
    {
        if (!val[i])
            continue;
        res.limb = i;
        break;
    }
    u64 li = val[res.limb];
    if (li >> (ITEM_SIZE - 1))
    {
        ++res.limb;
        res.bits = 0;
        return res;
    }
    for (; li; ++res.bits)
    {
        li >>= 1;
    }
    return res;
}
void bi_zero(u64 *v, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        v[i] = 0;
    }
}
void bi_print_hex(const u64 *val, bi_size size)
{
    static char bitset[] = "0123456789abcdef";
    printf("0x");
    unsigned time = (size.bits + 3) / 4;
    char *buf = malloc(16 * size.limb + time + 1);
    unsigned i = 0;
    do
    {
        while (time-- > 0)
        {
            // putchar(bitset[(val[size.limb] >> (4 * time)) & 0xf]);
            buf[i++] = bitset[(val[size.limb] >> (4 * time)) & 0xf];
        }
        time = 16;
    } while (size.limb-- > 0);
    buf[i] = '\0';
    puts(buf);
    free(buf);
}
void bi_comp(u64 *val, unsigned size)
{
    // 按位取反再加1
    for (unsigned i = 0; i < size; ++i)
        val[i] = ~val[i];
    bi_add_small(val, 1, size);
}

int bi_add(u64 *l, const u64 *r, unsigned s)
{
    u64 car = 0;
    for (unsigned i = 0; i < s; ++i)
    {
        u64 sum = l[i] + r[i];
        int b1 = sum < l[i];
        u64 sum2 = sum + car;
        int b2 = sum2 < sum;
        l[i] = sum2;
        car = b1 || b2;
    }
    return (int)car;
}
int bi_add_small(u64 *l, u64 r, unsigned s)
{
    u64 car = r;
    for (unsigned i = 0; i < s; ++i)
    {
        l[i] += car;
        car = l[i] < car;
    }
    return (int)car;
}

void bi_mul(const u64 *l, unsigned ls, const u64 *r, unsigned rs, u64 *t, unsigned os)
{
    for (unsigned i = 0; i < ls; ++i)
    {
        // 进位
        u64 car = 0;
        for (unsigned j = 0; j < rs; ++j)
        {
            u128 acc = u128_mul(l[i], r[j]);
            u128_self_add(acc, t[i + j]);
            u128_self_add(acc, car);

            t[i + j] = u128_low(acc);
            car = u128_high(acc);
        }
        unsigned k = i + rs;
        for (; car && k < os; ++k)
        {
            t[k] += car;
            car = t[k] < car;
        }
    }
}
void bi_square(const u64 *a, unsigned sa, u64 *r, unsigned sr)
{
    u64 car = 0;
    // 计算交叉项
    for (unsigned i = 0; i < sa; ++i)
    {
        u64 ai = a[i];
        for (unsigned int j = i + 1; j < sa; ++j)
        {
            // car += ai * a[j] + r[i + j];
            // r[i + j] = (unsigned)car;
            // car >>= ITEM_SIZE;
            u128 acc = u128_mul(ai, a[j]);
            u128_self_add(acc, r[i + j]);
            u128_self_add(acc, car);

            r[i + j] = u128_low(acc);
            car = u128_high(acc);
        }

        unsigned k = i + sa;
        for (; car && k < sr; ++k)
        {
            r[k] += car;
            car = r[k] < car;
        }
    }

    // 给交叉项乘2
    car = 0;
    for (unsigned i = 0; i < sr; ++i)
    {
        u64 tmp = r[i];
        r[i] = (tmp << 1) | car;
        car = tmp >> (ITEM_SIZE - 1);
    }

    // 处理平方项
    for (unsigned i = 0; i < sa; ++i)
    {
        u128 acc = u128_mul(a[i], a[i]);
        u128_self_add(acc, r[2 * i]);
        r[2 * i] = u128_low(acc);
        car = u128_high(acc);
        for (unsigned k = 2 * i + 1; car && k < sr; ++k)
        {
            r[k] += car;
            car = r[k] < car;
        }
    }
}
int bi_cmp(const u64 *l, const u64 *r, unsigned s)
{

    while (s > 0)
    {
        --s;
        if (l[s] > r[s])
        {
            return 1;
        }
        else if (l[s] < r[s])
        {
            return -1;
        }
    }
    return 0;
}
void bi_rshift(u64 *val, unsigned shift, unsigned size)
{
    if (shift >= size)
    {
        bi_zero(val, size);
        return;
    }
    unsigned n = size - shift;
    for (unsigned i = 0; i < n; ++i)
        val[i] = val[i + shift];
    for (unsigned i = size - shift; i < size; ++i)
        val[i] = 0;
}

unsigned bi_remainder(u64 *val, unsigned target, unsigned size)
{
    if (target == 0)
        return 0;

    unsigned long long rem = 0;

    for (unsigned i = size; i-- > 0;)
    {
        // 按32位做除法
        u64 v = val[i];
        rem = ((rem << 32) | (v >> 32)) % target;
        rem = ((rem << 32) | (u64)(u32)v) % target;
    }

    return (unsigned)rem;
}
// 末尾0的个数
bi_size bi_end0(const u64 *val, unsigned size)
{
    unsigned i = 0;
    for (; i < size && !val[i]; ++i)
        ;
    if (i == size)
        return (bi_size){.limb = size, .bits = 0};

    u64 bit = val[i]; // != 0
    unsigned count = 0;
    for (; !(bit & 1u); ++count)
        bit >>= 1;
    return (bi_size){.limb = i, .bits = count};
}
void bi_double_mod(u64 *val, const u64 *mod, const u64 *nmod, unsigned size)
{
    u64 car = 0;

    for (unsigned i = 0; i < size; ++i)
    {
        u64 tmp = val[i];
        val[i] = (tmp << 1) | car;
        car = tmp >> (ITEM_SIZE - 1);
    }

    if (car || bi_cmp(val, mod, size) >= 0)
        bi_add(val, nmod, size);
}
int bi_sub(u64 *l, const u64 *r, unsigned size)
{
    u64 borrow = 0;
    for (unsigned i = 0; i < size; ++i)
    {
        u64 li = l[i];
        u64 ri = r[i];

        u64 t1 = li - ri;
        int b1 = li < ri;
        u64 t2 = t1 - borrow;
        int b2 = t1 < borrow;
        l[i] = t2;
        borrow = b1 || b2;
    }

    return (int)borrow;
}

static unsigned parse_single(char c)
{

    if (c >= '0' && c <= '9')
    {
        return (unsigned)c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return (unsigned)c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F')
    {
        return (unsigned)c - 'A' + 10;
    }
    return 17;
}
u64 *bi_from_string(const char *s, unsigned *size)
{
    unsigned len = str_len(s);
    // 1. 安全检查长度，并支持 0X 前缀
    if (len < 3 || s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
    {
        return NULL;
    }

    unsigned hex_len = len - 2;
    *size = (hex_len + 15) / 16;
    u64 *res = bi_create(*size);

    for (unsigned i = 0; i < hex_len; ++i)
    {
        // 从字符串右侧向左读，i=0 对应最低位的字符
        char c = s[len - 1 - i];
        u64 val = parse_single(c);

        if (val == 17)
        {
            free(res);
            return NULL;
        }

        unsigned index = i / 16;       // limb
        unsigned shift = (i % 16) * 4; // bit

        res[index] |= (val << shift);
    }

    return res;
}
// u64*bi_from_string(const char *s, u64*size)
// {
//     unsigned len = str_len(s);
//     if (s[0] == '0' && s[1] == 'x')
//     {
//         *size = (len - 2) / 8 + 1;
//         u64*res = bi_create(*size);
//         for (unsigned i = 0; i < *size; ++i)
//         {
//             for (unsigned j = 0; j < 8; ++j)
//             {
//                 if (8 * i + j + 2 >= len)
//                     return res;
//                 unsigned val = parse_single(s[8 * i + 2 + j]);
//                 if (val == 17)
//                 {
//                     free(res);
//                     return NULL;
//                 }
//
//                 res[i] |= val << (j * 8);
//             }
//         }
//         return res;
//     }
//     return NULL;
// }
