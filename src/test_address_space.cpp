
#include <iostream>
#include <assert.h>
#include "address_space.h"

void run(const int bits, const int sample) {
	AddressSpace *s1 = new AddressSpace(bits, sample);
	std::cout << "Address space generated." << std::endl;
	std::cout << "Hash: " << s1->hash() << std::endl;
	s1->save("teste1.txt");
	delete s1;

	AddressSpace *s2 = new AddressSpace("teste1.txt");
	std::cout << "Hash: " << s2->hash() << std::endl;
	delete s2;
}

int main(void) {
	run(1000, 1000000);
	return 0;
}
