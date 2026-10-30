#include "test.h"
#include "bi.h"
#include "consts.h"
#include "mont.h"
#include <stdio.h>
#include <stdlib.h>

unsigned test_time = 10;

int test(item_t *val, unsigned size)
{
    bi_size size_detail = bi_get_size(val, size);
    if (size_detail.limb == 0)
    {

        if (size_detail.limb == 0 && size_detail.bits == 0)
            return 0;
        if (val[0] == 1)
            return 0;
        if (val[0] == 2)
            return 1;
    }

    if (!(val[0] & 1u))
        return 0;
    // 小素数试除
    if (size_detail.limb == 0)
        for (unsigned i = 0; i < PRIMES_LEN; ++i)
            if (val[0] == primes[i])
                return 1;

    for (unsigned i = 0; i < PRIMES_LEN; ++i)
        if (bi_remainder(val, primes[i], size) == 0)
            return 0;

    mont_init(val, size);
    item_t *tmp = bi_clone(val, size);

    tmp[0] ^= 1u;
    bi_size end0_size = bi_end0(tmp, size);
    unsigned count = end0_size.bits + end0_size.limb * ITEM_SIZE;
    // bi_rshift(tmp, end0_size, size);

    item_t *ar = bi_create(2 * mont_size + 1);
    item_t *a = bi_create(2 * mont_size + 1);
    for (unsigned t = 0; t < test_time; ++t)
    {
        do
        {
            bi_random(ar, (bi_size){.limb = mont_size, .bits = 0});
            bi_copy(a, ar, mont_size);
            mont_redc(a);
        } while (bi_cmp(a, tmp, mont_size) >= 0 || a[0] < 2);
        printf("test%u: a=", t);
        bi_print_hex(a, (bi_size){.limb = mont_size, .bits = 0});
        putchar('\n');
        mont_pow(ar, tmp, size, count);
        if (!bi_cmp(ar, mont_one, mont_size) || !bi_cmp(ar, mont_ione, mont_size))
            goto end_for;
        for (unsigned i = 1; i < count; ++i)
        {
            mont_square(ar);
            if (!bi_cmp(ar, mont_ione, mont_size))
            {
                goto end_for;
            }
            else if (!bi_cmp(ar, mont_one, mont_size))
            {
                goto end_no; // 写或不写那个可能更快一点？
            }
        }
        goto end_no;

    end_for:;
    }
    free(ar);
    free(tmp);
    free(a);
    mont_clean();
    return 1;
end_no:
    free(ar);
    free(tmp);
    free(a);
    mont_clean();
    return 0;
}

item_t *gen(unsigned digs)
{
    bi_size size = (bi_size){.limb = digs / (ITEM_SIZE / 4), .bits = 4 * (digs % (ITEM_SIZE / 4))};
    unsigned limbs = size.limb + 1;
    item_t *result = bi_create(limbs);
    for (;;)
    {
        do
            bi_random(result, size);
        while (!((result[size.limb - (size.bits == 0)] >> (size.bits ? size.bits - 4 : ITEM_SIZE - 4)) & 0xf));
        printf("try: ");
        bi_print_hex(result, size);
        printf("\n");
        if (test(result, limbs))
            return result;

        printf("failed.\n\n");
    }
}
