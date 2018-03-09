
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

	/* Create kernel. */
	if (this->verbose) {
		printf("OpenCL Scan with kernel %s (local_worksize: %zu  global_worksize: %zu)...\n", this->kernel_name, this->local_worksize, this->global_worksize);
	}
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

	/* Set arg4: bs */
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->bs_buf, CL_FALSE, 0, sizeof(cl_bitstring_t)*this->bs_len, bs, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}
	error = clSetKernelArg(kernel, 4, sizeof(this->bs_buf), &this->bs_buf);
	assert(error == CL_SUCCESS);

	/* Set arg5: radius */
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 5, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);

	/* Set arg6: counter */
	cl_uint counter = 0;
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}
	error = clSetKernelArg(kernel, 6, sizeof(this->counter_buf), &this->counter_buf);
	assert(error == CL_SUCCESS);

	/* Set arg7: selected */
	error = clSetKernelArg(kernel, 7, sizeof(this->selected_buf), &this->selected_buf);
	assert(error == CL_SUCCESS);

	/* Set arg8: partial_dist */
	if (this->local_worksize > 0) {
		error = clSetKernelArg(kernel, 8, sizeof(cl_uint) * this->local_worksize, 0);
		assert(error == CL_SUCCESS);
	} else {
		// I have tried passing a NULL pointer, but it did not work.
		error = clSetKernelArg(kernel, 8, sizeof(cl_uint), 0);
		assert(error == CL_SUCCESS);
	}

	/* Run kernel. */
	size_t qty = this->global_worksize / this->devices_count;
	size_t extra = this->global_worksize % this->devices_count;
	for(i=0; i<this->devices_count; i++) {
		size_t offset = i * qty + min(i, extra);
		size_t worksize = qty + (i < extra ? 1 : 0);
		if (this->local_worksize > 0) {
			error = clEnqueueNDRangeKernel(this->queues[i], kernel, 1, &offset, &worksize, &this->local_worksize, 0, NULL, NULL);
		} else {
			error = clEnqueueNDRangeKernel(this->queues[i], kernel, 1, &offset, &worksize, NULL, 0, NULL, NULL);
		}
		if (error != CL_SUCCESS) {
			printf("error code = %d\n", error);
		}
		assert(error == CL_SUCCESS);
	}

	/* Wait until all queue is done. */
	for(i=0; i<this->devices_count; i++) {
		error = clFinish(this->queues[i]);
		assert(error == CL_SUCCESS);
	}

	/* Read counter. */
	error = clEnqueueReadBuffer(this->queues[0], this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
	error = clFinish(this->queues[0]);
	assert(error == CL_SUCCESS);

	/* Read selected bitstring indexes. */
	if (counter > 0) {
		error = clEnqueueReadBuffer(this->queues[0], this->selected_buf, CL_FALSE, 0, sizeof(cl_uint)*counter, selected, 0, NULL, NULL);
		error = clFinish(this->queues[0]);
		assert(error == CL_SUCCESS);
	}

	/* Fill result vector. */
	cnt = counter;

	clReleaseKernel(kernel);
	return cnt;
}
