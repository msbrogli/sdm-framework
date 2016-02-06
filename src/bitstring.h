
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>

typedef uint64_t bitstring_t;

void bs_init_bitcount_table();

bitstring_t* bs_alloc(const unsigned int len);
void bs_free(bitstring_t *bs);

void bs_init_ones(bitstring_t *bs, unsigned int len, unsigned int bits_remaining);
void bs_init_random(bitstring_t *bs, unsigned int len, unsigned int bits_remaining);
void bs_to_hex(char *buf, bitstring_t *bs, unsigned int len);
void bs_to_b64(char *buf, bitstring_t *bs, unsigned int len);
int bs_distance(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);

unsigned int bs_get_bit(bitstring_t *this, unsigned int bit);

#endif
