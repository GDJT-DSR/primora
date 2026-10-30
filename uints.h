#ifndef UINTS_H_
#define UINTS_H_
typedef unsigned long long u64;
typedef unsigned u32;
// 验证大小正确
typedef char limb_is_64[sizeof(u64) == 8 ? 1 : -1];
typedef char half_is_32[sizeof(u32) == 4 ? 1 : -1];

// int128
#if defined(__SIZEOF_INT128__) && !defined(NO_U128)

typedef unsigned __int128 u128;

#define u128_self_add(a, b) (a) += (b)
static inline u128 u128_mul(u64 a, u64 b) { return (u128)a * b; }

static inline u64 u128_low(u128 val) { return (u64)val; }
static inline u64 u128_high(u128 val) { return (u64)(val >> 64); }

#else

typedef struct u128
{
    u64 low;
    u64 high;
} u128;

#define u128_self_add(a, b)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        a.low += b;                                                                                                    \
        a.high += a.low < b;                                                                                           \
    } while (0)

static inline u128 u128_mul(u64 a, u64 b)
{
    u64 a0 = (unsigned)a;
    u64 a1 = a >> 32;
    u64 b0 = (unsigned)b;
    u64 b1 = b >> 32;
    u64 p00 = a0 * b0;
    u64 p10 = a1 * b0;
    u64 p01 = a0 * b1;
    u64 p11 = a1 * b1;
    u128 r;

    u64 tmp = (p00 >> 32) + (u64)(u32)p01 + (u64)(u32)p10;
    r.low = (u64)(u32)p00 | (tmp << 32);
    r.high = p11 + (p10 >> 32) + (p01 >> 32) + (tmp >> 32);

    return r;
}

static inline u64 u128_low(u128 val) { return val.low; }
static inline u64 u128_high(u128 val) { return val.high; }

#endif
#endif
