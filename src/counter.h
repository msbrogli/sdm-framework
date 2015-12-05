#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>

#include "lib/base64.h"
#include "counter.h"

template <typename integer>
class Counter {
public:
    unsigned int bits;
    integer *data;

    Counter(unsigned int bits);
    Counter(unsigned int bits, std::string const &b64);
    Counter(Counter const &ct);
    ~Counter();

    void _init(unsigned int bits);

    integer get(unsigned int bit) const;
    void set(unsigned int bit, integer value);

    void incr(unsigned int bit);
    void decr(unsigned int bit);

    std::string str() const;
    std::string base64() const;
};

template <typename integer>
void Counter<integer>::_init(unsigned int bits) {
    this->bits = bits;
    this->data = (integer *) malloc(sizeof(integer) * this->bits);
}

template <typename integer>
Counter<integer>::Counter(unsigned int bits) {
    this->_init(bits);
    memset(this->data, 0, sizeof(integer) * this->bits);
}

template <typename integer>
Counter<integer>::Counter(unsigned int bits, std::string const &b64) {
    const size_t size = sizeof(integer) * this->bits;
    std::string buffer = base64_decode(b64);
    assert(size == buffer.length());

    this->_init(bits);
    memcpy(this->data, buffer.c_str(), size);
}

template <typename integer>
Counter<integer>::Counter(Counter<integer> const &ct) {
    this->_init(ct.bits);
    memcpy(this->data, ct.data, sizeof(integer) * this->bits);
}

template <typename integer>
Counter<integer>::~Counter() {
    free(this->data);
}

template <typename integer>
integer Counter<integer>::get(unsigned int bit) const {
    assert(bit <= this->bits);
    return data[bit];
}

template <typename integer>
void Counter<integer>::set(unsigned int bit, integer value) {
    assert(bit <= this->bits);
    data[bit] = value;
}

template <typename integer>
void Counter<integer>::incr(unsigned int bit) {
    // FIXME Athos Couto - [03-12-2015]: Treat overflow
    assert(bit <= this->bits);
    data[bit]++;
}

template <typename integer>
void Counter<integer>::decr(unsigned int bit) {
    // FIXME Athos Couto - [03-12-2015]: Treat overflow
    assert(bit <= this->bits);
    data[bit]--;
}

template <typename integer>
std::string Counter<integer>::str() const {
    unsigned int tab = 50;
    unsigned int sz = sizeof(integer);
    std::stringstream ss;
    for(int i=0; i<this->bits; i++) {
        if (i % tab == 0) {
            ss << i << ": " << data[i] << ", ";
        } else if (i % tab == tab - 1) {
            ss << data[i] << ";" << std::endl;
        } else {
            ss << data[i] << ", ";
        }
    }
    return ss.str();
}

template <typename integer>
std::string Counter<integer>::base64() const {
    // FIXME It cannot depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
    return base64_encode((const unsigned char *)this->data, sizeof(int64_t)*this->bits);
}
#endif
