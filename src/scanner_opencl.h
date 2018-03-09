
#ifndef SDM_SCANNER_OPENCL_H
#define SDM_SCANNER_OPENCL_H

#ifdef OS_OSX
#include <OpenCL/cl.h>
#endif

#ifdef OS_LINUX
#include <CL/cl.h>
#endif

#include "bitstring.h"
#include "address_space.h"

typedef cl_ulong cl_bitstring_t;

struct opencl_scanner_s {
	//---
	// These fields must be on the top because they are the only ones available in Python.
	char* kernel_name;
	size_t global_worksize;
	size_t local_worksize;
	unsigned int max_compute_units;
	unsigned int verbose;
	//---

	struct address_space_s *address_space;
	char* opencl_source;

	cl_context context;
	cl_program program;

	unsigned int devices_count;
	cl_device_id *devices;
	cl_command_queue *queues;

	cl_uchar bitcount_table[1<<16];
	cl_mem bitcount_table_buf;
	cl_mem bitstrings_buf;
	cl_uint bs_len;
	cl_mem bs_buf;
	cl_mem selected_buf;
	cl_mem counter_buf;
};

int as_scanner_opencl_init(struct opencl_scanner_s *this, struct address_space_s *as, char *opencl_source);
void as_scanner_opencl_free(struct opencl_scanner_s *this);
int as_scan_opencl(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, uint8_t *result);
int as_scan_opencl2(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, unsigned int *result);
void opencl_scanner_devices(struct opencl_scanner_s *this);

#endif
