
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/base64.h"
#include "bitstring.h"

#ifdef SDM_USE_BITCOUNT_TABLE
uint8_t bitcount_table[1<<16];
uint8_t bitcount_table_ready = 0;

void bs_init_bitcount_table() {
	unsigned int a;
	uint8_t cnt;
	for(unsigned int i=0; i<(1<<16); i++) {
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
#endif

bitstring_t* bs_alloc(const unsigned int len) {
	return (bitstring_t*) malloc(sizeof(bitstring_t) * len);
}

void bs_free(bitstring_t *bs) {
	free(bs);
}

void bs_copy(bitstring_t *dst, const bitstring_t *src, unsigned int len) {
	memcpy(dst, src, sizeof(bitstring_t) * len);
}

void bs_init_ones(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	for (int i=0; i<len; i++) {
		bs[i] = -1;
	}
	// Clear the remaining bits.
	bitstring_t v = -1;
	if (bits_remaining > 0) {
		bs[len-1] &= (v << bits_remaining);
	}
}

void bs_init_random(bitstring_t *bs, unsigned int len, unsigned int bits_remaining) {
	arc4random_buf(bs, sizeof(bitstring_t) * len);
	// Clear the remaining bits.
	bitstring_t v = -1;
	if (bits_remaining > 0) {
		bs[len-1] &= (v << bits_remaining);
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
	int i;
	for (i=0; i<len; i++) {
		sprintf(buf, "%016llx", *bs);
		bs++;
		buf += 16;
	}
}

void bs_to_b64(char *buf, bitstring_t *bs, unsigned int len) {
	// TODO Handle little-endian and big-endian.
	Base64encode(buf, (char *)bs, sizeof(bitstring_t) * len);
}

int bs_distance(const bitstring_t *bs1, const bitstring_t *bs2, const unsigned int len) {
#ifdef SDM_USE_BITCOUNT_TABLE
	uint16_t *ptr;
#endif

	bitstring_t a;
	unsigned int dist = 0;
	for(int i=0; i<len; i++) {
		a = bs1[i] ^ bs2[i];
#ifdef SDM_USE_BITCOUNT_TABLE
		ptr = (uint16_t *)&a;
		dist += bitcount_table[ptr[0]] + bitcount_table[ptr[1]] + bitcount_table[ptr[2]] + bitcount_table[ptr[3]];
#else
		while(a) {
			if (a&1) {
				dist++;
			}
			a >>= 1;
		}
#endif
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
	bs[offset] ^= ~((bitstring_t)1<<idx);
}

void bs_flip_random_bits(bitstring_t *bs, unsigned int bits, unsigned int flips) {
	int i, idx;
	int cnt = 0;
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
	for(i=0; i<bits; i++) {
		if (v[i] == 1) {
			bs_flip_bit(bs, i);
		}
	}
}

