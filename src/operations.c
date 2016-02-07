
#include "bitstring.h"
#include "address_space.h"
#include "counter.h"
#include "operations.h"

int sdm_iter_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, unsigned int max_iter, bitstring_t *output) {
	int i;
	bitstring_t *bs1, *bs2, *tmp;
	bs1 = bs_alloc(sdm->address_space->bs_len);
	bs2 = bs_alloc(sdm->address_space->bs_len);
	bs_copy(bs1, addr, sdm->address_space->bs_len);
	for(i=0; i<max_iter; i++) {
		sdm_read(sdm, bs1, radius, bs2);
		if (bs_distance(bs1, bs2, sdm->address_space->bs_len) == 0) {
			break;
		}
		tmp = bs1;
		bs1 = bs2;
		bs2 = tmp;
	}
	bs_copy(output, bs1, sdm->address_space->bs_len);
	bs_free(bs1);
	bs_free(bs2);
	return i;
}

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

