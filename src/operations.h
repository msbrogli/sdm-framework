
#ifndef SDM_OPERATIONS_H
#define SDM_OPERATIONS_H

struct sdm_s {
	int bits;
	int sample;

	struct address_space_s *address_space;
	struct counter_s *counter;
};

#endif
