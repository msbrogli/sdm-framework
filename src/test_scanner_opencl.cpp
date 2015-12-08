
#include <iostream>
#include <assert.h>
#include <set>

#include "address_space.h"
#include "bitstring.h"
#include "scanner_thread.h"
#include "scanner_opencl.h"

void run(const unsigned int bits, const unsigned int sample, int radius) {
	std::cout << "bits=" << bits << ", sample=" << sample << ", radius=" << radius << std::endl;

	AddressSpace *addresses = new AddressSpace(bits, sample);
	LinearScanner *scanner1 = new LinearScanner(addresses);
	ThreadScanner *scanner2 = new ThreadScanner(addresses, 8);
	OpenCLScanner *scanner3 = new OpenCLScanner(addresses);

	//scanner3->devices();

	for(int i=0; i<20; i++) {
		Bitstring *bs = new Bitstring(bits);

		std::vector<Bitstring *> v1;
		std::vector<Bitstring *> v2;
		std::vector<Bitstring *> v3;

		scanner1->scan(bs, radius, &v1);
		std::cout << v1.size() << " hard-locations activated." << std::endl;

		scanner2->scan(bs, radius, &v2);
		std::cout << v2.size() << " hard-locations activated." << std::endl;

		scanner3->scan(bs, radius, &v3);
		std::cout << v3.size() << " hard-locations activated." << std::endl;

		assert(v1.size() == v2.size());
		assert(v2.size() == v3.size());

		std::set<Bitstring *> s1(v1.begin(), v1.end());
		std::set<Bitstring *> s2(v2.begin(), v2.end());
		std::set<Bitstring *> s3(v3.begin(), v3.end());

		assert(s1 == s2);
		assert(s2 == s3);

		delete bs;
	}

	delete addresses;
	delete scanner1;
	delete scanner2;
	delete scanner3;
}

int main(void) {
	run(100, 10000, 45);
	run(256, 100000, 120);
	run(1000, 1000000, 451);
	return 0;
}

