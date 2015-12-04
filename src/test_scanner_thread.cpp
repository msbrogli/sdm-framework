
#include <iostream>
#include <assert.h>

#include "address_space.h"
#include "bitstring.h"
#include "scanner_thread.h"

void run(const unsigned int bits, const unsigned int sample, int radius) {
	std::cout << "bits=" << bits << ", sample=" << sample << ", radius=" << radius << std::endl;

	AddressSpace *addresses = new AddressSpace(bits, sample);
	Scanner *scanner1 = new LinearScanner(addresses);
	Scanner *scanner2 = new ThreadScanner(addresses, 8);

	Bitstring *bs = new Bitstring(bits);

	std::vector<Bitstring *> v1;
	std::vector<Bitstring *> v2;

	{
		clock_t begin_time = clock();
		scanner1->scan(bs, radius, &v1);
		std::cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
		std::cout << v1.size() << " hard-locations activated." << std::endl;
	}

	{
		clock_t begin_time = clock();
		scanner2->scan(bs, radius, &v2);
		std::cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC << std::endl;
		std::cout << v2.size() << " hard-locations activated." << std::endl;
	}

	assert(v1.size() == v2.size());

	free(bs);
	free(addresses);
	free(scanner1);
	free(scanner2);
}

int main(void) {
	run(100, 10000, 45);
	run(1000, 1000000, 451);
	return 0;
}

