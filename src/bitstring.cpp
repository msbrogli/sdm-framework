
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <base64.h>
#include "bitstring.h"

void Bitstring::_init(unsigned int bits) {
	this->bits = bits;

	this->len = bits/64;
	if (bits % 64 > 0) {
		this->len++;
	}

	this->data = (int64_t *) malloc(sizeof(int64_t) * this->len);
}

void Bitstring::clear_surplus() {
	int last = this->bits % 64;
	if (last > 0) {
		this->data[this->len-1] &= ~((int64_t)0xFFFFFFFFFFFFFFFF << last);
	}
}

Bitstring::Bitstring(unsigned int bits) {
	this->_init(bits);
	arc4random_buf(this->data, sizeof(int64_t) * this->len);
	this->clear_surplus();
}

Bitstring::Bitstring(unsigned int bits, std::string const &b64) {
	const size_t size = sizeof(int64_t) * this->len;
	std::string buffer = base64_decode(b64);
	assert(size == buffer.length());

	this->_init(bits);
	memcpy(this->data, buffer.c_str(), size);
	this->clear_surplus();
}

Bitstring::Bitstring(Bitstring const &bs) {
	this->_init(bs.bits);
	memcpy(this->data, bs.data, sizeof(int64_t) * this->len);
	this->clear_surplus();
}

Bitstring::~Bitstring() {
	free(this->data);
}

unsigned int Bitstring::get(unsigned int bit) const {
	assert(bit <= this->bits);
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	return (data[offset] & ((int64_t)1<<idx) ? 1 : 0);
}

void Bitstring::set(unsigned int bit, unsigned int value) {
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	if (value == 0) {
		data[offset] &= ~((int64_t)1<<idx);
	} else {
		data[offset] |= ((int64_t)1<<idx);
	}
}

int Bitstring::distance(const Bitstring *bs) const {
	if (this->bits != bs->bits) {
		return -1;
	}

	uint64_t a;
	int dist = 0;
	for(int i=0; i<this->len; i++) {
		a = this->data[i] ^ bs->data[i];
		while(a) {
			if (a%2) {
				dist++;
			}
			a >>= 1;
		}
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
	return base64_encode((const unsigned char *)this->data, sizeof(int64_t)*this->len);
}

bool Bitstring::operator<(const Bitstring &bs) const {
	assert(this->bits == bs.bits);

	int64_t x;
	for(int i=0; i<this->len; i++) {
		x = this->data[i] - bs.data[i];
		if (x < 0) {
			return true;
		} else if (x > 0) {
			return false;
		}
	}
	return false;
}

