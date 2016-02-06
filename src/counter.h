#ifndef SDM_COUNTER_H
#define SDM_COUNTER_H

struct counter_s {
	unsigned int bits;
	unsigned int sample;
	int fd;
	char *filename;
	int *data;
};

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample);
int counter_init_file(char *filename, struct counter_s *this, unsigned int bits, unsigned int sample);
void counter_free(struct counter_s *this);
int counter_add_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs);
void counter_print(struct counter_s *this, unsigned int index);

#endif
