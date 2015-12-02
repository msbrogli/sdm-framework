
#ifndef SDM_ADDRESS_SPACE_H
#define SDM_ADDRESS_SPACE_H

#include <vector>

#include "bitstring.h"

class AddressSpace {
public:
	unsigned int bits;
	unsigned int len;

	Bitstring *addresses;

	int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring*> *dst);
};

#endif
