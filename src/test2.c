
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "sdm.h"

int main(void) {
	struct sdm_s *sdm;
	struct address_space_s *as;
	struct counter_s *counter;
	bitstring_t *bs1, *bs2;
	char buf[1000];

	int bits = 1000;
	int sample = 1000000;

	as = (struct address_space_s *) malloc(sizeof(struct address_space_s));
	counter = (struct counter_s *) malloc(sizeof(struct counter_s));
	sdm = (struct sdm_s *) malloc(sizeof(struct sdm_s));

	assert(!as_init_random(as, bits, sample));
	as_print_summary(as);
	
	//assert(!counter_init(counter, bits, sample));
	counter_init_file("test2_counter.bin", counter);
	assert(counter->bits == bits);
	assert(counter->sample == sample);

	bs_init_bitcount_table();
	bs1 = bs_alloc(as->bs_len);
	bs2 = bs_alloc(as->bs_len);
	bs_init_random(bs1, as->bs_len, as->bs_bits_remaining);
	bs_to_hex(buf, bs1, as->bs_len);
	printf("bs1 = %s\n", buf);

	sdm->bits = bits;
	sdm->sample = sample;
	sdm->address_space = as;
	sdm->counter = counter;

	sdm_write(sdm, bs1, 451, bs1);
	sdm_read(sdm, bs1, 451, bs2);
	bs_to_hex(buf, bs2, as->bs_len);
	printf("bs2 = %s\n", buf);

	bs_free(bs1);
	bs_free(bs2);
	counter_free(counter);
	free(counter);
	free(as);
}

