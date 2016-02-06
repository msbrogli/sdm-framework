
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitstring.h"
#include "counter.h"

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;
	this->data = (int *) malloc(sizeof(int) * this->bits * this->sample);
	if (this->data == NULL) {
		return -1;
	}
	memset(this->data, 0, sizeof(int) * this->bits * this->sample);
	return 0;
}

void counter_free(struct counter_s *this) {
	free(this->data);
}

void counter_print(struct counter_s *this, unsigned int index) {
	int i, cols = 20;
	int *ptr = &this->data[index*this->bits];
	for(i=0; i<this->bits; i++) {
		if(i%cols == 0) {
			if (i > 0) printf("\n");
			printf("[%6d] ", i);
		}
		printf("%3d ", ptr[i]);
	}
	printf("\n");
}

int counter_add_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs) {
	int i;
	int *ptr = &this->data[index*this->bits];
	for(i=0; i<this->bits; i++) {
		if (bs_get_bit(bs, i)) {
			ptr[i]++;
		} else {
			ptr[i]--;
		}
	}
	return 0;
}

