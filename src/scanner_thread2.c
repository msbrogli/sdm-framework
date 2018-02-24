
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
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
	unsigned int *selected;
	unsigned int cnt;
	unsigned int radius;
	unsigned int idx_begin;
	unsigned int idx_end;
};

static
void* scan_task(void *ptr);

int as_scan_thread2(const struct address_space_s *this, const bitstring_t *bs, unsigned int radius, unsigned int *selected, unsigned int thread_count) {
	pthread_t threads[thread_count];
	struct thread_params_t *params;
	unsigned int qty, extra;
	unsigned int i, j, idx_begin, idx_end, len;
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
		params[i].selected = (unsigned int *) malloc(sizeof(unsigned int) * this->sample);
		assert(params[i].selected != NULL);
		params[i].cnt = 0;
		pthread_create(&threads[i], NULL, scan_task, (void*) &params[i]);
	}
	for(i=0; i<thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
	for(i=0; i<thread_count; i++) {
		for(j=0; j<params[i].cnt; j++) {
			selected[cnt++] = params[i].selected[j];
		}
		free(params[i].selected);
	}

	free(params);
	return cnt;
}

static
void* scan_task(void *ptr) {
	unsigned int i;
	struct thread_params_t *params = (struct thread_params_t*) ptr;

	const unsigned int idx_end = params->idx_end;
	const unsigned int radius = params->radius;

	for(i=params->idx_begin; i<idx_end; i++) {
		if (bs_distance(params->bs, params->address_space->addresses[i], params->address_space->bs_len) <= radius) {
			params->selected[params->cnt++] = i;
		}
	}
	return NULL;
}

