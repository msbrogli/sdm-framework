
#include <iostream>
#include <assert.h>
#include <set>

#include "address_space.h"
#include "bitstring.h"
#include "scanner_thread.h"
#include "scanner_opencl.h"
#include "utils.h"

void run(const unsigned int bits, const unsigned int sample, const unsigned int radius) {
	const unsigned int n = 1000;

	std::cout << "bits=" << bits << ", sample=" << sample << ", radius=" << radius << ", iteractions=" << n << std::endl;

	AddressSpace *addresses = new AddressSpace(bits, sample);
	LinearScanner *scanner1 = new LinearScanner(addresses);
	ThreadScanner *scanner2 = new ThreadScanner(addresses, 8);
	OpenCLScanner *scanner3 = new OpenCLScanner(addresses);

	TimeMeasure *tm = new TimeMeasure();
	tm->start();

	for(int i=0; i<n; i++) {
		Bitstring *bs = new Bitstring(bits);
		std::vector<Bitstring *> v1;
		scanner1->scan(bs, radius, &v1);
		delete bs;
	}
	tm->mark("LinearScanner");
	std::cout << tm->str();
	tm->restart();

	for(int i=0; i<n; i++) {
		Bitstring *bs = new Bitstring(bits);
		std::vector<Bitstring *> v1;
		scanner2->scan(bs, radius, &v1);
		delete bs;
	}
	tm->mark("ThreadScanner (8 threads)");
	std::cout << tm->str();
	tm->restart();

	for(int i=0; i<n; i++) {
		Bitstring *bs = new Bitstring(bits);
		std::vector<Bitstring *> v1;
		scanner3->scan(bs, radius, &v1);
		delete bs;
	}
	tm->mark("OpenCLScanner");
	std::cout << tm->str() << std::endl;
	tm->restart();

	delete tm;

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

