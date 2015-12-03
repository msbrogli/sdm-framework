
#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

#include <stdint.h>
#include <string>

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


#endif
