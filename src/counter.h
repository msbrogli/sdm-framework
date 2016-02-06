#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

typedef int counter_t;

struct counter_s {
	unsigned int bits;
	unsigned int sample;
	int fd;
	char *filename;
	counter_t **counter;
	counter_t *data;
};

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample);
int counter_init_file(char *filename, struct counter_s *this, unsigned int bits, unsigned int sample);
void counter_free(struct counter_s *this);

void counter_print(struct counter_s *this, unsigned int index);

int counter_add_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);
int counter_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2);

int counter_to_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);

#endif
