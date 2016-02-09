
#ifndef SDM_ADDRESS_SPACE_H
#define SDM_ADDRESS_SPACE_H

#include "bitstring.h"

struct address_space_s {
	/* SDM dimension */
	unsigned int bits;

	/* Number of hard-locations. */
	unsigned int sample;

	/*
	This approach allocates a continuous chunk of memory for all bitstring addresses.
	The `addresses` allows the use of array notation: addresses[0], addresses[1], ...

	Let `a` be `addresses`. Then:

	          a[0]   a[1]   a[2]   a[3]   a[4]
			  |      |      |      |      |
			  v      v      v      v      v
	bs_data = xxxxxx|xxxxxx|xxxxxx|xxxxxx|xxxxxx
	*/
	bitstring_t **addresses;

	unsigned int bs_len;
	unsigned int bs_bits_remaining;
	bitstring_t *bs_data;
};

int as_free(struct address_space_s *this);
int as_init(struct address_space_s *this, unsigned int bits, unsigned int sample);
int as_init_random(struct address_space_s *this, unsigned int bits, unsigned int sample);
int as_init_from_b64_file(struct address_space_s *this, char *filename);
void as_print_summary(struct address_space_s *this);
void as_print_addresses_b64(struct address_space_s *this);
void as_print_addresses_hex(struct address_space_s *this);
int as_scan_linear(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *buf);

int as_scan_thread(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *buf, unsigned int thread_count);

int as_save_b64_file(const struct address_space_s *this, char *filename);

#endif
