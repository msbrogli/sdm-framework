
#ifndef SDM_SCANNER_OPENCL_H
#define SDM_SCANNER_OPENCL_H

#include <OpenCL/cl.h>

#include "scanner.h"
#include "bitstring.h"
#include "address_space.h"
#include "utils.h"

class OpenCLScanner : public Scanner {
	public:
		AddressSpace *addresses;

		std::string kernel_name;
		size_t global_worksize;
		size_t local_worksize;

		cl_context context;
		cl_device_id device_id;
		cl_command_queue queue;
		cl_program program;

		cl_uchar bitcount_table[1<<16];
		cl_mem bitcount_table_buf;
		cl_ushort *bitstrings;
		cl_mem bitstrings_buf;
		cl_uint bs_len;
		cl_mem bs_buf;
		cl_mem selected_buf;
		cl_uchar *selected;
		cl_mem counter_buf;

		TimeMeasure time;

		OpenCLScanner(AddressSpace *addresses);
		~OpenCLScanner();

		virtual void devices() const;
		virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const;
		virtual int scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result, TimeMeasure *time) const;
};

#endif
