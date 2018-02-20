
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "lib/base64.h"
#include "bitstring.h"

uint8_t bitcount_table[1<<16];
uint8_t bitcount_table_ready = 0;

void bs_init_bitcount_table() {
	unsigned int a, i;
	uint8_t cnt;
	for(i=0; i<(1<<16); i++) {
		a = i;
		cnt = 0;
		while(a) {
			if (a&1) {
				cnt++;
			}
			a >>= 1;
		}
		bitcount_table[i] = cnt;
	}
	bitcount_table_ready = 1;
}

bitstring_t* bs_alloc(const unsigned int len) {
	bitstring_t *bs = (bitstring_t*) malloc(sizeof(bitstring_t) * len);
	bs[len-1] = 0;
	return bs;
}

void bs_free(bitstring_t *bs) {
	free(bs);
}

void bs_copy(bitstring_t *dst, const bitstring_t *src, unsigned int len) {
	memcpy(dst, src, sizeof(bitstring_t) * len);
}

void bs_init_zeros(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	unsigned int i;
	for (i=0; i<len; i++) {
		bs[i] = 0;
	}
}

void bs_init_ones(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	unsigned int i;
	bitstring_t v;
	for (i=0; i<len; i++) {
		bs[i] = -1;
	}
	/* Clear the remaining bits. */
	v = -1;
	if (bits_remaining > 0) {
		bs[len-1] &= (v << bits_remaining);
	}
}

void bs_calculate_params(unsigned int bits, unsigned int *len, unsigned int *bits_remaining) {
	*len = bits / 8 / sizeof(bitstring_t);
	if ((*len) * 8 * sizeof(bitstring_t) < bits) {
		(*len)++;
	}
	*bits_remaining = (*len) * 8 * sizeof(bitstring_t) - bits;
}

void bs_clear_remaining_bits(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	bitstring_t v = -1;
	if (bits_remaining > 0) {
		bs[len-1] &= (v << bits_remaining);
	}
}

void bs_init_random(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	bitstring_t v;
	arc4random_buf(bs, sizeof(bitstring_t) * len);
	/* Clear the remaining bits. */
	v = -1;
	if (bits_remaining > 0) {
		bs[len-1] &= (v << bits_remaining);
	}
}

void bs_init_hex(bitstring_t *bs, unsigned int len, char *hex) {
	unsigned int i;
	for (i=0; i<len; i++) {
		sscanf(hex, "%016llx", bs);
		bs++;
		hex += 16;
	}
}

void bs_init_b64(bitstring_t *bs, char *b64) {
	Base64decode((char *)bs, b64);
}

/*
We could have used the following code, but it would change the output if the processor is little or big endian. [msbrogli]
```
	const int n = len * sizeof(bitstring_t);
	unsigned char *ptr = (unsigned char *)bs;
	for(i=0; i<n; i++) {
		sprintf(buf, "%02x", *ptr);
		ptr++;
		buf += 2;
	}
```
*/
void bs_to_hex(char *buf, bitstring_t *bs, unsigned int len) {
	unsigned int i;
	for (i=0; i<len; i++) {
		sprintf(buf, "%016llx", *bs);
		bs++;
		buf += 16;
	}
}

void bs_to_b64(char *buf, bitstring_t *bs, unsigned int len) {
	/* TODO Handle little-endian and big-endian. */
	Base64encode(buf, (char *)bs, sizeof(bitstring_t) * len);
}

void bs_not(bitstring_t *bs, const unsigned int len) {
	// FIXME XXX BUG The remaining bits must be cleaned.
	unsigned int i;
	for(i=0; i<len; i++) {
		bs[i] = ~bs[i];
	}
}

void bs_xor(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;
	for(i=0; i<len; i++) {
		bs1[i] ^= bs2[i];
	}
}

void bs_and(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;
	for(i=0; i<len; i++) {
		bs1[i] &= bs2[i];
	}
}

void bs_or(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;
	for(i=0; i<len; i++) {
		bs1[i] |= bs2[i];
	}
}

void bs_average(bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	// FIXME XXX TODO Need to be tested.
	unsigned int i, bit;
	bitstring_t c;
	for(i=0; i<len; i++) {
		c = bs1[i] ^ bs2[i];
		bit = 0;
		while (c) {
			if (c && 1) {
				// Flip a coin! :)
				if (arc4random() % 2) {
					bs1[i] ^= (1<<bit);
				}
			}
			bit++;
			c >>= 1;
		}
	}
}


unsigned int bs_distance(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
#ifdef SDM_USE_BUILTIN_POPCOUNT
	return bs_distance_popcount(bs1, bs2, len);

#elif defined SDM_USE_BITCOUNT_TABLE
	return bs_distance_lookup16(bs1, bs2, len);

#else
	return bs_distance_naive(bs1, bs2, len);

#endif
}

inline unsigned int bs_distance_popcount(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;
	unsigned int dist = 0;
	for(i=0; i<len; i++) {
		dist += __builtin_popcountll(bs1[i] ^ bs2[i]);
	}
	return dist;
}

inline unsigned int bs_distance_lookup16(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;
	uint16_t *ptr;

	bitstring_t a;
	unsigned int dist = 0;
	for(i=0; i<len; i++) {
		a = bs1[i] ^ bs2[i];
		ptr = (uint16_t *)&a;
		dist += bitcount_table[ptr[0]] + bitcount_table[ptr[1]] + bitcount_table[ptr[2]] + bitcount_table[ptr[3]];
	}
	return dist;
}

inline unsigned int bs_distance_naive(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
	unsigned int i;

	bitstring_t a;
	unsigned int dist = 0;
	for(i=0; i<len; i++) {
		a = bs1[i] ^ bs2[i];
		while(a) {
			if (a&1) {
				dist++;
			}
			a >>= 1;
		}
	}
	return dist;
}

unsigned int bs_get_bit(bitstring_t *bs, unsigned int bit) {
	unsigned int offset = bit / 8 / sizeof(bitstring_t);
	unsigned int idx = 8*sizeof(bitstring_t) - 1 - bit % (8*sizeof(bitstring_t));
	return (bs[offset] & ((bitstring_t)1<<idx) ? 1 : 0);
}

void bs_set_bit(bitstring_t *bs, unsigned int bit, unsigned int value) {
	unsigned int offset = bit / 8 / sizeof(bitstring_t);
	unsigned int idx = 8*sizeof(bitstring_t) - 1 - bit % (8*sizeof(bitstring_t));
	if (value == 0) {
		bs[offset] &= ~((bitstring_t)1<<idx);
	} else {
		bs[offset] |= ((bitstring_t)1<<idx);
	}
}

void bs_flip_bit(bitstring_t *bs, unsigned int bit) {
	unsigned int offset = bit / 8 / sizeof(bitstring_t);
	unsigned int idx = 8*sizeof(bitstring_t) - 1 - bit % (8*sizeof(bitstring_t));
	bs[offset] ^= ((bitstring_t)1<<idx);
}

int bs_flip_random_bits(bitstring_t *bs, unsigned int bits, unsigned int flips) {
	unsigned int i, j, tmp, v[bits];
	for(i=0; i<bits; i++) {
		v[i] = i;
	}
	// Fisher-Yates shuffle.
	for(i=bits-1; i>0; i--) {
		j = arc4random_uniform(i+1);
		tmp = v[i];
		v[i] = v[j];
		v[j] = tmp;
	}
	for(i=0; i<flips; i++) {
		bs_flip_bit(bs, v[i]);
	}
	return flips;
}

int bs_flip_random_bits_old(bitstring_t *bs, unsigned int bits, unsigned int flips) {
	// This algorithm is not efficient when `flips` is close to `bits`.
	// We recommend to use `bs_flip_random_bits`.
	unsigned int i, idx;
	unsigned int cnt = 0;
	uint8_t v[bits];
	memset(v, 0, sizeof(v));
	do {
		for(i=0; i<flips - cnt; i++) {
			idx = arc4random_uniform(bits);
			v[idx] = 1;
		}
		cnt = 0;
		for(i=0; i<bits; i++) {
			cnt += v[i];
		}
	} while(cnt < flips);
	cnt = 0;
	for(i=0; i<bits; i++) {
		if (v[i] == 1) {
			bs_flip_bit(bs, i);
			cnt++;
		}
	}
	return cnt;
}

