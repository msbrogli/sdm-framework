
#include "bitstring.h"
#include "address_space.h"
#include "counter.h"
#include "operations.h"

int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output) {
	uint8_t selected[sdm->sample];
	struct counter_s counter;
	int cnt = 0;

	as_scan_linear(sdm->address_space, addr, radius, selected);

	counter_init(&counter, sdm->bits, 1);
	for(int i=0; i<sdm->sample; i++) {
		if (selected[i] == 1) {
			counter_add_counter(&counter, 0, sdm->counter, i);
			cnt++;
		}
	}
	counter_to_bitstring(&counter, 0, output);
	counter_free(&counter);

	return cnt;
}

int sdm_write(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *datum) {
	uint8_t selected[sdm->sample];
	int cnt = 0;
	as_scan_linear(sdm->address_space, addr, radius, selected);
	for(int i=0; i<sdm->sample; i++) {
		if (selected[i] == 1) {
			counter_add_bitstring(sdm->counter, i, datum);
			cnt++;
		}
	}
	return cnt;
}

