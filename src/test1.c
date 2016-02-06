
#include <stdio.h>
#include <assert.h>
#include "address_space.h"
#include "scanner_opencl.h"
#include "counter.h"

int main(void) {
	struct address_space_s as;
	bitstring_t *bs1;
	char buf[1000];
	struct opencl_scanner_s opencl;
	struct counter_s counter;
	//int i;

	int bits = 1000;
	int sample = 1000000;

	assert(!as_init_random(&as, bits, sample));
	as_print_summary(&as);
	//as_print_addresses_b64(&as);
	
	bs_init_bitcount_table();

	bs1 = bs_alloc(as.bs_len);
	bs_init_random(bs1, as.bs_len, as.bs_bits_remaining);
	bs_to_hex(buf, bs1, as.bs_len);
	printf("bs1 = %s\n", buf);

	//for(i=0; i<8*as.bs_len*sizeof(bitstring_t); i++) {
	//	printf("%d", bs_get_bit(bs1, i));
	//}
	//printf("\n");

	as_scan_linear(&as, bs1, 451, NULL);
	as_scan_thread(&as, bs1, 451, NULL, 4);

	opencl_scanner_init(&opencl, &as);
	as_scan_opencl(&opencl, bs1, 451, NULL);

	//counter_init(&counter, bits, sample);
	counter_init_file("test1_counter.bin", &counter, bits, sample);
	//printf("Before\n");
	//counter_print(&counter, 0);
	counter_add_bitstring(&counter, 0, bs1);
	//printf("\nAfter\n");
	counter_print(&counter, 0);
	counter_free(&counter);

	bs_free(bs1);
}

