
#include <iostream>
#include <assert.h>
#include "bitstring.h"

void run(const int N) {
	Bitstring *bs1 = new Bitstring(N);
	std::cout << bs1->str() << "\n";
	std::cout << bs1->base64() << "\n";

	for (int i=0; i<N; i++) {
		bs1->set(i, 1);
		assert(bs1->get(i) == 1);
		assert(bs1->distance(bs1) == 0);

		bs1->set(i, 0);
		assert(bs1->get(i) == 0);
		assert(bs1->distance(bs1) == 0);
	}
	std::cout << bs1->str() << "\n";
	std::cout << bs1->base64() << "\n";

	Bitstring *bs2 = new Bitstring(N);
	for (int i=0; i<N; i++) {
		bs2->set(i, i%2);
		assert(bs2->distance(bs2) == 0);
	}
	std::cout << bs2->str() << "\n";
	std::cout << bs2->base64() << "\n";

	int d = bs1->distance(bs2);
	std::cout << N << "\n";
	assert(d == N/2);

	std::cout << "Success.\n";

	free(bs1);
}

int main(void) {
	run(1000);
	return 0;
}
