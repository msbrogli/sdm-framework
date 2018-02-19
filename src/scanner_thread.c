
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "bitstring.h"
#include "address_space.h"

/* Prevent the double evaluation problem. */
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	   _a < _b ? _a : _b; })

struct thread_params_t {
	int id;
	const bitstring_t* bs;
	const struct address_space_s *address_space;
	uint8_t *selected;
	int cnt;
	unsigned int radius;
	unsigned int idx_begin;
	unsigned int idx_end;
};

static
void* scan_task(void *ptr);

int as_scan_thread(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, uint8_t *selected, unsigned int thread_count) {
	pthread_t threads[thread_count];
	struct thread_params_t *params;
	unsigned int qty, extra;
	unsigned int i, idx_begin, idx_end, len;
	int cnt = 0;

	params =  (struct thread_params_t *) malloc(sizeof(struct thread_params_t) * thread_count);
	
	qty = (this->sample)/thread_count;
	extra = (this->sample)%thread_count;

	for(i=0; i<thread_count; i++) {
		idx_begin = i*qty + min(i, extra);
		len = qty + (i < extra ? 1 : 0);
		idx_end = idx_begin + len;

		params[i].id = i;
		params[i].address_space = this;
		params[i].bs = bs;
		params[i].radius = radius;
		params[i].idx_begin = idx_begin;
		params[i].idx_end = idx_end;
		params[i].selected = selected;
		params[i].cnt = 0;
		pthread_create(&threads[i], NULL, scan_task, (void*) &params[i]);
	}
	for(i=0; i<thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
	for(i=0; i<thread_count; i++) {
		cnt += params[i].cnt;
	}

	free(params);
	return cnt;
}

static
void* scan_task(void *ptr) {
	unsigned int i;
	uint8_t x;
	struct thread_params_t *params = (struct thread_params_t*) ptr;

	const unsigned int idx_end = params->idx_end;
	const unsigned int radius = params->radius;

	for(i=params->idx_begin; i<idx_end; i++) {
		x = params->selected[i] = (bs_distance(params->bs, params->address_space->addresses[i], params->address_space->bs_len) <= radius);
		if (x) {
			params->cnt++;
		}
		/*
		if (bs_distance(params->bs, params->address_space->addresses[i], params->address_space->bs_len) <= radius) {
			params->cnt++;
			if (params->selected) {
				params->selected[i] = 1;
			}
		} else {
			if (params->selected) {
				params->selected[i] = 0;
			}
		}
		*/
	}
	return NULL;
}

