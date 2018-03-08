
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "address_space.h"
#include "scanner_opencl.h"
#include "counter.h"

void run(unsigned int bits, unsigned int sample, unsigned int radius, int verbose) {
	struct address_space_s as;
	bitstring_t *bs1;
	char buf[10000];
	struct opencl_scanner_s opencl;
	unsigned int i;

	assert(!as_init_random(&as, bits, sample));
	if (verbose) as_print_summary(&as);

	as.verbose = verbose;
	
	bs_init_bitcount_table();

	bs1 = bs_alloc(as.bs_len);
	bs_init_random(bs1, as.bs_len, as.bs_bits_remaining);
	bs_to_hex(buf, bs1, as.bs_len);
	if (verbose) printf("bs1 = %s\n", buf);

	//for(i=0; i<8*as.bs_len*sizeof(bitstring_t); i++) {
	//	printf("%d", bs_get_bit(bs1, i));
	//}
	//printf("\n");

	uint8_t selected1[sample];
	uint8_t selected2[sample];

	unsigned int len_linear = as_scan_linear(&as, bs1, radius, selected1);
	if (verbose) printf("@@ Linear %d\n", len_linear);

	memset(selected2, 0, sizeof(selected2));
	unsigned int len_thread = as_scan_thread(&as, bs1, radius, selected2, 4);
	if (verbose) printf("@@ Thread %d\n", len_thread);
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

	unsigned int *selected_int = (unsigned int *) malloc(sizeof(unsigned int)*sample);

	memset(selected_int, 0, sizeof(unsigned int)*sample);
	unsigned int len_linear2 = as_scan_linear2(&as, bs1, radius, selected_int);
	if (verbose) printf("@@ Linear2 %d\n", len_linear2);
	for(i=0; i<len_linear2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}
	assert(len_linear2 == len_linear);

	memset(selected_int, 0, sizeof(unsigned int)*sample);
	unsigned int len_thread2 = as_scan_thread2(&as, bs1, radius, selected_int, 4);
	if (verbose) printf("@@ Thread2 %d\n", len_thread2);
	for(i=0; i<len_thread2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}
	assert(len_thread2 == len_linear);


	memset(selected_int, 0, sizeof(unsigned int)*sample);
	as_scanner_opencl_init(&opencl, &as, "scanner_opencl2.cl");
	unsigned int len_opencl2 = as_scan_opencl2(&opencl, bs1, radius, selected_int);
	if (verbose) printf("@@ OpenCL2 %d\n", len_opencl2);
	as_scanner_opencl_free(&opencl);
	for(i=0; i<len_opencl2; i++) {
		assert(selected1[selected_int[i]] == 1);
	}
	assert(len_opencl2 == len_linear);

	bs_free(bs1);
	free(selected_int);
}

int main(void) {
	int i, n;

	run(256, 1000000, 103, 1);
	printf("\n\n");
	run(1000, 1000000, 451, 1);
	printf("\n\n");
	run(10000, 1000000, 4845, 1);
	printf("\n\n");

	n = 20;
	printf("Running %d tests for 256 bits... ", n);
	for (i=0; i<n; i++) {
		run(256, 1000000, 103, 0);
		printf("!");
		fflush(stdout);
	}
	printf("\n");

	n = 20;
	printf("Running %d tests for 1,000 bits... ", n);
	for (i=0; i<n; i++) {
		run(1000, 1000000, 451, 0);
		printf("!");
		fflush(stdout);
	}
	printf("\n");

	n = 3;
	printf("Running %d tests for 10,000 bits... ", n);
	for (i=0; i<n; i++) {
		run(10000, 1000000, 4845, 0);
		printf("!");
		fflush(stdout);
	}
	printf("\n");
}

