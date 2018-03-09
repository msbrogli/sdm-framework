
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

/*
 * TODO Multiple device support.
 *
 * Links:
 * - https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html
 * - https://code.google.com/p/simple-opencl/
 */

int as_scanner_opencl_init(struct opencl_scanner_s *this, struct address_space_s *as, char *opencl_source) {
	cl_int error;
	size_t deviceBufferSize;
	unsigned int i;

	this->address_space = as;
	this->opencl_source = opencl_source;
	this->verbose = 0;

	/* ==============
	 * Create context.
	 * ==============
	 */
	// query the number of platforms
	cl_uint numPlatforms;
	error = clGetPlatformIDs(0, NULL, &numPlatforms);
	assert(error == CL_SUCCESS);

	// now get all the platform IDs
	cl_platform_id platforms[numPlatforms];
	error = clGetPlatformIDs(numPlatforms, platforms, NULL);
	assert(error == CL_SUCCESS);

	// set platform property - we just pick the first one
	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (int) platforms[0], 0};
	this->context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, NULL, NULL, &error);
	assert(error == CL_SUCCESS);

	/* =================
	 * Select device_id.
	 * =================
	 */
	error = clGetContextInfo(this->context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	assert(error == CL_SUCCESS);

	this->devices = (cl_device_id *) malloc(deviceBufferSize);
	error = clGetContextInfo(this->context, CL_CONTEXT_DEVICES, deviceBufferSize, this->devices, NULL);
	assert(error == CL_SUCCESS);
	this->devices_count = deviceBufferSize / sizeof(cl_device_id);
	if (this->address_space->verbose) {
		printf("OpenCL platforms: %d devices: %d\n", numPlatforms, this->devices_count);
	}

	/* =============
	 * Create queue.
	 * =============
	 */
	cl_uint max_compute_units;
	this->queues = (cl_command_queue *) malloc(sizeof(cl_command_queue) * this->devices_count);
	for (i=0; i<this->devices_count; i++) {
		// TODO Handle multiple devices with different values.
		clGetDeviceInfo(this->devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(max_compute_units), &max_compute_units, NULL);
		this->max_compute_units = max_compute_units;
		this->queues[i] = clCreateCommandQueue(this->context, this->devices[i], 0, &error);
		assert(error == CL_SUCCESS);
	}


	/* =============================
	 * Set local and group worksize.
	 * =============================
	 */
	// Local worksize is a multiple of 16.
	/*
	this->local_worksize = this->address_space->bs_len / 16;
	if (this->address_space->bs_len % 16 != 0) {
		this->local_worksize++;
	}
	this->local_worksize *= 16;
	*/

	// Local worksize is a power-of-2.
	this->local_worksize = 1;
	while (this->local_worksize < this->address_space->bs_len) {
		this->local_worksize <<= 1;
	}
	//this->global_worksize = 2 * 16 * this->local_worksize * this->max_compute_units;

	{
		// Each workgroup will calculate around 20 bitstring distances.
		const size_t step = 2 * this->local_worksize * this->max_compute_units;
		const size_t target = this->address_space->sample / 20;
		this->global_worksize = target / step;
		if (target % step != 0) {
			this->global_worksize++;
		}
		this->global_worksize *= step;
	}
	this->kernel_name = "single_scan5_unroll";
	//if (this->address_space->bs_len == 16) {
	//	this->kernel_name = "single_scan3_16";
	//}

	if (this->address_space->verbose) {
		printf("OpenCL Max compute units=%u Default kernel=%s Local worksize=%zu  Global worksize=%zu\n", this->max_compute_units, this->kernel_name, this->local_worksize, this->global_worksize);
	}


	/* =======================
	 * Read and build program.
	 * =======================
	 */
	const int MAX_SOURCE_SIZE = 1<<20;
	char *source_str = (char *)malloc(sizeof(char)*MAX_SOURCE_SIZE);
	size_t source_size;
	FILE *fp = fopen(this->opencl_source, "r");
	if (fp == NULL) {
		printf("ERROR OpenCL source code not found (file=%s).\n", this->opencl_source);
	}
	assert(fp != NULL);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	this->program = clCreateProgramWithSource(this->context, 1, (const char **)&source_str, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clBuildProgram(this->program, this->devices_count, this->devices, NULL, NULL, NULL);

	/* Print build log. */
	size_t log_size;
	clGetProgramBuildInfo(this->program, this->devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	char *log = (char *) malloc(log_size);
	clGetProgramBuildInfo(this->program, this->devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	if (strlen(log) > 0) {
		printf("=== LOG (%lu bytes)\n%s\n=== END\n", strlen(log), log);
	}
	free(log);

	assert(error == CL_SUCCESS);
	free(source_str);

	/* ========================
	 * Generate bitcount_table.
	 * ========================
	 */
	for(i=0; i<(1<<16); i++) {
		int a = i, d = 0;
		while(a) {
			if (a&1) d++;
			a >>= 1;
		}
		this->bitcount_table[i] = d;
	}
	this->bitcount_table_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(this->bitcount_table), NULL, &error);
	assert(error == CL_SUCCESS);
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->bitcount_table_buf, CL_FALSE, 0, sizeof(this->bitcount_table), this->bitcount_table, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}

	/* ====================
	 * Generate bitstrings.
	 * ====================
	 */
	this->bs_len = this->address_space->bs_len;
	assert(sizeof(cl_bitstring_t) == sizeof(bitstring_t));
	size_t bs_size = sizeof(cl_bitstring_t)*this->bs_len*this->address_space->sample;
	this->bitstrings_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, bs_size, NULL, &error);
	assert(error == CL_SUCCESS);
	for(i=0; i<this->devices_count; i++) {
		error = clEnqueueWriteBuffer(this->queues[i], this->bitstrings_buf, CL_FALSE, 0, bs_size, this->address_space->bs_data, 0, NULL, NULL);
		assert(error == CL_SUCCESS);
	}

	/* =========================
	 * Allocating other buffers.
	 * =========================
	 */
	this->bs_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(cl_bitstring_t)*this->bs_len, NULL, &error);
	assert(error == CL_SUCCESS);
	this->selected_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uint)*this->address_space->sample, NULL, &error);
	assert(error == CL_SUCCESS);
	this->counter_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uint), NULL, &error);
	assert(error == CL_SUCCESS);
	assert(sizeof(cl_uchar) == sizeof(uint8_t));

	for(i=0; i<this->devices_count; i++) {
		error = clFinish(this->queues[i]);
		assert(error == CL_SUCCESS);
	}

	return 0;
}

void as_scanner_opencl_free(struct opencl_scanner_s *this) {
	unsigned int i;
	clReleaseMemObject(this->bitcount_table_buf);
	clReleaseMemObject(this->bitstrings_buf);
	clReleaseMemObject(this->bs_buf);
	clReleaseMemObject(this->selected_buf);
	clReleaseMemObject(this->counter_buf);
	clReleaseProgram(this->program);
	for(i=0; i<this->devices_count; i++) {
		clReleaseCommandQueue(this->queues[i]);
	}
	clReleaseContext(this->context);
	free(this->queues);
	free(this->devices);
}

void opencl_scanner_devices(struct opencl_scanner_s *this) {
	unsigned int i, j;
	char* value;
	size_t valueSize;
	cl_uint platformCount;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;
	cl_int error;

	/* get all platforms */
	error = clGetPlatformIDs(0, NULL, &platformCount);
	assert(error == CL_SUCCESS);

	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	error = clGetPlatformIDs(platformCount, platforms, NULL);
	assert(error == CL_SUCCESS);

	for (i = 0; i < platformCount; i++) {

		/* get all devices */
		error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
		assert(error == CL_SUCCESS);

		devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
		error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
		assert(error == CL_SUCCESS);

		printf("Platform %d\n", i+1);

		/* for each device print critical attributes */
		for (j = 0; j < deviceCount; j++) {

			/* print device name */
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
			printf("%d. Device: %s\n", j+1, value);
			free(value);

			/* print hardware device version */
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
			printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
			free(value);

			/* print software driver version */
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
			printf(" %d.%d Software version: %s\n", j+1, 2, value);
			free(value);

			/* print c version supported by compiler for device */
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
			printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
			free(value);

			/* print parallel compute units */
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
					sizeof(maxComputeUnits), &maxComputeUnits, NULL);
			printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);

		}

		free(devices);

	}

	free(platforms);
}

int as_scan_opencl(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, uint8_t *selected) {
	// You must use as_scan_opencl2.
	assert(0);

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
	error = clSetKernelArg(kernel, 6, sizeof(this->counter_buf), &this->counter_buf);
	assert(error == CL_SUCCESS);

	/* Set arg7: selected */
	error = clSetKernelArg(kernel, 7, sizeof(this->selected_buf), &this->selected_buf);
	assert(error == CL_SUCCESS);

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
	//cl_uint counter;
	//error = clEnqueueReadBuffer(this->queue, this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);

	/* Read selected bitstring indexes. */
	error = clEnqueueReadBuffer(this->queues[0], this->selected_buf, CL_FALSE, 0, sizeof(cl_uchar)*this->address_space->sample, selected, 0, NULL, NULL);
	/*time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");*/

	/* Wait until all queue is done. */
	error = clFinish(this->queues[0]);
	assert(error == CL_SUCCESS);
	//gettimeofday(&t1, NULL);
	//printf("==> clEnqueueReadBuffer %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);

	/* Fill result vector. */
	cnt = 0;
	for(i=0; i<this->address_space->sample; i++) {
		if (selected[i]) {
			cnt++;
		}
	}
	//gettimeofday(&t1, NULL);
	//printf("==> Processing results %f ms\n", t1.tv_sec*1000.0 + t1.tv_usec/1000.0 - t0.tv_sec*1000.0 - t0.tv_usec/1000.0);
	//gettimeofday(&t0, NULL);
	
	clReleaseKernel(kernel);

	//printf("\n");

	return cnt;
}
