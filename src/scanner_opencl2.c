
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "scanner_opencl.h"

/* Prevent the double evaluation problem. */
#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	   _a < _b ? _a : _b; })


int as_scan_opencl2(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, unsigned int *selected) {
	unsigned int i, cnt;
	cl_int error;
	//struct timeval t0, t1;

	//gettimeofday(&t0, NULL);

	/* Create kernel. */
	cl_kernel kernel = clCreateKernel(this->program, this->kernel_name, &error);
	assert(error == CL_SUCCESS);

	/* Set arg0: bitcount_table */
	error = clSetKernelArg(kernel, 0, sizeof(this->bitcount_table_buf), &this->bitcount_table_buf);
	assert(error == CL_SUCCESS);

	/* Set arg1: bitstrings */
	error = clSetKernelArg(kernel, 1, sizeof(this->bitstrings_buf), &this->bitstrings_buf);
	assert(error == CL_SUCCESS);

	/* Set arg2: bs_len */
	error = clSetKernelArg(kernel, 2, sizeof(this->bs_len), &this->bs_len);
	assert(error == CL_SUCCESS);

	/* Set arg3: sample */
	error = clSetKernelArg(kernel, 3, sizeof(this->address_space->sample), &this->address_space->sample);
	assert(error == CL_SUCCESS);

	/* Set arg4: global_worksize */
	//error = clSetKernelArg(kernel, 4, sizeof(this->global_worksize), &this->global_worksize);
//printf("@@ error = %d\n", error);
	//assert(error == CL_SUCCESS);

	/* Set arg5: bs */
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->bs_buf, CL_FALSE, 0, sizeof(cl_bitstring_t)*this->bs_len, bs, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}
	error = clSetKernelArg(kernel, 4, sizeof(this->bs_buf), &this->bs_buf);
	assert(error == CL_SUCCESS);

	/* Set arg6: radius */
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 5, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);

	/* Set arg8: counter */
	cl_uint counter = 0;
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}
	error = clSetKernelArg(kernel, 6, sizeof(this->counter_buf), &this->counter_buf);
	assert(error == CL_SUCCESS);

	/* Set arg8: selected */
	error = clSetKernelArg(kernel, 7, sizeof(this->selected_buf), &this->selected_buf);
	assert(error == CL_SUCCESS);

	/* Wait until all queue is done. */
	//error = clFinish(this->queue);
	//assert(error == CL_SUCCESS);
	//gettimeofday(&t1, NULL);
	//printf("==> clEnqueueWriteBuffer %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);

	/* Run kernel. */
	size_t qty = this->global_worksize / this->devices_count;
	size_t extra = this->global_worksize % this->devices_count;
	for(i=0; i<this->devices_count; i++) {
		size_t offset = i * qty + min(i, extra);
		size_t worksize = qty + (i < extra ? 1 : 0);
		if (this->local_worksize > 0) {
			error = clEnqueueNDRangeKernel(this->queues[i], kernel, 1, &offset, &worksize, &this->local_worksize, 0, NULL, NULL);
			assert(0);
		} else {
			error = clEnqueueNDRangeKernel(this->queues[i], kernel, 1, &offset, &worksize, NULL, 0, NULL, NULL);
		}
		if (error != CL_SUCCESS) {
			printf("error code = %d\n", error);
		}
		assert(error == CL_SUCCESS);
	}
	/*time->mark("OpenCLScanner::scan clEnqueueNDRangeKernel");*/

	/* Wait until all queue is done. */
	for(i=0; i<this->devices_count; i++) {
		error = clFinish(this->queues[i]);
		assert(error == CL_SUCCESS);
	}
	//gettimeofday(&t1, NULL);
	//printf("==> clEnqueueNDRangeKernel %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);

	/* Read counter. */
	error = clEnqueueReadBuffer(this->queues[0], this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
	error = clFinish(this->queues[0]);
	assert(error == CL_SUCCESS);

	/* Read selected bitstring indexes. */
	error = clEnqueueReadBuffer(this->queues[0], this->selected_buf, CL_FALSE, 0, sizeof(cl_uint)*counter, selected, 0, NULL, NULL);
	error = clFinish(this->queues[0]);
	assert(error == CL_SUCCESS);

	//gettimeofday(&t1, NULL);
	//printf("==> clEnqueueReadBuffer %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);

	/* Fill result vector. */
	cnt = counter;

	/*
	cnt = 0;
	for(i=0; i<this->address_space->sample; i++) {
		if (selected[i]) {
			cnt++;
		}
	}
	*/
	//gettimeofday(&t1, NULL);
	//printf("==> Processing results %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);
	
	clReleaseKernel(kernel);

	//printf("\n");

	return cnt;
}
