
#ifndef SDM_SCANNER_H
#define SDM_SCANNER_H

#include "address_space.h"
#include "bitstring.h"

#include <vector>

class Scanner {
public:
	virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const = 0;
};

class LinearScanner : Scanner {
public:
	AddressSpace *addresses;
	LinearScanner(AddressSpace *addresses);
	virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const;
};

#endif
