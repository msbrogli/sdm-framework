
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <assert.h>

#include "lib/sha256.h"
#include "utils.h"

#include "address_space.h"

AddressSpace::AddressSpace(unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;

	for(int i=0; i<this->sample; i++) {
		this->addresses.push_back(new Bitstring(this->bits));
	}

}

AddressSpace::~AddressSpace() {
	for(int i=0; i<this->addresses.size(); i++) {
		delete this->addresses[i];
	}
	this->addresses.clear();
}

AddressSpace::AddressSpace(std::string filename) {
	std::ifstream file(filename);
	std::string buffer;

	std::getline(file, buffer);
	assert(buffer == "SDM:ADDRESS SPACE");

	// Read headers.
	std::map<std::string, std::string> headers;
	unsigned int idx;
	std::string key, value;
	while(std::getline(file, buffer)) {
		if (buffer.length() == 0) {
			break;
		}
		idx = buffer.find(":");

		key = buffer.substr(0, idx);
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		value = trim(buffer.substr(idx+1));

		headers[key] = value;
	}
	this->bits = std::stoi(headers["bits"]);
	this->sample = std::stoi(headers["sample"]);

	// Read addresses.
	while(std::getline(file, buffer)) {
		this->addresses.push_back(new Bitstring(this->bits, buffer));
	}
	assert(this->sample == this->addresses.size());
	assert(this->hash() == headers["hash"]);
}

std::string AddressSpace::hash() const {
	SHA256 hash = SHA256();
	hash.init();
	for(int i=0; i<this->sample; i++) {
		hash.update((const unsigned char *)this->addresses[i]->data, sizeof(int64_t)*this->addresses[i]->len);
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
	file << "SDM:ADDRESS SPACE\nSDM-Version: v0.0.1\nFormat: base64-v1\n";
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

