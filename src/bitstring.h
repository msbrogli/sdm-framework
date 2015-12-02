
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>
#include <string>

class Bitstring {
public:
	unsigned int bits;
	unsigned int len;
	int64_t *data;

	Bitstring(unsigned int bits);
	~Bitstring();

	unsigned int get(unsigned int bit) const;
	void set(unsigned int bit, unsigned int value);

	int distance(const Bitstring *bs) const;

	std::string str() const;
};


#endif
