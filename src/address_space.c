
#include <stdio.h>
#include <stdlib.h>
#include "bitstring.h"
#include "address_space.h"

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
	int i;
	char buf[1000];
	for (i=0; i < this->sample; i++) {
		bs_to_hex(buf, this->addresses[i], this->bs_len);
		printf("[%8d] %s\n", i, buf);
	}
}

void as_print_addresses_b64(struct address_space_s *this) {
	int i;
	char buf[1000];
	for (i=0; i < this->sample; i++) {
		bs_to_b64(buf, this->addresses[i], this->bs_len);
		printf("[%8d] %s\n", i, buf);
	}
}

int as_init(struct address_space_s *this, unsigned int bits, unsigned int sample) {
	int i;

	this->bits = bits;
	this->sample = sample;

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

	return 0;
}

int as_init_random(struct address_space_s *this, unsigned int bits, unsigned int sample) {
	int i;
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
	return 0;
}

int as_scan_linear(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *selected) {
	int cnt = 0;
	for(int i=0; i<this->sample; i++) {
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
	}
	return cnt;
}

/*
int AddressSpace::save(std::string filename) const {
	std::ofstream file(filename);
	file << "SDM:ADDRESS SPACE\nSDM-Version: v0.0.1\nFormat: base64-v1\n";
	file << "Bits: " << this->bits << "\nSample: " << this->sample << "\nHash: " << this->hash() << "\n\n";
	const unsigned int n = this->sample;
	for(int i=0; i<n; i++) {
		Bitstring *addr = this->addresses[i];
		// It does not depend on whether the computer is big-endian or little-endian. [msbrogli 2015-12-01]
		file << addr->base64() << "\n";
	}
	file.close();
	return 0;
}
*/

