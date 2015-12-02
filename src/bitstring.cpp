
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

