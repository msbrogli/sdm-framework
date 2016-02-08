
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sdm.h>

int test_bitstring(unsigned int bs_len, unsigned int remaining_bits) {
	bitstring_t *bs1, *bs2, *bs3;
	unsigned int bits, flips;
	char buf1[1000], buf2[1000];
	int i, j;

	bits = sizeof(bitstring_t) * 8 * bs_len - remaining_bits;
	bs_init_bitcount_table();
	printf("Testing bs_* functions [bs_len=%d, bits=%d]...\n", bs_len, bits);

	bs1 = bs_alloc(bs_len);
	bs2 = bs_alloc(bs_len);
	bs3 = bs_alloc(bs_len);

	bs_init_ones(bs1, bs_len, remaining_bits);
	bs_copy(bs2, bs1, bs_len);
	assert(bs_distance(bs1, bs2, bs_len) == 0);
	for(i=0; i<bits; i++) {
		assert(bs_get_bit(bs1, i) == 1);
		assert(bs_get_bit(bs2, i) == 1);
	}
	bs_to_hex(buf1, bs1, bs_len);
	for(i=0; i<(bs_len-1)*2*sizeof(bitstring_t); i++) {
		assert(buf1[i] == 'f');
	}
	for(i=0; i<bits; i++) {
		bs_flip_bit(bs1, i);
	}
	assert(bs_distance(bs1, bs2, bs_len) == bits);

	bs_init_random(bs1, bs_len, remaining_bits);
	bs_copy(bs2, bs1, bs_len);
	assert(bs_distance(bs1, bs2, bs_len) == 0);
	for(i=0; i<bits; i++) {
		assert(bs_get_bit(bs1, i) == bs_get_bit(bs2, i));
	}
	for(i=0; i<bits; i++) {
		bs_flip_bit(bs1, i);
	}
	bs_to_hex(buf1, bs1, bs_len);
	bs_to_hex(buf2, bs2, bs_len);
	assert(bs_distance(bs1, bs2, bs_len) == bits);

	bs_copy(bs2, bs1, bs_len);
	assert(bs_distance(bs1, bs2, bs_len) == 0);
	flips = bits/4;
	j = bs_flip_random_bits(bs1, bits, flips);
	assert(bs_distance(bs1, bs2, bs_len) == flips);

	bs_to_b64(buf1, bs1, bs_len);
	bs_init_b64(bs2, buf1);
	assert(bs_distance(bs1, bs2, bs_len) == 0);

	return 0;
}

int main(void) {
	test_bitstring(26, 20);
	test_bitstring(25, 40);
	test_bitstring(24, 0);
	test_bitstring(1, 0);
	test_bitstring(1, 30);
	return 0;
}
