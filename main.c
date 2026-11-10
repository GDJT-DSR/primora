#include "bi.h"
#include "rand.h"
#include "test.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Options
{
    enum Mode
    {
        GENERATE,
        VERIFY
    } mode;
    const char *position;
    rand_t seed;
    unsigned time;
    const char *out;
} Options;

const char *usage =
    "用法：%s <命令> [选项]\n\n"
    "命令：\n"
    "\tgen <hex-digits>\t生成指定16进制位数的素数\n"
    "\tverify <num>\t\t判断传入整数是不是素数，0x开头表示16进制\n\t\t\t\t留空或显式传入'-'表示从控制台读入\n\n"
    // ，以0x开头，输入16进制，
    "选项：\n"
    "\t-o\t\t\t只在gen命令中可用，为素数的输出路径\n\t\t\t\t默认为prime.txt，无参数则不输出到文件\n"
    "\t-t\t\t\t表示Miller-Rabbin算法的次数\n"
    "\t-s\t\t\t表示随机数种子，默认为time(NULL)\n\n"
    "例子：\n"
    "\t%s verify 0x1f\n"
    "\t%s gen 1024 -oout.txt\n";

void parse_fail(const char *reason, const char *name)
{
    puts(reason);
    printf(usage, name, name, name);
    exit(1);
}

Options parse_args(int argc, char *argv[])

{
    Options res;
    res.position = NULL;
    res.out = "prime.txt";
    res.seed = (rand_t)time(0);
    res.time = 10;

    if (argc <= 1)
    {
        parse_fail("该程序需要接收命令\n", argv[0]);
    }
    if (str_eq(argv[1], "gen"))
    {
        res.mode = GENERATE;
    }
    else if (str_eq(argv[1], "verify"))
    {
        res.mode = VERIFY;
    }
    else
    {
        parse_fail("不支持的命令\n", argv[0]);
    }

    for (int i = 2; i < argc; ++i)
    {
        const char *key = argv[i];
        if (*key != '-')
        {
            if (res.position)
            {
                parse_fail("参数数量过多\n", argv[0]);
            }
            res.position = key;
            continue;
        }
        const char *val = NULL;
        if (key[2])
        {
            val = key + 2;
        }
        else if (i != argc - 1 && *argv[i + 1] != '-')
        {
            val = argv[i + 1];
            ++i;
        }
        switch (key[1])
        {
        case 't':
        {

            if (!val)
            {
                parse_fail("-t选项需要参数\n", argv[0]);
            }
            unsigned time;
            if (sscanf(val, "%u", &time) <= 0)
            {
                parse_fail("-t选项参数解析失败\n", argv[0]);
            }
            res.time = time;
            break;
        }
        case 's':
        {
            if (!val)
            {
                parse_fail("-s选项需要参数\n", argv[0]);
            }
            rand_t seed;
            if (sscanf(val, "%llu", &seed) <= 0)
            {
                parse_fail("-s选项参数解析失败\n", argv[0]);
            }
            res.seed = seed;
            break;
        }
        case 'o':
            if (res.mode != GENERATE)
            {
                parse_fail("无效的参数：-o\n", argv[0]);
            }
            res.out = val;
            break;
        default:
            printf("无效的参数：-%c", key[1]);
            parse_fail("\n", argv[0]);
        }
    }

    return res;
}

int main(int argc, char *argv[])
{

    Options opt = parse_args(argc, argv);
    my_srand(opt.seed);
    if (opt.mode == GENERATE)
    {
        if (!opt.position)
        {
            parse_fail("需要位数参数\n", argv[0]);
            return 1;
        }
        unsigned digs = 0;
        sscanf(opt.position, "%u", &digs);
        if (digs < 2 || digs > 4096)
        {
            printf("不支持的长度: %u，需要2~4096", digs);
            parse_fail("\n", argv[0]);
            return 1;
        }
        char *val = gen(digs, opt.time);
        puts("\n\nResult:");
        puts(val);
        if (opt.out)
        {
            FILE *fp = fopen(opt.out, "w");
            if (!fp)
            {
                perror("fopen");
                return 3;
            }
            fputs(val, fp);
            fputc('\n', fp);

            fclose(fp);
        }
        free(val);
        return 0;
    }
    if (opt.mode == VERIFY)
    {
        item_t *val = NULL;
        unsigned size = 0;
        if (opt.position && !str_eq(opt.position, "-"))
            val = bi_from_string(opt.position, &size);
        else
        {
            char buf[32768];
            if (!fgets(buf, sizeof(buf), stdin))
            {
                printf("读取失败\n");
                return 1;
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
            return 1;
        }
        int res = test(val, size, opt.time);
        switch (res)
        {
        case 0:
            puts("合数");
            break;
        case 1:
            puts("可能是素数");
            break;
        case 2:
            puts("素数");
            break;
        }
        free(val);
        return 0;
    }

    return 0;
}
