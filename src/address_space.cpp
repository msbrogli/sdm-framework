
#include <iostream>
#include <fstream>

#include <base64.h>

#include "address_space.h"

AddressSpace::AddressSpace(unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;

	for(int i=0; i<this->sample; i++) {
		this->addresses.push_back(new Bitstring(this->bits));
	}
}

int AddressSpace::scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring*> *dst) const {
	if (bs->bits != this->bits) {
		return -1;
	}
	const unsigned int n = this->sample;
	for(int i=0; i<n; i++) {
		Bitstring *addr = this->addresses[i];
		if (bs->distance(addr) <= radius) {
			dst->push_back(addr);
		}
	}
	return 0;
}

int AddressSpace::save(std::string filename) const {
	std::ofstream file(filename);
	file << "SDM|v0.0.1|ADDRESS SPACE|v1|" << this->bits << "|" << this->sample << "\n";
	const unsigned int n = this->sample;
	for(int i=0; i<n; i++) {
		Bitstring *addr = this->addresses[i];
		// FIXME It cannot depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
		file << i << "|" << base64_encode((const unsigned char *)addr->data, 8*addr->len) << "\n";
	}
	file.close();
	return 0;
}
