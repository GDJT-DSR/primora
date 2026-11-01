#ifndef MT19937_H_
#define MT19937_H_
typedef unsigned long long rand_t;

typedef char rand_t_len_is_64_bit[sizeof(rand_t) == 8 ? 1 : -1];

void romu_srand(rand_t);
rand_t romu_gen(void);

#endif
