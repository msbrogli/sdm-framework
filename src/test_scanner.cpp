
#include <iostream>

#include "address_space.h"
#include "bitstring.h"
#include "scanner.h"

void run(const unsigned int bits, const unsigned int sample, int radius) {
	std::cout << "bits=" << bits << ", sample=" << sample << ", radius=" << radius << std::endl;

	AddressSpace *addresses = new AddressSpace(bits, sample);
	LinearScanner *scanner = new LinearScanner(addresses);

	Bitstring *bs = new Bitstring(bits);

	std::vector<Bitstring *> v;
	scanner->scan(bs, radius, &v);

	std::cout << v.size() << " hard-locations activated." << std::endl;

	free(bs);
	free(addresses);
	free(scanner);
}

int main(void) {
	run(100, 10000, 45);
	run(1000, 1000000, 451);
	return 0;
}

