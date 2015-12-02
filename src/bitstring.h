
#ifndef SDM_BITSTRING_H
#define SDM_BITSTRING_H

#include <stdint.h>
#include <string>

class Bitstring {
public:
	unsigned int bits;
	unsigned int len;
	int64_t *data;

	Bitstring(unsigned int bits);
	Bitstring(unsigned int bits, std::string const &b64);
	Bitstring(Bitstring const &bs);
	~Bitstring();

	void _init(unsigned int bits);
	void clear_surplus();

	unsigned int get(unsigned int bit) const;
	void set(unsigned int bit, unsigned int value);

	int distance(const Bitstring *bs) const;

	std::string str() const;
	std::string base64() const;

	bool operator<(const Bitstring &bs) const;
};


#endif
