#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#include "lib/base64.h"

template <typename integer>
class MappedCounter: public Counter<integer> {
public:
    string filepath;
    MappedCounter(unsigned int bits, string filepath);
    MappedCounter(unsigned int bits, std::string const &b64, string filepath);
    Mapped(Counter const &ct);
    ~MappedCounter();

    void _init(unsigned int bits);
};

template <typename integer>
virtual void Counter<integer>::_init(unsigned int bits) {
    this->bits = bits;
    FILE *file = fopen(this->filepath, "r+");
    size_t page_size = (size_t) sysconf (_SC_PAGESIZE);
    // More on mmap: http://www.gnu.org/software/libc/manual/html_node/Memory_002dmapped-I_002fO.html
    // Will we use offset (for a header maybe)? Is map shared worth it?
    this->data = (integer *) mmap(NULL, sizeof(integer)*this->bits, PROT_READ | PROT_WRITE, MAP_SHARED, 0*page_size);
}
