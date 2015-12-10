
#ifndef SDM_ADDRESS_SPACE_H
#define SDM_ADDRESS_SPACE_H

#include <vector>
#include <string>

#include "bitstring.h"

class AddressSpace {
public:
	unsigned int bits;
	unsigned int sample;

	std::vector<Bitstring*> addresses;

	AddressSpace(unsigned int bits, unsigned int sample);
	AddressSpace(std::string filename);

	~AddressSpace();

	int save(std::string filename) const;
	int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring*> *dst) const;

	std::string hash() const;
};

#endif
