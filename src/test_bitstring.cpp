
#include <iostream>
#include <assert.h>
#include "bitstring.h"

void run(const int bits) {
	Bitstring *bs1 = new Bitstring(bits);
	std::cout << bs1->str() << "\n";
	std::cout << bs1->base64() << "\n";

	for (int i=0; i<bits; i++) {
		bs1->set(i, 1);
		assert(bs1->get(i) == 1);
		assert(bs1->distance(bs1) == 0);

		bs1->set(i, 0);
		assert(bs1->get(i) == 0);
		assert(bs1->distance(bs1) == 0);
	}
	std::cout << bs1->str() << "\n";
	std::cout << bs1->base64() << "\n";

	Bitstring *bs2 = new Bitstring(bits);
	for (int i=0; i<bits; i++) {
		bs2->set(i, i%2);
		assert(bs2->distance(bs2) == 0);
	}
	std::cout << bs2->str() << "\n";
	std::cout << bs2->base64() << "\n";

	int d = bs1->distance(bs2);
	std::cout << bits << "\n";
	assert(d == bits/2);

	for(int i=0; i<1000; i++) {
		Bitstring a(bits);
		Bitstring b(a);
		assert(a.distance(&b) == 0);
	}

	for(int i=0; i<1000; i++) {
		Bitstring a(bits);
		Bitstring b(bits, a.base64());
		assert(a.distance(&b) == 0);
	}

	std::cout << "Success.\n";

	delete bs1;
	delete bs2;
}

int main(void) {
	run(1000);
	return 0;
}
