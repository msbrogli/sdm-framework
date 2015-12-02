
#include <stdlib.h>
#include "bitstring.h"

Bitstring::Bitstring(unsigned int bits) {
	this->bits = bits;

	this->len = bits/64;
	if (bits % 64 > 0) {
		this->len++;
	}

	this->data = (int64_t *) malloc(sizeof(int64_t) * this->len);
}

Bitstring::~Bitstring() {
	free(this->data);
}

unsigned int Bitstring::getbit(unsigned int bit) const {
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	return (data[offset] & ((int64_t)1<<idx) ? 1 : 0);
}

void Bitstring::setbit(unsigned int bit) {
	unsigned int offset = bit / 64;
	unsigned int idx = bit % 64;
	if (bit == 0) {
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
		a = this->data[i] % bs->data[i];
		while(a) {
			if (a%2) {
				dist++;
			}
			a >>= 1;
		}
	}
	return dist;
}

/*
string Bitstring::str() {
	char str[this->bits+1];
	for(int i=0; i<this->bits; i++) {
		str[i] = 
	}
	return string(str);
}
*/

