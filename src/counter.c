
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "bitstring.h"
#include "counter.h"

int _post_init(struct counter_s *this) {
	int i;
	this->counter = (counter_t **) malloc(sizeof(counter_t*) * this->sample);
	if (this->counter == NULL) {
		return -1;
	}
	for (i=0; i < this->sample; i++) {
		this->counter[i] = &this->data[this->bits * i];
	}
	return 0;
}

int counter_init(struct counter_s *this, unsigned int bits, unsigned int sample) {
	this->bits = bits;
	this->sample = sample;
	this->fd = -1;
	this->data = (counter_t *) malloc(sizeof(counter_t) * this->bits * this->sample);
	if (this->data == NULL) {
		return -1;
	}
	memset(this->data, 0, sizeof(counter_t) * this->bits * this->sample);
	if (_post_init(this)) {
		free(this->data);
		return -2;
	}
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
		return -2;
	}
	if (_post_init(this)) {
		munmap(this->data, sizeof(counter_t) * this->bits * this->sample);
		close(this->fd);
		return -3;
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
	free(this->counter);
}

void counter_print(struct counter_s *this, unsigned int index) {
	int i, cols = 20;
	counter_t *ptr = this->counter[index];
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
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
		// TODO Handle overflow.
		if (bs_get_bit(bs, i)) {
			ptr[i]++;
		} else {
			ptr[i]--;
		}
	}
	return 0;
}

int counter_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2) {
	int i;
	counter_t *ptr1 = c1->counter[idx1];
	counter_t *ptr2 = c2->counter[idx2];
	for(i=0; i<c1->bits; i++) {
		// TODO Handle overflow.
		ptr1[i] += ptr2[i];
	}
	return 0;
}

int counter_to_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs) {
	int i;
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
		if (ptr[i] > 0) {
			bs_set_bit(bs, i, 1);
		} if (ptr[i] < 0) {
			bs_set_bit(bs, i, 0);
		} else {
			// Flip a coin! :)
			bs_set_bit(bs, i, arc4random() % 2);
		}
	}
	return 0;
}

