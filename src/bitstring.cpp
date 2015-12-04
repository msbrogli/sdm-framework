
#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include "lib/base64.h"
#include "bitstring.h"

#ifdef SDM_USE_BITCOUNT_TABLE
uint8_t bitcount_table[1<<16];
bool bitcount_table_ready = false;

void init_bitcount_table() {
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
}
#endif

void Bitstring::_init(unsigned int bits) {
	this->bits = bits;

	this->len = bits/64;
	if (bits % 64 > 0) {
		this->len++;
	}

	this->data = (uint64_t *) malloc(sizeof(uint64_t) * this->len);

#ifdef SDM_USE_BITCOUNT_TABLE
	if (bitcount_table_ready == false) {
		init_bitcount_table();
		bitcount_table_ready = true;
	}
#endif
}

void Bitstring::clear_surplus() {
	int last = this->bits % 64;
	if (last > 0) {
		this->data[this->len-1] &= ~((uint64_t)0xFFFFFFFFFFFFFFFF << last);
	}
}

Bitstring::Bitstring(unsigned int bits) {
	this->_init(bits);
	arc4random_buf(this->data, sizeof(uint64_t) * this->len);
	this->clear_surplus();
}

Bitstring::Bitstring(unsigned int bits, std::string const &b64) {
	const size_t size = sizeof(uint64_t) * this->len;
	std::string buffer = base64_decode(b64);
	assert(size == buffer.length());

	this->_init(bits);
	memcpy(this->data, buffer.c_str(), size);
	this->clear_surplus();
}

Bitstring::Bitstring(Bitstring const &bs) {
	this->_init(bs.bits);
	memcpy(this->data, bs.data, sizeof(uint64_t) * this->len);
	this->clear_surplus();
}

Bitstring::~Bitstring() {
	free(this->data);
}

unsigned int Bitstring::get(unsigned int bit) const {
	assert(bit <= this->bits);
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	return (data[offset] & ((uint64_t)1<<idx) ? 1 : 0);
}

void Bitstring::set(unsigned int bit, unsigned int value) {
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	if (value == 0) {
		data[offset] &= ~((uint64_t)1<<idx);
	} else {
		data[offset] |= ((uint64_t)1<<idx);
	}
}

int Bitstring::distance(const Bitstring *bs) const {
	if (this->bits != bs->bits) {
		return -1;
	}

#ifdef SDM_USE_BITCOUNT_TABLE
	uint16_t *ptr;
#endif

	uint64_t a;
	unsigned int dist = 0;
	for(int i=0; i<this->len; i++) {
		a = this->data[i] ^ bs->data[i];
#ifdef SDM_USE_BITCOUNT_TABLE
		ptr = (uint16_t *)&a;
		dist += (unsigned int)bitcount_table[ptr[0]] + (unsigned int)bitcount_table[ptr[1]] + bitcount_table[ptr[2]] + bitcount_table[ptr[3]];
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

std::string Bitstring::str() const {
	char str[this->bits+1];
	for(int i=0; i<this->bits; i++) {
		str[i] = this->get(i) ? '1' : '0';
	}
	str[this->bits] = '\0';
	return std::string(str);
}

std::string Bitstring::base64() const {
	// FIXME It cannot depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
	return base64_encode((const unsigned char *)this->data, sizeof(uint64_t)*this->len);
}

bool Bitstring::operator<(const Bitstring &bs) const {
	assert(this->bits == bs.bits);

	uint64_t x;
	for(int i=0; i<this->len; i++) {
		if (this->data[i] < bs.data[i]) {
			return true;
		} else if (this->data[i] > bs.data[i]) {
			return false;
		}
	}
	return false;
}

