
#ifndef SDM_OPERATIONS_H
#define SDM_OPERATIONS_H

struct sdm_s {
	int bits;
	int sample;

	struct address_space_s *address_space;
	struct counter_s *counter;
};

int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output);
int sdm_write(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *datum);

#endif
