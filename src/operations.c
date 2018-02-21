
#include <math.h>
#include <string.h>
#include "bitstring.h"
#include "address_space.h"
#include "counter.h"
#include "operations.h"

static
int _sdm_init(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter) {
	if (address_space->bits != counter->bits) {
		return -1;
	}
	if (address_space->sample != counter->sample) {
		return -2;
	}
	sdm->bits = address_space->bits;
	sdm->sample = address_space->sample;
	return 0;
}

int sdm_init_linear(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter) {
	int ret = _sdm_init(sdm, address_space, counter);
	if (ret) {
		return ret;
	}
	sdm->scanner_type = SDM_SCANNER_LINEAR;
	return 0;
}

int sdm_init_thread(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, unsigned int thread_count) {
	int ret = _sdm_init(sdm, address_space, counter);
	if (ret) {
		return ret;
	}
	sdm->scanner_type = SDM_SCANNER_THREAD;
	sdm->thread_count = thread_count;
	return 0;
}

#ifdef SDM_ENABLE_OPENCL
int sdm_init_opencl(struct sdm_s *sdm, struct address_space_s *address_space, struct counter_s *counter, char *opencl_source) {
	int ret = _sdm_init(sdm, address_space, counter);
	if (ret) {
		return ret;
	}
	sdm->scanner_type = SDM_SCANNER_OPENCL;
	sdm->opencl_opts = (struct opencl_scanner_s *) malloc(sizeof(struct opencl_scanner_s));
	as_scanner_opencl_init(sdm->opencl_opts, sdm->address_space, opencl_source);
	return 0;
}
#endif

void sdm_free(struct sdm_s *sdm) {
	switch(sdm->scanner_type) {
		case SDM_SCANNER_LINEAR:
		case SDM_SCANNER_THREAD:
			break;
#ifdef SDM_ENABLE_OPENCL
		case SDM_SCANNER_OPENCL:
			as_scanner_opencl_free(sdm->opencl_opts);
			free(sdm->opencl_opts);
			break;
#endif
	}
}

int sdm_iter_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, unsigned int max_iter, bitstring_t *output) {
	unsigned int i;
	bitstring_t *bs1, *bs2, *tmp;
	bs1 = bs_alloc(sdm->address_space->bs_len);
	bs2 = bs_alloc(sdm->address_space->bs_len);
	bs_copy(bs1, addr, sdm->address_space->bs_len);
	for(i=0; i<max_iter; i++) {
		if (sdm_read(sdm, bs1, radius, bs2) < 0) {
			i = -1;
			goto exit;
		}
		if (bs_distance(bs1, bs2, sdm->address_space->bs_len) == 0) {
			break;
		}
		tmp = bs1;
		bs1 = bs2;
		bs2 = tmp;
	}
	bs_copy(output, bs1, sdm->address_space->bs_len);
exit:
	bs_free(bs1);
	bs_free(bs2);
	return i;
}

int sdm_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output) {
	uint8_t selected[sdm->sample];
	struct counter_s counter;
	unsigned int i, cnt = 0;

	switch(sdm->scanner_type) {
		case SDM_SCANNER_LINEAR:
			as_scan_linear(sdm->address_space, addr, radius, selected);
			break;
		case SDM_SCANNER_THREAD:
			as_scan_thread(sdm->address_space, addr, radius, selected, sdm->thread_count);
			break;
#ifdef SDM_ENABLE_OPENCL
		case SDM_SCANNER_OPENCL:
			as_scan_opencl(sdm->opencl_opts, addr, radius, selected);
			break;
#endif
		default:
			return -1;
	}

	counter_init(&counter, sdm->bits, 1);
	for(i=0; i<sdm->sample; i++) {
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
	unsigned int i, cnt = 0;

	switch(sdm->scanner_type) {
		case SDM_SCANNER_LINEAR:
			as_scan_linear(sdm->address_space, addr, radius, selected);
			break;
		case SDM_SCANNER_THREAD:
			as_scan_thread(sdm->address_space, addr, radius, selected, sdm->thread_count);
			break;
#ifdef SDM_ENABLE_OPENCL
		case SDM_SCANNER_OPENCL:
			as_scan_opencl(sdm->opencl_opts, addr, radius, selected);
			break;
#endif
		default:
			return -1;
	}

	for(i=0; i<sdm->sample; i++) {
		if (selected[i] == 1) {
			counter_add_bitstring(sdm->counter, i, datum);
			cnt++;
		}
	}
	return cnt;
}

int sdm_generic_read(struct sdm_s *sdm, bitstring_t *addr, unsigned int radius, bitstring_t *output, double z) {
	uint8_t selected[sdm->sample];
	double counter[sdm->bits];
	counter_t *ptr;
	double sign;
	unsigned int i, j, cnt = 0;

	switch(sdm->scanner_type) {
		case SDM_SCANNER_LINEAR:
			as_scan_linear(sdm->address_space, addr, radius, selected);
			break;
		case SDM_SCANNER_THREAD:
			as_scan_thread(sdm->address_space, addr, radius, selected, sdm->thread_count);
			break;
#ifdef SDM_ENABLE_OPENCL
		case SDM_SCANNER_OPENCL:
			as_scan_opencl(sdm->opencl_opts, addr, radius, selected);
			break;
#endif
		default:
			return -1;
	}

	memset(counter, 0, sizeof(counter));
	for(i=0; i<sdm->sample; i++) {
		if (selected[i] == 1) {
			ptr = sdm->counter->counter[i];
			for(j=0; j<sdm->bits; j++) {
				sign = 1;
				if (ptr[j] < 0) {
					sign = -1;
				}
				counter[j] += sign * pow(fabs(ptr[j]), z);
			}
			cnt++;
		}
	}
	for(j=0; j<sdm->bits; j++) {
		if (counter[j] > 0) {
			bs_set_bit(output, j, 1);
		} else if (counter[j] < 0) {
			bs_set_bit(output, j, 0);
		} else {
			bs_set_bit(output, j, arc4random() % 2);
		}
	}

	return cnt;
}
