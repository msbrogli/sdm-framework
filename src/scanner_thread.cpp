
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include "scanner_thread.h"

void* scan_task(void *ptr);

ThreadScanner::ThreadScanner(AddressSpace *addresses, unsigned int thread_count) {
	this->addresses = addresses;
	this->thread_count = thread_count;
}

int ThreadScanner::scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const {
	pthread_t threads[this->thread_count];
	struct thread_params_t *params = new thread_params_t[this->thread_count];

	unsigned int qty, extra;
	qty = (this->addresses->sample)/this->thread_count;
	extra = (this->addresses->sample)%this->thread_count;

	unsigned int i, idx_begin, idx_end, len;
	for(i=0; i<this->thread_count; i++) {
		idx_begin = i*qty + std::min(i, extra);
		len = qty + (i < extra ? 1 : 0);
		idx_end = idx_begin + len;

		params[i].id = i;
		params[i].addresses = &this->addresses->addresses;
		params[i].address = bs;
		params[i].radius = radius;
		params[i].idx_begin = idx_begin;
		params[i].idx_end = idx_end;
		pthread_create(&threads[i], NULL, scan_task, (void*) &params[i]);
	}
	for(i=0; i<this->thread_count; i++) {
		pthread_join(threads[i], NULL);
	}
	for(i=0; i<this->thread_count; i++) {
		result->insert(result->end(), params[i].result.begin(), params[i].result.end());
	}

	delete [] params;
	return 0;
}

void* scan_task(void *ptr) {
	struct thread_params_t *params = (struct thread_params_t*) ptr;

	const unsigned int idx_end = params->idx_end;
	const unsigned int radius = params->radius;

	Bitstring *bs;
	for(unsigned int i=params->idx_begin; i<idx_end; i++) {
		bs = (*params->addresses)[i];
		if (bs->distance(params->address) <= radius) {
			params->result.push_back(bs);
		}
	}
	return NULL;
}

