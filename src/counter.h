#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

#include <limits.h>

typedef int counter_t;

#define COUNTER_MAX INT_MAX;
#define COUNTER_MIN INT_MIN;

// Uncomment to enable counter overflow checking.
//#define COUNTER_CHECK_OVERFLOW

struct counter_s {
	unsigned int bits;
	unsigned int sample;
	int fd;
	char *filename;
	counter_t **counter;
	counter_t *data;
};

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample);
int counter_init_file(char *filename, struct counter_s *this);
void counter_free(struct counter_s *this);
void counter_print_summary(struct counter_s *this);

void counter_reset(struct counter_s *this, unsigned int index);
void counter_print(struct counter_s *this, unsigned int index);

int counter_add_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);
int counter_add_bitstring_weighted(struct counter_s *this, unsigned int index, bitstring_t *bs, int weight);
int counter_sub_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);
int counter_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2);
int counter_weighted_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2, unsigned int weight);

int counter_to_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);

int counter_create_file(char *filename, unsigned int bits, unsigned int sample);
int counter_save_file(struct counter_s *this, char *filename);

#endif
