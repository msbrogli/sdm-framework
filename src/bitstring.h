
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>

typedef uint64_t bitstring_t;

void bs_init_bitcount_table(void);

bitstring_t* bs_alloc(const unsigned int len);
void bs_free(bitstring_t *bs);

void bs_init_zeros(bitstring_t *bs, unsigned int len, unsigned int bits_remaining);
void bs_init_ones(bitstring_t *bs, unsigned int len, unsigned int bits_remaining);
void bs_init_random(bitstring_t *bs, unsigned int len, unsigned int bits_remaining);
void bs_init_b64(bitstring_t *bs, char *b64);
void bs_copy(bitstring_t *dst, const bitstring_t *src, unsigned int len);
void bs_to_hex(char *buf, bitstring_t *bs, unsigned int len);
void bs_to_b64(char *buf, bitstring_t *bs, unsigned int len);
inline unsigned int bs_distance_popcount(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
inline unsigned int bs_distance_lookup16(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
inline unsigned int bs_distance_naive(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
int unsigned bs_distance(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);

unsigned int bs_get_bit(bitstring_t *this, unsigned int bit);
void bs_set_bit(bitstring_t *bs, unsigned int bit, unsigned int value);
void bs_flip_bit(bitstring_t *bs, unsigned int bit);

int bs_flip_random_bits(bitstring_t *bs, unsigned int bits, unsigned int flips);

void bs_xor(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
void bs_and(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
void bs_or(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);
void bs_average(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len);

#endif
