
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "address_space.h"
#include "scanner_opencl.h"
#include "counter.h"

int main(void) {
	struct address_space_s as;
	bitstring_t *bs1;
	char buf[1000];
	struct opencl_scanner_s opencl;
	struct counter_s counter;
	int i, status;

	int bits = 1000;
	int sample = 1000000;

	status = as_init_from_b64_file(&as, "test1_address_space_b64.as");
	assert(status == 0);

	//assert(!as_init_random(&as, bits, sample));
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

	uint8_t selected_linear[sample];
	printf("@@ Linear %d\n", as_scan_linear(&as, bs1, 451, selected_linear));

	uint8_t selected_thread[sample];
	printf("@@ Thread %d\n", as_scan_thread(&as, bs1, 451, selected_thread, 4));

	opencl_scanner_init(&opencl, &as);
	uint8_t selected_opencl[sample];
	printf("@@ OpenCL %d\n", as_scan_opencl(&opencl, bs1, 451, selected_opencl));
	opencl_scanner_free(&opencl);

	return 0;

	for(i=0; i<sample; i++) {
		assert(selected_linear[i] == selected_thread[i]);
		assert(selected_linear[i] == selected_opencl[i]);
	}

	counter_init(&counter, bits, sample);
	//counter_init_file("test1_counter.bin", &counter, bits, sample);
	//printf("Before\n");
	//counter_print(&counter, 0);
	counter_add_bitstring(&counter, 0, bs1);
	//printf("\nAfter\n");
	counter_print(&counter, 0);
	counter_free(&counter);

	bs_free(bs1);
}

