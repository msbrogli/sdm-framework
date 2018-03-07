
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "address_space.h"
#include "scanner_opencl.h"
#include "counter.h"

int main(void) {
	struct address_space_s as;
	bitstring_t *bs1;
	struct opencl_scanner_s opencl;
	char buf[10000];
	int i;

	//int bits = 10000;
	//int sample = 1000000;
	//int radius = 4845;

	int bits = 1000;
	int sample = 1000000;
	int radius = 451;

	//int status = as_init_from_b64_file(&as, "test1_address_space_b64.as");
	//assert(status == 0);

	assert(!as_init_random(&as, bits, sample));
	//as_save_b64_file(&as, "test1_address_space_b64.as");

	as_print_summary(&as);
	
	bs_init_bitcount_table();

	bs1 = bs_alloc(as.bs_len);
	bs_init_random(bs1, as.bs_len, as.bs_bits_remaining);
	bs_to_hex(buf, bs1, as.bs_len);
	printf("bs1 = %s\n", buf);

	//for(i=0; i<8*as.bs_len*sizeof(bitstring_t); i++) {
	//	printf("%d", bs_get_bit(bs1, i));
	//}
	//printf("\n");

	as_scanner_opencl_init(&opencl, &as, "scanner_opencl2.cl");
	unsigned int selected_opencl[sample];
	for (i=0; i<10000; i++) {
		as_scan_opencl2(&opencl, bs1, radius, selected_opencl);
	}
	printf("Done.\n");
	as_scanner_opencl_free(&opencl);

	bs_free(bs1);
	return 0;
}

