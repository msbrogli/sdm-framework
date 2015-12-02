
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>

class Bitstring {
public:
	unsigned int bits;
	unsigned int len;
	int64_t *data;

	Bitstring(unsigned int bits);
	~Bitstring();

	unsigned int getbit(unsigned int bit) const;
	void setbit(unsigned int bit);

	int distance(const Bitstring *bs) const;
};


#endif
