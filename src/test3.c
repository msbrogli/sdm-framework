
#include <stdio.h>
#include "sdm.h"

int main(void) {
	bitstring_t *bs1, *bs2;
	int bs_len = 16;
	char buf[1000];

	bs1 = bs_alloc(bs_len);
	bs2 = bs_alloc(bs_len);

	bs_init_random(bs1, bs_len, 0);
	bs_to_b64(buf, bs1, bs_len);
	printf("@@ %s\n", buf);
	printf("\n");

	bs_init_b64(bs2, buf);
	bs_to_b64(buf, bs2, bs_len);
	printf("@@ %s\n", buf);

	return 0;
}
