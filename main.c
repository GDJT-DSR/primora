#include "bi.h"
#include "test.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{

    if (argc <= 1)
    {
        printf("需要参数\n");
        return 2;
    }
    if (str_eq(argv[1], "gen"))
    {
        unsigned digs = 0;
        if (argc <= 2)
        {
            printf("需要位数.\n");
            return 2;
        }
        sscanf(argv[2], "%u", &digs);
        if (digs <= 2 || digs > 4096)
        {
            printf("不支持的长度: %u，需要2~4096", digs);
            return 2;
        }
        unsigned seed = (unsigned)time(NULL);
        if (argc >= 4 && sscanf(argv[3], "%u", &seed) != 1)
        {
            printf("seed参数错误\n");
        }
        mt19937_init(&bi_rand, seed);
        item_t *val = gen(digs);
        printf("\n\nResult:\n");
        bi_print_hex(val, (bi_size){.limb = digs / (ITEM_SIZE / 4), .bits = 4 * (digs % (ITEM_SIZE / 4))});
        printf("\n");
        free(val);
        return 0;
    }
    if (str_eq(argv[1], "verify"))
    {
        mt19937_init(&bi_rand, (unsigned)time(NULL));
        item_t *val = NULL;
        unsigned size = 0;
        if (argc >= 3)
            val = bi_from_string(argv[2], &size);
        else
        {
            char buf[32768];
            if (!fgets(buf, sizeof(buf), stdin))
            {
                printf("读取失败\n");
                return 2;
            }
            for (char *p = buf; *p; ++p)
            {
                if (*p == '\n' || *p == ' ' || *p == '\t' || *p == '\r')
                {
                    *p = '\0';
                    break;
                }
            }
            val = bi_from_string(buf, &size);
        }
        if (!val)
        {
            printf("参数解析错误\n");
            return 2;
        }
        int res = test(val, size);
        printf(res ? "可能是素数\n" : "合数\n");
        free(val);
        return !res;
    }

    return 0;
}
