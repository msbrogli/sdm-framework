
#include <iostream>
#include <fstream>

#include "lib/sha256.h"

#include "address_space.h"

AddressSpace::AddressSpace(unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;

	for(int i=0; i<this->sample; i++) {
		this->addresses.push_back(new Bitstring(this->bits));
	}

}

std::string AddressSpace::hash() const {
	std::vector<Bitstring*> v(this->addresses);
	std::sort(v.begin(), v.end());

	SHA256 hash = SHA256();
	hash.init();
	for(int i=0; i<this->sample; i++) {
		hash.update((const unsigned char *)v[i]->data, sizeof(int64_t)*v[i]->len);
	}
	return hash.string();
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
	file << "SDM:ADDRESS SPACE\nSDM-Version: v0.0.1\nFormat: v1\n";
	file << "Bits: " << this->bits << "\nSample: " << this->sample << "\nHash: " << this->hash() << "\n\n";
	const unsigned int n = this->sample;
	for(int i=0; i<n; i++) {
		Bitstring *addr = this->addresses[i];
		// It does not depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
		file << addr->base64() << "\n";
	}
	file.close();
	return 0;
}

/*
int AddressSpace::load(std::string filename) {
	std::ifstream file(filename);
	std::string buffer;

	std::getline(file, buffer);
	std::cout << buffer << std::endl;

	int cnt = 0;
	while(std::getline(file, buffer)) {
		cnt++;
	}

	std::cout << cnt << std::endl;
}
*/
