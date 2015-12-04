#include <iostream>
#include <assert.h>
#include "counter.h"

void run(const int N) {
    Counter<int> *ct1 = new Counter<int>(N);
    std::cout << ct1->str() << std::endl << std::endl;
    std::cout << ct1->base64() << std::endl << std::endl;

    for (int i=0; i<N; i++) {
        ct1->set(i, 1);
        assert(ct1->get(i) == 1);
        ct1->set(i, 0);
        assert(ct1->get(i) == 0);
        ct1->incr(i);
        assert(ct1->get(i) == 1);
        ct1->decr(i);
        assert(ct1->get(i) == 0);
    }
    std::cout << "Success.\n";
    delete ct1;
}

int main(void) {
    run(1000);
    return 0;
}
