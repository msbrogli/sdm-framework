
#ifndef SDM_SCANNER_OPENCL_H
#define SDM_SCANNER_OPENCL_H

#include <OpenCL/cl.h>

#include "bitstring.h"
#include "address_space.h"

typedef cl_ulong cl_bitstring_t;

struct opencl_scanner_s {
	struct address_space_s *address_space;

	char* kernel_name;
	size_t global_worksize;
	size_t local_worksize;

	cl_context context;
	cl_device_id device_id;
	cl_command_queue queue;
	cl_program program;

	cl_uchar bitcount_table[1<<16];
	cl_mem bitcount_table_buf;
	cl_mem bitstrings_buf;
	cl_uint bs_len;
	cl_mem bs_buf;
	cl_mem selected_buf;
	cl_uchar *selected;
	cl_mem counter_buf;

	//TimeMeasure time;
};

int opencl_scanner_init(struct opencl_scanner_s *this, struct address_space_s *as);
int as_scan_opencl(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, void *result);

#endif
