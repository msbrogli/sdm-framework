
#ifndef SDM_OPERATIONS_H
#define SDM_OPERATIONS_H

#ifdef SDM_ENABLE_OPENCL
#include "scanner_opencl.h"
#endif

#define SDM_SCANNER_LINEAR 1
#define SDM_SCANNER_THREAD 2
#ifdef SDM_ENABLE_OPENCL
#define SDM_SCANNER_OPENCL 3
#endif

struct sdm_s {
	unsigned int bits;
	unsigned int sample;

	unsigned int scanner_type;

#ifdef SDM_ENABLE_OPENCL
	/* Options for SDM_SCANNER_OPENCL. */
	struct opencl_scanner_s *opencl_opts;
#endif

	/* Number of threads for SDM_SCANNER_THREAD. */
	unsigned int thread_count;

	struct address_space_s *address_space;
	struct counter_s *counter;
};

int sdm_init_linear(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter);
int sdm_init_thread(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, unsigned int thread_count);
#ifdef SDM_ENABLE_OPENCL
int sdm_init_opencl(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, char *opencl_source);
#endif

void sdm_free(struct sdm_s *sdm);

int sdm_write(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *datum);
int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output);
int sdm_iter_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, unsigned int max_iter, bitstring_t *output);
int sdm_generic_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output, double z);
int sdm_read_counter(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, struct counter_s *counter);

#endif
