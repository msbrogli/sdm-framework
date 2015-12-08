
#ifndef SDM_SCANNER_OPENCL_H
#define SDM_SCANNER_OPENCL_H

#include <OpenCL/cl.h>

#include "scanner.h"
#include "bitstring.h"
#include "address_space.h"

class OpenCLScanner : public Scanner {
	public:
		AddressSpace *addresses;

		cl_context context;
		cl_device_id device_id;
		cl_command_queue queue;
		cl_program program;

		cl_ushort bitcount_table[1<<16];
		cl_mem bitcount_table_buf;
		cl_ulong *bitstrings;
		cl_mem bitstrings_buf;
		cl_uint bs_len;

		OpenCLScanner(AddressSpace *addresses);
		virtual void devices() const;
		virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const;
};

#endif
