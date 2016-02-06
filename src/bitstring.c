
#include <stdlib.h>
#include <stdio.h>
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
	int i, j;
	bitstring_t a;
	uint8_t a_hi;
	for (i=0; i<len; i++) {
		a = bs[i];
		for (j=0; j<sizeof(bitstring_t); j++) {
			// Get the highest byte.
			a_hi = a>>(sizeof(bitstring_t)*8-8);
			Base64encode(buf, (const char*)&a_hi, 1);
			buf++;
			a <<= 8;
		}
	}
	*buf = '\0';
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

/*
void bs_clear_surplus(struct bitstring_s *this) {
	int last = this->bits % 64;
	if (last > 0) {
		this->data[this->len-1] &= ~((uint64_t)0xFFFFFFFFFFFFFFFF << last);
	}
}

void bs_init_from_base64(struct bitstring_s *this, unsigned int bits, const char *b64) {
	bs_init(this, bits);

	const size_t size = sizeof(uint64_t) * this->len;
	//std::string buffer = base64_decode(b64);
	bs_clear_surplus(this);
}


void bs_init_from_bs(struct bitstring_s *this, const struct bitstring_s *other) {
	bs_init(this, other->bits);
	memcpy(this->data, other->data, sizeof(uint64_t) * this->len);
	bs_clear_surplus(this);
}

unsigned int bs_get_bit(struct bitstring_s *this, unsigned int bit) const {
	assert(bit <= this->bits);
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	return (data[offset] & ((uint64_t)1<<idx) ? 1 : 0);
}

unsigned int bs_set_bit(struct bitstring_s *this, unsigned int bit, unsigned int value) {
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	if (value == 0) {
		data[offset] &= ~((uint64_t)1<<idx);
	} else {
		data[offset] |= ((uint64_t)1<<idx);
	}
}

int bs_str(struct bitstring_s *this, char *str) const {
	for(int i=0; i<this->bits; i++) {
		str[i] = bs_get_bit(this, i) ? '1' : '0';
	}
	str[this->bits] = '\0';
	return std::string(str);
}
*/

/*
std::string Bitstring::base64() const {
	// FIXME It cannot depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
	return base64_encode((const unsigned char *)this->data, sizeof(uint64_t)*this->len);
}

bool Bitstring::operator<(const Bitstring &bs) const {
	assert(this->bits == bs.bits);

	for(int i=0; i<this->len; i++) {
		if (this->data[i] < bs.data[i]) {
			return true;
		} else if (this->data[i] > bs.data[i]) {
			return false;
		}
	}
	return false;
}
*/
