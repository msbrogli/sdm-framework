
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>

class Bitstring {
	unsigned int bits;
	unsigned int len;
	int64_t *data;

	Bitstring(unsigned int bits);
	~Bitstring();
};


#endif
