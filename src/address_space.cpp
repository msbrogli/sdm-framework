
#include "address_space.h"

int AddressSpace::scan(const Bitstring *bs, unsigned int radius, std::vector<Address*> *dst) {
	if (bs->len != this->len) {
		return -1;
	}
	const unsigned int n = this->len;
	for(int i=0; i<n; i++) {
		Address *addr = &this->addresses[i];
		if (bs->distance(addr->bitstring) <= radius) {
			dst->push_back(addr);
		}
	}
	return 0;
}

