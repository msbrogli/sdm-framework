
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "bitstring.h"
#include "counter.h"

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;
	this->fd = -1;
	this->data = (counter_t *) malloc(sizeof(counter_t) * this->bits * this->sample);
	if (this->data == NULL) {
		return -1;
	}
	memset(this->data, 0, sizeof(counter_t) * this->bits * this->sample);
	return 0;
}

int counter_init_file(char *filename, struct counter_s *this, unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;
	this->filename = filename;
	this->fd = open(this->filename, O_RDWR);
	if (this->fd == -1) {
		return -1;
	}
	// FIXME Possible problem with big-endian and little-endian format.
	this->data = (counter_t *) mmap(0, sizeof(counter_t) * this->bits * this->sample, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0);
	if (this->data == MAP_FAILED) {
		close(this->fd);
		return -1;
	}
	return 0;
}

void counter_free(struct counter_s *this) {
	if (this->fd != -1) {
		munmap(this->data, sizeof(counter_t) * this->bits * this->sample);
		close(this->fd);
	} else {
		free(this->data);
	}
}

void counter_print(struct counter_s *this, unsigned int index) {
	int i, cols = 20;
	counter_t *ptr = &this->data[index*this->bits];
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
	counter_t *ptr = &this->data[index*this->bits];
	for(i=0; i<this->bits; i++) {
		if (bs_get_bit(bs, i)) {
			ptr[i]++;
		} else {
			ptr[i]--;
		}
	}
	return 0;
}

