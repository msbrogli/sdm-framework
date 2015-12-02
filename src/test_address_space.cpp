
#include <iostream>
#include <assert.h>
#include "address_space.h"

void run(const int N) {
	AddressSpace *s1 = new AddressSpace(N, 1000000);
	std::cout << "Address space generated." << std::endl;
	std::cout << "Hash: " << s1->hash() << std::endl;
	s1->save("teste1.txt");
	free(s1);
}

int main(void) {
	run(1000);
	return 0;
}
