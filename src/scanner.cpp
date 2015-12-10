
#include "scanner.h"

Scanner::~Scanner() {
}

LinearScanner::LinearScanner(AddressSpace *addresses) {
	this->addresses = addresses;
}

int LinearScanner::scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const {
	Bitstring *b;
	const int n = this->addresses->sample;
	for(int i=0; i<n; i++) {
		b = this->addresses->addresses[i];
		if (b->distance(bs) <= radius) {
			result->push_back(b);
		}
	}
	return 0;
}
