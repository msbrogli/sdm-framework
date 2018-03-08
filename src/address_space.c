
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assert.h"
#include "bitstring.h"
#include "address_space.h"
#include "utils.h"
#include "version.h"

void as_print_summary(struct address_space_s *this) {
	double alloc = this->bs_len * sizeof(bitstring_t) * this->sample / 1024.0;
	char unit = 'k';
	if (alloc > 10000) {
		alloc /= 1024.0;
		unit = 'M';
	}
	printf("Dimension: %d\n", this->bits);
	printf("Number of hardlocations: %d\n", this->sample);
	printf("Allocated memory for addresses: %.1f %cB\n", alloc, unit);
	printf("Allocated bits per bitstring: %ld\n", this->bs_len * 8 * sizeof(bitstring_t));
	printf("Remaining bits per bitstring: %d\n", this->bs_bits_remaining);
	printf("Memory efficiency: %.1f%%\n", (100.0 * this->bits) / this->bs_len / 8 / sizeof(bitstring_t));
}

void as_print_addresses_hex(struct address_space_s *this) {
	unsigned int i;
	char buf[1000];
	for (i=0; i < this->sample; i++) {
		bs_to_hex(buf, this->addresses[i], this->bs_len);
		printf("[%8d] %s\n", i, buf);
	}
}

void as_print_addresses_b64(struct address_space_s *this) {
	unsigned int i;
	char buf[1000];
	for (i=0; i < this->sample; i++) {
		bs_to_b64(buf, this->addresses[i], this->bs_len);
		printf("[%8d] %s\n", i, buf);
	}
}

int as_init(struct address_space_s *this, unsigned int bits, unsigned int sample) {
	unsigned int i;

	this->bits = bits;
	this->sample = sample;
	this->verbose = 1;

	this->bs_len = bits / 8 / sizeof(bitstring_t);
	if (this->bs_len * 8 * sizeof(bitstring_t) < bits) {
		this->bs_len++;
	}
	this->bs_bits_remaining = this->bs_len * 8 * sizeof(bitstring_t) - bits;

	this->bs_data = (bitstring_t *) malloc(sizeof(bitstring_t) * this->bs_len * this->sample);
	if (this->bs_data == NULL) {
		return -1;
	}

	this->addresses = (bitstring_t **) malloc(sizeof(bitstring_t*) * this->sample);
	if (this->addresses == NULL) {
		free(this->bs_data);
		return -2;
	}
	for (i=0; i<this->sample; i++) {
		this->addresses[i] = &this->bs_data[i * this->bs_len];
	}

#ifdef SDM_ENABLE_OPENCL
	this->opencl_opts = (struct opencl_scanner_s *) malloc(sizeof(struct opencl_scanner_s));
	if (this->opencl_opts == NULL) {
		free(this->bs_data);
		free(this->addresses);
		return -3;
	}
#endif

	return 0;
}

int as_init_random(struct address_space_s *this, unsigned int bits, unsigned int sample) {
	unsigned int i;
	if (as_init(this, bits, sample)) {
		return -1;
	}
	for(i=0; i < this->sample; i++) {
		bs_init_random(this->addresses[i], this->bs_len, this->bs_bits_remaining);
	}
	return 0;
}

int as_free(struct address_space_s *this) {
	free(this->addresses);
	free(this->bs_data);
#ifdef SDM_ENABLE_OPENCL
	free(this->opencl_opts);
#endif
	return 0;
}

int as_scan_linear(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *selected) {
	unsigned int i, cnt;
	cnt = 0;
	for(i=0; i<this->sample; i++) {
		selected[i] = (bs_distance(this->addresses[i], bs, this->bs_len) <= radius);
		cnt += selected[i];
		/*
		if (bs_distance(this->addresses[i], bs, this->bs_len) <= radius) {
			cnt++;
			if (selected) {
				selected[i] = 1;
			}
		} else {
			if (selected) {
				selected[i] = 0;
			}
		}
		*/
	}
	return cnt;
}

int as_scan_linear2(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, unsigned int *selected) {
	unsigned int i, cnt;
	cnt = 0;
	for(i=0; i<this->sample; i++) {
		if (bs_distance(this->addresses[i], bs, this->bs_len) <= radius) {
			selected[cnt++] = i;
		}
	}
	return cnt;
}

void as_reset_address(struct address_space_s *this, unsigned int index) {
	bitstring_t *bs = this->addresses[index];
	bs_init_random(bs, this->bs_len, this->bs_bits_remaining);
}

int as_save_b64_file(const struct address_space_s *this, char *filename) {
	unsigned int i;
	char buf[1000];
	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		return -1;
	}
	fprintf(fp, "SDM ADDRESS SPACE\n");
	fprintf(fp, "SDM-Version: " SDM_VERSION_STR "\n");
	fprintf(fp, "Format: base64\n");
	fprintf(fp, "Order-of-bytes: %s\n", (is_little_endian() ? "little-endian" : "big-endian"));
	fprintf(fp, "Bits-per-Bitstring: %lu\n", this->bs_len * 8 * sizeof(bitstring_t));
	fprintf(fp, "Bits: %d\n", this->bits);
	fprintf(fp, "Sample: %d\n", this->sample);
	fprintf(fp, "\n");
	for(i=0; i<this->sample; i++) {
		bs_to_b64(buf, this->addresses[i], this->bs_len);
		fprintf(fp, "%s\n", buf);
	}
	fclose(fp);
	return 0;
}

int as_init_from_b64_file(struct address_space_s *this, char *filename) {
	FILE *fp;
	char line[2000], *key, *value;
	int ret = 0;
	unsigned int len, cnt;
	unsigned int bits = 0, sample = 0, bits_per_bitstring = 0;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		return -1;
	}
	fgets(line, sizeof(line), fp);
	if (strcmp(line, "SDM ADDRESS SPACE\n")) {
		return -1;
	}
	while(fgets(line, sizeof(line), fp)) {
		len = strlen(line);
		if (len <= 1) {
			break;
		}
		assert(line[len-1] == '\n');
		line[len-1] = '\0';
		key = line;
		value = strchr(line, ':');
		value[0] = '\0';
		value++;
		while(isspace(*value)) value++;
		/*printf("!! [%s] \"%s\"\n", key, value);*/

		if (!strcmp(key, "SDM-Version")) {
			/* Check version. */
			if (strcmp(value, SDM_VERSION_STR)) {
				ret = -2;
				goto exit;
			}
		} else if (!strcmp(key, "Format")) {
			/* Check format. */
			if (strcmp(value, "base64")) {
				ret = -3;
				goto exit;
			}
		} else if (!strcmp(key, "Order-of-bytes")) {
			/* Check byte order. */
			if (strcmp(value, (is_little_endian() ? "little-endian" : "big-endian"))) {
				ret = -4;
				goto exit;
			}
		} else if (!strcmp(key, "Bits-per-Bitstring")) {
			/* Check bits per bitstring. */
			bits_per_bitstring = atoi(value);
		} else if (!strcmp(key, "Bits")) {
			/* Check bits. */
			bits = atoi(value);
		} else if (!strcmp(key, "Sample")) {
			/* Check sample. */
			sample = atoi(value);
		} else {
			/* Unknown header. */
			ret = -5;
			goto exit;
		}
	}
	if (bits == 0 || sample == 0 || bits_per_bitstring == 0) {
		ret = -6;
		goto exit;
	}
	if (bits_per_bitstring % (8 * sizeof(bitstring_t)) != 0) {
		ret = -7;
		goto exit;
	}
	if (as_init(this, bits, sample)) {
		ret = -8;
		goto exit;
	}

	/* Read address space (this->bs_data). */
	cnt = 0;
	while(fgets(line, sizeof(line), fp)) {
		bs_init_b64(this->addresses[cnt], line);
		cnt++;
	}
	if (cnt != sample) {
		as_free(this);
		ret = -9;
		goto exit;
	}

exit:
	fclose(fp);
	return ret;
}

