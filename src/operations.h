
#ifndef SDM_OPERATIONS_H
#define SDM_OPERATIONS_H

#include "scanner_opencl.h"

#define SDM_SCANNER_LINEAR 1
#define SDM_SCANNER_THREAD 2
#define SDM_SCANNER_OPENCL 3

struct sdm_s {
	unsigned int bits;
	unsigned int sample;

	unsigned int scanner_type;

	// Options for SDM_SCANNER_OPENCL.
	struct opencl_scanner_s *opencl_opts;

	// Number of threads for SDM_SCANNER_THREAD.
	unsigned int thread_count;

	struct address_space_s *address_space;
	struct counter_s *counter;
};

int sdm_init_linear(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter);
int sdm_init_thread(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, unsigned int thread_count);
int sdm_init_opencl(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter);

void sdm_free(struct sdm_s *sdm);

int sdm_write(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *datum);
int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output);
int sdm_iter_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, unsigned int max_iter, bitstring_t *output);

#endif
