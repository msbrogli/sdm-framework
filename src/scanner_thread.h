
#ifndef SDM_SCANNER_THREAD_H
#define SDM_SCANNER_THREAD_H

#include "scanner.h"
#include "bitstring.h"
#include "address_space.h"

#include <vector>

struct thread_params_t {
	int id;
	const Bitstring* address;
	std::vector<Bitstring *> *addresses;
	std::vector<Bitstring *> result;
	unsigned int radius;
	unsigned int idx_begin;
	unsigned int idx_end;
};

class ThreadScanner : public Scanner {
	public:
		AddressSpace *addresses;
		unsigned int thread_count;

		ThreadScanner(AddressSpace *addresses, unsigned int thread_count);
		virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const;
};

#endif
