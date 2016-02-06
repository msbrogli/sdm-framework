
#include <stdio.h>
#include <assert.h>
#include "address_space.h"

int main(void) {
	struct address_space_s as;
	bitstring_t *bs1;
	char buf[1000];

	assert(!as_init_random(&as, 1000, 1000000));
	as_print_summary(&as);
	//as_print_addresses_b64(&as);
	
	bs_init_bitcount_table();

	bs1 = bs_alloc(as.bs_len);
	bs_init_random(bs1, as.bs_len, as.bs_bits_remaining);
	bs_to_hex(buf, bs1, as.bs_len);
	printf("bs1 = %s\n", buf);
	as_scan_linear(&as, bs1, 451, NULL);
	as_scan_thread(&as, bs1, 451, NULL, 4);
}

