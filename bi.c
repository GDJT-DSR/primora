#include "bi.h"
#include "utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

mt19937_t bi_rand;
unsigned *bi_clone(const unsigned *origin, unsigned size)
{
    unsigned *tmp = malloc(sizeof(unsigned) * size);
    bi_copy(tmp, origin, size);
    return tmp;
}

unsigned *bi_create(unsigned size)
{
    unsigned *tmp = malloc(sizeof(unsigned) * size);
    for (unsigned i = 0; i < size; ++i)
    {
        tmp[i] = 0;
    }
    return tmp;
}
void bi_copy(unsigned *dest, const unsigned *src, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        dest[i] = src[i];
    }
}
void bi_random(unsigned *val, bi_size size)
{
    static mt19937_t *m = &bi_rand;
    for (unsigned i = 0; i < size.limb; ++i)
    {
        // val[i] =
        // (unsigned)((rand() & 0xff) | ((rand() & 0xff) << 8) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 24));
        val[i] = mt19937_gen(m);
    }
    if (size.bits)
        val[size.limb] = mt19937_gen(m) & ((1u << size.bits) - 1);
}
bi_size bi_get_size(const unsigned *val, unsigned size)
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
    unsigned li = val[res.limb];
    if (li >> 31)
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
void bi_zero(unsigned *v, unsigned s)
{
    for (unsigned i = 0; i < s; ++i)
    {
        v[i] = 0;
    }
}
void bi_print_hex(const unsigned *val, bi_size size)
{
    static char bitset[] = "0123456789abcdef";
    printf("0x");
    unsigned time = (size.bits + 3) / 4;
    char buf[8 * size.limb + 9];
    unsigned i = 0;
    do
    {
        while (time-- > 0)
        {
            // putchar(bitset[(val[size.limb] >> (4 * time)) & 0xf]);
            buf[i++] = bitset[(val[size.limb] >> (4 * time)) & 0xf];
        }
        time = 8;
    } while (size.limb-- > 0);
    buf[i] = '\0';
    puts(buf);
}
void bi_comp(unsigned *val, unsigned size)
{
    // 按位取反再加1

    for (unsigned i = 0; i < size; ++i)
        val[i] = ~val[i];
    unsigned *one = bi_create(size);
    one[0] = 1;
    bi_add(val, one, size);
    free(one);
}

int bi_add(unsigned *l, const unsigned *r, unsigned s)
{
    unsigned long long car = 0;
    for (unsigned i = 0; i < s; ++i)
    {
        car += (unsigned long long)l[i] + r[i];
        l[i] = (unsigned)car;
        car >>= ITEM_SIZE;
    }
    return (int)car;
}
int bi_add_small(unsigned *l, unsigned r, unsigned s)
{
    unsigned long long car = r;
    for (unsigned i = 0; i < s && car; ++i)
    {
        car += l[i];
        l[i] = (unsigned)car;
        car >>= ITEM_SIZE;
    }
    return (int)car;
}

void bi_mul(const unsigned *l, unsigned ls, const unsigned *r, unsigned rs, unsigned *t, unsigned os)
{
    typedef unsigned long long uli;
    for (unsigned i = 0; i < ls; ++i)
    {
        // 进位
        uli car = 0;
        for (unsigned j = 0; j < rs; ++j)
        {
            car += 1ull * l[i] * r[j] + t[i + j];
            t[i + j] = (unsigned)car;
            car >>= ITEM_SIZE;
        }
        unsigned k = i + rs;
        for (; car && k < os; ++k)
        {
            car += t[k];
            t[k] = (unsigned)car;
            car >>= ITEM_SIZE;
        }
    }
}
void bi_square(const unsigned *a, unsigned sa, unsigned *r, unsigned sr)
{
    typedef unsigned long long uli;
    uli car = 0;
    // 计算交叉项
    for (unsigned i = 0; i < sa; ++i)
    {
        uli ai = a[i];
        for (unsigned int j = i + 1; j < sa; ++j)
        {
            car += ai * a[j] + r[i + j];
            r[i + j] = (unsigned)car;
            car >>= ITEM_SIZE;
        }

        unsigned k = i + sa;
        for (; car && k < sr; ++k)
        {
            car += r[k];
            r[k] = (unsigned)car;
            car >>= ITEM_SIZE;
        }
    }

    // 给交叉项乘2
    car = 0;
    for (unsigned i = 0; i < sr; ++i)
    {
        car += 2ull * r[i];
        r[i] = (unsigned)car;
        car >>= ITEM_SIZE;
    }

    // 处理平方项
    for (unsigned i = 0; i < sa; ++i)
    {
        car = (uli)a[i] * a[i] + r[2 * i];
        r[2 * i] = (unsigned)car;
        car >>= ITEM_SIZE;
        for (unsigned k = 2 * i + 1; car && k < sr; ++k)
        {
            car += r[k];
            r[k] = (unsigned)car;
            car >>= ITEM_SIZE;
        }
    }
}
int bi_cmp(const unsigned *l, const unsigned *r, unsigned s)
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
void bi_rshift(unsigned *val, unsigned shift, unsigned size)
{
    // a[i] = a[i + l] >> b | a[i + l + 1] << (32-b);
    if (shift >= size)
    {
        bi_zero(val, size);
        return;
    }
    unsigned n = size - shift;
    // if (shift.bits == 0)
    // {
    for (unsigned i = 0; i < n; ++i)
        val[i] = val[i + shift];
    for (unsigned i = size - shift; i < size; ++i)
        val[i] = 0;
    // }
    // else
    // {
    //     for (unsigned i = 0; i < (n - 1); ++i)
    //     {
    //         val[i] = (val[i + shift.limb] >> shift.bits) | (val[i + shift.limb + 1] << (32 - shift.bits));
    //     }
    //     // 单独处理i+l-1
    //     val[n - 1] = val[size - 1] >> shift.bits;
    //     for (unsigned i = n; i < size; ++i)
    //     {
    //         val[i] = 0;
    //     }
    // }
}

unsigned bi_div(unsigned *val, unsigned target, unsigned size)
{
    typedef unsigned long long ulg;
    unsigned long long cur = 0;

    while (size > 0)
    {
        --size;
        cur += val[size];
        ulg k = cur / target;
        ulg r = cur - target * k;
        val[size] = (unsigned)k;
        cur = r << ITEM_SIZE;
    }
    return (unsigned)cur;
}
unsigned bi_remainder(unsigned *val, unsigned target, unsigned size)
{
    if (target == 0)
        return 0;

    unsigned long long rem = 0;

    for (unsigned i = size; i-- > 0;)
    {
        unsigned long long cur = (rem << ITEM_SIZE) | val[i];
        rem = cur % target;
    }
    // printf("%d, %llx\n", target, rem);

    return (unsigned)rem;
}
unsigned bi_bit(const unsigned *v, unsigned b)
{
    unsigned u = b / ITEM_SIZE;
    unsigned r = b - u * ITEM_SIZE;

    return !!(v[u] & (1u << r));
}
// 末尾0的个数
bi_size bi_end0(const unsigned *val, unsigned size)
{
    unsigned i = 0;
    for (; i < size && !val[i]; ++i)
        ;
    if (i == size)
        return (bi_size){.limb = size, .bits = 0};

    unsigned bit = val[i]; // != 0
    unsigned count = 0;
    for (; !(bit & 1u); ++count)
        bit >>= 1;
    return (bi_size){.limb = i, .bits = count};
}
void bi_double_mod(unsigned *val, const unsigned *mod, const unsigned *nmod, unsigned size)
{
    unsigned long long carry = 0;

    for (unsigned i = 0; i < size; ++i)
    {
        carry += ((unsigned long long)val[i] << 1);
        val[i] = (unsigned)carry;
        carry >>= ITEM_SIZE;
    }

    if (carry || bi_cmp(val, mod, size) >= 0)
        bi_add(val, nmod, size);
}
int bi_sub(unsigned *l, const unsigned *r, unsigned size)
{
    int borrow = 0;
    for (unsigned i = 0; i < size; ++i)
    {
        // borrow = l[i] < r[i];
        // l[i] -= r[i];
        unsigned long long li = l[i];
        unsigned long long ri = r[i] + (borrow ? 1ull : 0ull);
        borrow = li < ri;
        l[i] = (unsigned)(li - ri);
    }

    return borrow;
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
unsigned *bi_from_string(const char *s, unsigned *size)
{
    unsigned len = str_len(s);
    // 1. 安全检查长度，并支持 0X 前缀
    if (len < 3 || s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
    {
        return NULL;
    }

    unsigned hex_len = len - 2;
    *size = (hex_len + 7) / 8;
    unsigned *res = bi_create(*size);

    for (unsigned i = 0; i < hex_len; ++i)
    {
        // 从字符串右侧向左读，i=0 对应最低位的字符
        char c = s[len - 1 - i];
        unsigned val = parse_single(c);

        if (val == 17)
        {
            free(res);
            return NULL;
        }

        unsigned word_index = i / 8;       // limb
        unsigned byte_shift = (i % 8) * 4; // bit

        res[word_index] |= (val << byte_shift);
    }

    return res;
}
// unsigned *bi_from_string(const char *s, unsigned *size)
// {
//     unsigned len = str_len(s);
//     if (s[0] == '0' && s[1] == 'x')
//     {
//         *size = (len - 2) / 8 + 1;
//         unsigned *res = bi_create(*size);
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
