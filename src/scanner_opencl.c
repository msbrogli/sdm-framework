
#include <stdio.h>
#include <assert.h>
#include <OpenCL/cl.h>

#include "scanner_opencl.h"

// Links:
// - https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html
// - https://code.google.com/p/simple-opencl/

int opencl_scanner_init(struct opencl_scanner_s *this, struct address_space_s *as) {
	this->address_space = as;

	// =============================
	// Set local and group worksize.
	// =============================
	this->local_worksize = 0;

	if (this->local_worksize == 0) {
		this->global_worksize = this->address_space->sample;
	} else {
		this->global_worksize = this->address_space->sample/this->local_worksize;
		if (this->address_space->sample%this->local_worksize > 0) {
			this->global_worksize++;
		}
		this->global_worksize *= this->local_worksize;
	}

	// Choose the right kernel.
	if (this->global_worksize >= this->address_space->sample) {
		this->kernel_name = "single_scan";
	} else {
		this->kernel_name = "scan";
	}

	cl_int error;

	// ==============
	// Create context.
	// ==============
	this->context = clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &error);
	assert(error == CL_SUCCESS);

	// =================
	// Select device_id.
	// =================
	size_t deviceBufferSize;
	error = clGetContextInfo(this->context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	assert(error == CL_SUCCESS);

	cl_device_id *devices = (cl_device_id *) malloc(deviceBufferSize);
	error = clGetContextInfo(this->context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	assert(error == CL_SUCCESS);
	free(devices);

	this->device_id = devices[0];

	// =============
	// Create queue.
	// =============
	this->queue = clCreateCommandQueue(this->context, this->device_id, 0, &error);
	assert(error == CL_SUCCESS);

	// =======================
	// Read and build program.
	// =======================
	const int MAX_SOURCE_SIZE = 1<<20;
	char *source_str = (char *)malloc(sizeof(char)*MAX_SOURCE_SIZE);
	size_t source_size;
	FILE *fp = fopen("scanner_opencl.cl", "r");
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	this->program = clCreateProgramWithSource(this->context, 1, (const char **)&source_str, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clBuildProgram(this->program, 1, &this->device_id, NULL, NULL, NULL);

	// Print build log.
	size_t log_size;
	clGetProgramBuildInfo(this->program, this->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	char *log = (char *) malloc(log_size);
	clGetProgramBuildInfo(this->program, this->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	printf("=== LOG\n%s\n=== END\n", log);
	free(log);

	assert(error == CL_SUCCESS);
	free(source_str);

	// ========================
	// Generate bitcount_table.
	// ========================
	for(int i=0; i<(1<<16); i++) {
		int a = i, d = 0;
		while(a) {
			if (a&1) d++;
			a >>= 1;
		}
		this->bitcount_table[i] = d;
	}
	this->bitcount_table_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(this->bitcount_table), NULL, &error);
	assert(error == CL_SUCCESS);
	error = clEnqueueWriteBuffer(this->queue, this->bitcount_table_buf, CL_FALSE, 0, sizeof(this->bitcount_table), this->bitcount_table, 0, NULL, NULL);
	assert(error == CL_SUCCESS);

	// ====================
	// Generate bitstrings.
	// ====================
	this->bs_len = this->address_space->bs_len;
	assert(sizeof(cl_bitstring_t) == sizeof(bitstring_t));
	size_t bs_size = sizeof(cl_bitstring_t)*this->bs_len*this->address_space->sample;
	this->bitstrings_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, bs_size, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clEnqueueWriteBuffer(this->queue, this->bitstrings_buf, CL_FALSE, 0, bs_size, this->address_space->bs_data, 0, NULL, NULL);
	assert(error == CL_SUCCESS);

	// =========================
	// Allocating other buffers.
	// =========================
	this->bs_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(cl_bitstring_t)*this->bs_len, NULL, &error);
	assert(error == CL_SUCCESS);
	this->selected_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uchar)*this->address_space->sample, NULL, &error);
	assert(error == CL_SUCCESS);
	this->counter_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uint), NULL, &error);
	assert(error == CL_SUCCESS);
	this->selected = (cl_uchar *)malloc(sizeof(cl_uchar)*this->address_space->sample);

	error = clFinish(this->queue);
	assert(error == CL_SUCCESS);

	return 0;
}

void opencl_scanner_free(struct opencl_scanner_s *this) {
	free(this->selected);

	clReleaseMemObject(this->bitcount_table_buf);
	clReleaseMemObject(this->bitstrings_buf);
	clReleaseMemObject(this->bs_buf);
	clReleaseMemObject(this->selected_buf);
	clReleaseMemObject(this->counter_buf);
	clReleaseProgram(this->program);
	clReleaseCommandQueue(this->queue);
	clReleaseContext(this->context);
}

void opencl_scanner_devices(struct opencl_scanner_s *this) {
	int i, j;
	char* value;
	size_t valueSize;
	cl_uint platformCount;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_uint maxComputeUnits;

	// get all platforms
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);

	for (i = 0; i < platformCount; i++) {

		// get all devices
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
		devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

		// for each device print critical attributes
		for (j = 0; j < deviceCount; j++) {

			// print device name
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
			printf("%d. Device: %s\n", j+1, value);
			free(value);

			// print hardware device version
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
			printf(" %d.%d Hardware version: %s\n", j+1, 1, value);
			free(value);

			// print software driver version
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
			printf(" %d.%d Software version: %s\n", j+1, 2, value);
			free(value);

			// print c version supported by compiler for device
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
			value = (char*) malloc(valueSize);
			clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
			printf(" %d.%d OpenCL C version: %s\n", j+1, 3, value);
			free(value);

			// print parallel compute units
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
					sizeof(maxComputeUnits), &maxComputeUnits, NULL);
			printf(" %d.%d Parallel compute units: %d\n", j+1, 4, maxComputeUnits);

		}

		free(devices);

	}

	free(platforms);
}

int as_scan_opencl(struct opencl_scanner_s *this, bitstring_t *bs, unsigned int radius, void *result) {
	cl_int error;

	// Create kernel.
	cl_kernel kernel = clCreateKernel(this->program, this->kernel_name, &error);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clCreateKernel");

	// Set arg0: bitcount_table
	error = clSetKernelArg(kernel, 0, sizeof(this->bitcount_table_buf), &this->bitcount_table_buf);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg0:bitcount_table");

	// Set arg1: bitstrings
	error = clSetKernelArg(kernel, 1, sizeof(this->bitstrings_buf), &this->bitstrings_buf);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg1:bitstrings");

	// Set arg2: bs_len
	error = clSetKernelArg(kernel, 2, sizeof(this->bs_len), &this->bs_len);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg2:bs_len");

	// Set arg3: sample
	error = clSetKernelArg(kernel, 3, sizeof(this->address_space->sample), &this->address_space->sample);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg3:sample");

	// Set arg4: global_worksize
	error = clSetKernelArg(kernel, 4, sizeof(this->global_worksize), &this->global_worksize);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg4:global_worksize");

	// Set arg5: bs
	error = clEnqueueWriteBuffer(this->queue, this->bs_buf, CL_FALSE, 0, sizeof(cl_bitstring_t)*this->bs_len, bs, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clEnqueueWriteBuffer:bs");
	error = clSetKernelArg(kernel, 5, sizeof(this->bs_buf), &this->bs_buf);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg5:bs");

	// Set arg6: radius
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 6, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg6:radius");

	// Set arg8: counter
	error = clSetKernelArg(kernel, 7, sizeof(this->counter_buf), &this->counter_buf);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg7:counter_buf");

	// Set arg8: selected
	error = clSetKernelArg(kernel, 8, sizeof(this->selected_buf), &this->selected_buf);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clSetKernelArg8:selected");

	// Wait until all queue is done.
	//error = clFinish(queue);
	//assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clFinish (before running)");

	// Run kernel.
	if (this->local_worksize > 0) {
		error = clEnqueueNDRangeKernel(this->queue, kernel, 1, NULL, &this->global_worksize, &this->local_worksize, 0, NULL, NULL);
	} else {
		error = clEnqueueNDRangeKernel(this->queue, kernel, 1, NULL, &this->global_worksize, NULL, 0, NULL, NULL);
	}
	if (error != CL_SUCCESS) {
		printf("error code = %d\n", error);
	}
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clEnqueueNDRangeKernel");

	// Wait until all queue is done.
	//error = clFinish(queue);
	//assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clFinish (after running)");

	// Read selected bitstring indexes.
	cl_uint counter;
	error = clEnqueueReadBuffer(this->queue, this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
	//time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");

	// Read selected bitstring indexes.
	error = clEnqueueReadBuffer(this->queue, this->selected_buf, CL_FALSE, 0, sizeof(cl_uchar)*this->address_space->sample, this->selected, 0, NULL, NULL);
	//time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");

	// Wait until all queue is done.
	error = clFinish(this->queue);
	assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clFinish (after reading)");

	// Fill result vector.
	int cnt = 0;
	for(int i=0; i<this->address_space->sample; i++) {
		if (this->selected[i]) {
			cnt++;
			//result->push_back(this->addresses->addresses[i]);
		}
	}
	clReleaseKernel(kernel);
	//time->mark("OpenCLScanner::scan Filling result vector");
	
	printf("@@ OpenCL %d\n", cnt);

	return 0;
}
