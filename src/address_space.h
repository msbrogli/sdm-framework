
#ifndef SDM_ADDRESS_SPACE_H
#define SDM_ADDRESS_SPACE_H

#include <vector>

#include "bitstring.h"
#include "address.h"

class AddressSpace {
	unsigned int bits;
	unsigned int len;

	Address *addresses;

	int scan(const Bitstring *bs, unsigned int radius, std::vector<Address*> *dst);
};

#endif
