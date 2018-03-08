
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include <assert.h>
#include "bitstring.h"
#include "counter.h"
#include "utils.h"
#include "version.h"

int _post_init(struct counter_s *this) {
	unsigned int i;
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

void counter_print_summary(struct counter_s *this) {
	printf("Dimension: %d\n", this->bits);
	printf("Numer of hardlocations: %d\n", this->sample);
	if (this->fd != -1) {
		printf("Filename: %s\n", this->filename);
	}
}

static
int save(char *filename, unsigned int bits, unsigned int sample, counter_t *data, unsigned int nitems) {
	FILE *fp1, *fp2;
	unsigned int i;

	char meta[1000], bin[1000];
	sprintf(meta, "%s.meta", filename);
	sprintf(bin, "%s.bin", filename);

	fp1 = fopen(meta, "w");
	if (fp1 == NULL) {
		return -1;
	}
	fp2 = fopen(bin, "w");
	if (fp2 == NULL) {
		return -2;
	}

	fprintf(fp1, "SDM COUNTER\n");
	fprintf(fp1, "SDM-Version: " SDM_VERSION_STR "\n");
	fprintf(fp1, "Format: binary\n");
	fprintf(fp1, "Bytes-per-counter: %lu\n", sizeof(counter_t));
	fprintf(fp1, "Order-of-bytes: %s\n", (is_little_endian() ? "little-endian" : "big-endian"));
	fprintf(fp1, "Bits: %d\n", bits);
	fprintf(fp1, "Sample: %d\n", sample);

	for(i=0; i<sample; i+=nitems) {
		fwrite(data, sizeof(counter_t)*bits, nitems, fp2);
	}

	fclose(fp1);
	fclose(fp2);
	return 0;
}

int counter_save_file(struct counter_s *this, char *filename) {
	return save(filename, this->bits, this->sample, this->data, this->sample);
}

int counter_create_file(char *filename, unsigned int bits, unsigned int sample) {
	counter_t v[bits];
	memset(v, 0, sizeof(counter_t)*bits);
	return save(filename, bits, sample, v, 1);
}

int counter_check_meta_file(char *filename, unsigned int *bits, unsigned int *sample) {
	FILE *fp;
	char line[2000], *key, *value;
	int ret = 0;
	int len;
	*bits = 0;
	*sample = 0;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		return -1;
	}
	fgets(line, sizeof(line), fp);
	if (strcmp(line, "SDM COUNTER\n")) {
		return -1;
	}
	while(fgets(line, sizeof(line), fp)) {
		len = strlen(line);
		assert(line[len-1] == '\n');
		line[len-1] = '\0';
		key = line;
		value = strchr(line, ':');
		value[0] = '\0';
		value++;
		while(isspace(*value)) value++;
		//printf("!! [%s] \"%s\"\n", key, value);

		if (!strcmp(key, "SDM-Version")) {
			// Check version.
			if (strcmp(value, SDM_VERSION_STR)) {
				ret = -2;
				goto exit;
			}
		} else if (!strcmp(key, "Format")) {
			// Check format.
			if (strcmp(value, "binary")) {
				ret = -3;
				goto exit;
			}
		} else if (!strcmp(key, "Order-of-bytes")) {
			// Check byte order.
			if (strcmp(value, (is_little_endian() ? "little-endian" : "big-endian"))) {
				ret = -4;
				goto exit;
			}
		} else if (!strcmp(key, "Bytes-per-counter")) {
			// Check bits per bitstring.
			if (sizeof(counter_t) != atoi(value)) {
				ret = -4;
				goto exit;
			}
		} else if (!strcmp(key, "Bits")) {
			// Check bits.
			*bits = atoi(value);
		} else if (!strcmp(key, "Sample")) {
			// Check sample.
			*sample = atoi(value);
		} else {
			// Unknown header.
			ret = -4;
			goto exit;
		}
	}
	if (bits == 0 || sample == 0) {
		ret = -5;
		goto exit;
	}
exit:
	fclose(fp);
	return ret;
}

int counter_init_file(char *filename, struct counter_s *this) {
	char meta[1000], bin[1000];
	unsigned int bits, sample;
	sprintf(meta, "%s.meta", filename);
	sprintf(bin, "%s.bin", filename);

	if(counter_check_meta_file(meta, &bits, &sample)) {
		return -1;
	}

	this->bits = bits;
	this->sample = sample;
	this->filename = filename;
	this->fd = open(bin, O_RDWR);
	if (this->fd == -1) {
		return -2;
	}
	this->data = (counter_t *) mmap(0, sizeof(counter_t) * this->bits * this->sample, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd, 0);
	if (this->data == MAP_FAILED) {
		close(this->fd);
		return -3;
	}
	if (_post_init(this)) {
		munmap(this->data, sizeof(counter_t) * this->bits * this->sample);
		close(this->fd);
		return -4;
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
	unsigned int i, cols = 20;
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
	unsigned int i;
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
#ifdef COUNTER_CHECK_OVERFLOW
		if (bs_get_bit(bs, i)) {
			if (ptr[i] < COUNTER_MAX) {
				ptr[i]++;
			}
		} else {
			if (ptr[i] > COUNTER_MIN) {
				ptr[i]--;
			}
		}
#else
		if (bs_get_bit(bs, i)) {
			ptr[i]++;
		} else {
			ptr[i]--;
		}
#endif
	}
	return 0;
}

int counter_add_bitstring_weighted(struct counter_s *this, unsigned int index, bitstring_t *bs, int weight) {
	unsigned int i;
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
#ifdef COUNTER_CHECK_OVERFLOW
		if (bs_get_bit(bs, i)) {
			if (ptr[i] <= COUNTER_MAX - weight) {
				ptr[i] += weight;
			}
		} else {
			if (ptr[i] >= COUNTER_MIN + weight) {
				ptr[i] -= weight;
			}
		}
#else
		if (bs_get_bit(bs, i)) {
			ptr[i] += weight;
		} else {
			ptr[i] -= weight;
		}
#endif
	}
	return 0;
}

int counter_sub_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs) {
	unsigned int i;
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
		if (bs_get_bit(bs, i)) {
			if (ptr[i] > 0) {
				ptr[i]--;
			}
		} else {
			if (ptr[i] < 0) {
				ptr[i]++;
			}
		}
	}
	return 0;
}

int counter_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2) {
	unsigned int i;
	counter_t *ptr1 = c1->counter[idx1];
	counter_t *ptr2 = c2->counter[idx2];
	for(i=0; i<c1->bits; i++) {
		// TODO Handle overflow.
		ptr1[i] += ptr2[i];
	}
	return 0;
}

int counter_weighted_add_counter(struct counter_s *c1, unsigned int idx1, struct counter_s *c2, unsigned int idx2, unsigned int weight) {
	unsigned int i;
	counter_t *ptr1 = c1->counter[idx1];
	counter_t *ptr2 = c2->counter[idx2];
	for(i=0; i<c1->bits; i++) {
		// TODO Handle overflow.
		ptr1[i] += weight * ptr2[i];
	}
	return 0;
}

int counter_to_bitstring(struct counter_s *this, unsigned int index, bitstring_t *bs) {
	unsigned int i;
	counter_t *ptr = this->counter[index];
	for(i=0; i<this->bits; i++) {
		if (ptr[i] > 0) {
			bs_set_bit(bs, i, 1);
		} else if (ptr[i] < 0) {
			bs_set_bit(bs, i, 0);
		} else {
			// Flip a coin! :)
			bs_set_bit(bs, i, arc4random() % 2);
		}
	}
	return 0;
}

void counter_reset(struct counter_s *this, unsigned int index) {
	counter_t *ptr = this->counter[index];
	memset(ptr, 0, sizeof(counter_t) * this->bits);
}

int counter_add_bitstring1(struct counter_s *this, unsigned int index, bitstring_t *bs) {
	/* This function is as slow as `counter_add_bitstring`, but it is more complex,
	 * hence error prone.
	 *
	 * If you plan to use it, comment the assert inside the for. It is only for debugging purposes
	 * and will degrade the performance because of the bs_get_bit call.
	 */
	unsigned int i;
	unsigned int idx;
	bitstring_t a;
	counter_t *ptr = this->counter[index];
	unsigned int offset = 0;
	bitstring_t *bs2 = bs;

	while(offset < this->bits) {
		a = *bs2;
		bs2++;
		for(i=0; i<8*sizeof(bitstring_t); i++) {
			idx = offset + 8*sizeof(bitstring_t) - 1 - i;
			if (idx > this->bits) {
				a >>= 1;
				continue;
			}
			assert((a&1) == bs_get_bit(bs, idx));
			if (a&1) {
				ptr[idx]++;
			} else {
				ptr[idx]--;
			}
			a >>= 1;
		}
		offset += 8*sizeof(bitstring_t);
	}
	return 0;
}
