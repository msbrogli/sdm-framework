
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "address_space.h"
#include "scanner_opencl.h"
#include "counter.h"

int main(void) {
	struct address_space_s as;
	bitstring_t *bs1;
	char buf[10000];
	struct opencl_scanner_s opencl;
	unsigned int i;

	//unsigned int bits = 10000;
	//unsigned int sample = 1000000;
	//unsigned int radius = 4845;

	unsigned int bits = 1000;
	unsigned int sample = 1000000;
	unsigned int radius = 451;

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

	uint8_t selected1[sample];
	uint8_t selected2[sample];

	printf("@@ Linear %d\n", as_scan_linear(&as, bs1, radius, selected1));

	memset(selected2, 0, sizeof(selected2));
	printf("@@ Thread %d\n", as_scan_thread(&as, bs1, radius, selected2, 4));
	for(i=0; i<sample; i++) {
		assert(selected1[i] == selected2[i]);
	}

	/*
	memset(selected2, 0, sizeof(selected2));
	as_scanner_opencl_init(&opencl, &as, "scanner_opencl.cl");
	printf("@@ OpenCL %d\n", as_scan_opencl(&opencl, bs1, radius, selected2));
	as_scanner_opencl_free(&opencl);
	for(i=0; i<sample; i++) {
		assert(selected1[i] == selected2[i]);
	}
	*/

	unsigned int selected_int[sample];

	memset(selected_int, 0, sizeof(selected_int));
	unsigned int len_linear2 = as_scan_linear2(&as, bs1, radius, selected_int);
	printf("@@ Linear2 %d\n", len_linear2);
	for(i=0; i<len_linear2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}

	memset(selected_int, 0, sizeof(selected_int));
	unsigned int len_thread2 = as_scan_thread2(&as, bs1, radius, selected_int, 4);
	printf("@@ Thread2 %d\n", len_thread2);
	for(i=0; i<len_thread2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}

	memset(selected_int, 0, sizeof(selected_int));
	as_scanner_opencl_init(&opencl, &as, "scanner_opencl2.cl");
	unsigned int len_opencl2 = as_scan_opencl2(&opencl, bs1, radius, selected_int);
	printf("@@ OpenCL2 %d\n", len_opencl2);
	as_scanner_opencl_free(&opencl);
	for(i=0; i<len_opencl2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}

	bs_free(bs1);
}

