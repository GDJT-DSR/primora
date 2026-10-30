#include "bi.h"
#include "mt19937.h"
#include "utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

mt19937_t bi_rand;
item_t *bi_clone(const item_t *origin, unsigned size)
{
    item_t *tmp = malloc(sizeof(item_t) * size);
    bi_copy(tmp, origin, size);
    return tmp;
}

item_t *bi_create(unsigned size)
{
    item_t *tmp = malloc(sizeof(item_t) * size);
    for (unsigned i = 0; i < size; ++i)
    {
        tmp[i] = 0;
    }
    return tmp;
}
void bi_copy(item_t *dest, const item_t *src, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        dest[i] = src[i];
    }
}
void bi_random(item_t *val, bi_size size)
{
    static mt19937_t *m = &bi_rand;
    for (unsigned i = 0; i < size.limb; ++i)
    {
        // val[i] =
        // (unsigned)((rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24));
        val[i] = mt19937_gen(m);
#if ITEM_SIZE == 64
        val[i] |= (item_t)mt19937_gen(m) << 32;
#endif
    }
    if (size.bits)
    {
        // val[size.limb] = mt19937_gen(m) & ((1u << size.bits) - 1);
        val[size.limb] = mt19937_gen(m);
#if ITEM_SIZE == 64
        val[size.limb] |= (item_t)mt19937_gen(m) << 32;
#endif
        val[size.limb] &= ((item_t)1 << size.bits) - 1;
    }
    else
    {
        val[size.limb] = 0;
    }
}
bi_size bi_get_size(const item_t *val, unsigned size)
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
    item_t li = val[res.limb];
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
void bi_zero(item_t *v, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        v[i] = 0;
    }
}
void bi_print_hex(const item_t *val, bi_size size)
{
    static char bitset[] = "0123456789abcdef";
    printf("0x");
    unsigned time = (size.bits + 3) / 4;
    char *buf = malloc(size.limb * (ITEM_SIZE / 4) + time + 1);
    unsigned i = 0;
    do
    {
        while (time-- > 0)
        {
            // putchar(bitset[(val[size.limb] >> (4 * time)) & 0xf]);
            buf[i++] = bitset[(val[size.limb] >> (4 * time)) & 0xf];
        }
        time = (ITEM_SIZE / 4);
    } while (size.limb-- > 0);
    buf[i] = '\0';
    puts(buf);
    free(buf);
}
void bi_comp(item_t *val, unsigned size)
{
    // 按位取反再加1
    for (unsigned i = 0; i < size; ++i)
        val[i] = ~val[i];
    bi_add_small(val, 1, size);
}

int bi_add(item_t *l, const item_t *r, unsigned s)
{
    item_t car = 0;
    for (unsigned i = 0; i < s; ++i)
    {
        item_t sum = l[i] + r[i];
        int b1 = sum < l[i];
        item_t sum2 = sum + car;
        int b2 = sum2 < sum;
        l[i] = sum2;
        car = b1 || b2;
    }
    return (int)car;
}
int bi_add_small(item_t *l, item_t r, unsigned s)
{
    item_t car = r;
    for (unsigned i = 0; i < s; ++i)
    {
        l[i] += car;
        car = l[i] < car;
    }
    return (int)car;
}

void bi_square(const item_t *a, unsigned sa, item_t *r, unsigned sr)
{
    item_t car = 0;
    // 计算交叉项
    for (unsigned i = 0; i < sa; ++i)
    {
        item_t ai = a[i];
        for (unsigned int j = i + 1; j < sa; ++j)
        {
            ditem_t acc = (ditem_t)ai * a[j] + r[i + j] + car;
            r[i + j] = (item_t)acc;
            car = acc >> ITEM_SIZE;
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
        item_t tmp = r[i];
        r[i] = (tmp << 1) | car;
        car = tmp >> (ITEM_SIZE - 1);
    }

    // 处理平方项
    for (unsigned i = 0; i < sa; ++i)
    {
        ditem_t acc = (ditem_t)a[i] * a[i] + r[2 * i];
        r[2 * i] = (item_t)acc;
        car = acc >> ITEM_SIZE;

        for (unsigned k = 2 * i + 1; car && k < sr; ++k)
        {
            r[k] += car;
            car = r[k] < car;
        }
    }
}
int bi_cmp(const item_t *l, const item_t *r, unsigned s)
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
void bi_rshift(item_t *val, unsigned shift, unsigned size)
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

unsigned bi_remainder(item_t *val, unsigned target, unsigned size)
{
    if (target == 0)
        return 0;

    unsigned long long rem = 0;

    for (unsigned i = size; i-- > 0;)
    {
#if ITEM_SIZE == 64
        // 按32位做除法
        item_t v = val[i];
        rem = ((rem << 32) | (v >> 32)) % target;
        rem = ((rem << 32) | (item_t)(unsigned)v) % target;
#else
        rem = ((rem << 32) | val[i]) % target;
#endif
    }

    return (unsigned)rem;
}
// 末尾0的个数
bi_size bi_end0(const item_t *val, unsigned size)
{
    unsigned i = 0;
    for (; i < size && !val[i]; ++i)
        ;
    if (i == size)
        return (bi_size){.limb = size, .bits = 0};

    item_t bit = val[i]; // != 0
    unsigned count = 0;
    for (; !(bit & 1u); ++count)
        bit >>= 1;
    return (bi_size){.limb = i, .bits = count};
}
void bi_double_mod(item_t *val, const item_t *mod, const item_t *nmod, unsigned size)
{
    item_t car = 0;

    for (unsigned i = 0; i < size; ++i)
    {
        item_t tmp = val[i];
        val[i] = (tmp << 1) | car;
        car = tmp >> (ITEM_SIZE - 1);
    }

    if (car || bi_cmp(val, mod, size) >= 0)
        bi_add(val, nmod, size);
}
int bi_sub(item_t *l, const item_t *r, unsigned size)
{
    item_t borrow = 0;
    for (unsigned i = 0; i < size; ++i)
    {
        item_t li = l[i];
        item_t ri = r[i];

        item_t t1 = li - ri;
        int b1 = li < ri;
        item_t t2 = t1 - borrow;
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
item_t *bi_from_string(const char *s, unsigned *size)
{
    unsigned len = str_len(s);
    // 1. 安全检查长度，并支持 0X 前缀
    if (len < 3 || s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
    {
        return NULL;
    }

    unsigned hex_len = len - 2;
    *size = (hex_len + (ITEM_SIZE - 1) / 4) / (ITEM_SIZE / 4);
    item_t *res = bi_create(*size);

    for (unsigned i = 0; i < hex_len; ++i)
    {
        // 从字符串右侧向左读，i=0 对应最低位的字符
        char c = s[len - 1 - i];
        item_t val = parse_single(c);

        if (val == 17)
        {
            free(res);
            return NULL;
        }

        unsigned index = i / (ITEM_SIZE / 4);       // limb
        unsigned shift = (i % (ITEM_SIZE / 4)) * 4; // bit

        res[index] |= (val << shift);
    }

    return res;
}
