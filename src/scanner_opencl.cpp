
#include <iostream>
#include <cassert>
#include <OpenCL/cl.h>

#include "scanner_opencl.h"

// Links:
// - https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html
// - https://code.google.com/p/simple-opencl/

// TODO Create a destructor with clReleaseContext(this->context), free(this->bitstrings), clReleaseMemObject([buffers]), ...

OpenCLScanner::OpenCLScanner(AddressSpace *addresses) {
	this->addresses = addresses;

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

	cl_device_id *devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
	error = clGetContextInfo(this->context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	assert(error == CL_SUCCESS);
	delete [] devices;

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
	error = clBuildProgram(this->program, 1, &device_id, NULL, NULL, NULL);

	// Print build log.
	size_t log_size;
	clGetProgramBuildInfo(program, this->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	char *log = (char *) malloc(log_size);
	clGetProgramBuildInfo(program, this->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	std::cout << log << std::endl;
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

	// =================
	// Calculate bs_len.
	// =================
	this->bs_len = this->addresses->bits / 64;
	if (this->addresses->bits % 64 > 0) {
		this->bs_len++;
	}

	// ====================
	// Generate bitstrings.
	// ====================
	size_t bs_size = sizeof(cl_ulong)*this->bs_len*this->addresses->sample;
	this->bitstrings = (cl_ulong*) malloc(bs_size);
	int k = 0;
	for(int i=0; i<this->addresses->sample; i++) {
		Bitstring *bs = this->addresses->addresses[i];
		assert(bs->len == this->bs_len);
		assert(sizeof(this->bitstrings[0]) == sizeof(bs->data[0]));
		for(int j=0; j<bs->len; j++) {
			this->bitstrings[k++] = bs->data[j];
		}
	}
	this->bitstrings_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, bs_size, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clEnqueueWriteBuffer(this->queue, this->bitstrings_buf, CL_FALSE, 0, bs_size, this->bitstrings, 0, NULL, NULL);
	assert(error == CL_SUCCESS);

	// =========================
	// Allocating other buffers.
	// =========================
	this->bs_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(cl_ulong)*this->bs_len, NULL, &error);
	assert(error == CL_SUCCESS);
	this->selected_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uint)*this->addresses->sample, NULL, &error);
	assert(error == CL_SUCCESS);
}

void OpenCLScanner::devices() const {
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

int OpenCLScanner::scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result) const {
	TimeMeasure *time = new TimeMeasure;
	int ret = this->scan(bs, radius, result, time);
	delete time;
	return ret;
}

int OpenCLScanner::scan(const Bitstring *bs, unsigned int radius, std::vector<Bitstring *> *result, TimeMeasure *time) const {
	cl_int error;

	// Create kernel.
	cl_kernel kernel = clCreateKernel(program, "scan", &error);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clCreateKernel");

	// Set arg0: bitcount_table
	error = clSetKernelArg(kernel, 0, sizeof(this->bitcount_table_buf), &this->bitcount_table_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg0:bitcount_table");

	// Set arg1: bitstrings
	error = clSetKernelArg(kernel, 1, sizeof(this->bitstrings_buf), &this->bitstrings_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg1:bitstrings");

	// Set arg2: bs_len
	error = clSetKernelArg(kernel, 2, sizeof(this->bs_len), &this->bs_len);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg2:bs_len");

	// Set arg3: sample
	error = clSetKernelArg(kernel, 3, sizeof(this->addresses->sample), &this->addresses->sample);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg3:sample");

	// Set arg4: worksize
	size_t worksize = 500;
	error = clSetKernelArg(kernel, 4, sizeof(worksize), &worksize);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg4:worksize");

	// Set arg5: bs
	error = clEnqueueWriteBuffer(this->queue, this->bs_buf, CL_FALSE, 0, sizeof(cl_ulong)*this->bs_len, bs->data, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clEnqueueWriteBuffer:worksize");
	error = clSetKernelArg(kernel, 5, sizeof(this->bs_buf), &this->bs_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg5:bs");

	// Set arg6: radius
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 6, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg6:radius");

	// Set arg7: counter
	cl_uint counter;
	cl_mem counter_buf;
	counter_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(counter), NULL, &error);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clCreateBuffer:counter_buf");
	error = clSetKernelArg(kernel, 7, sizeof(counter_buf), &counter_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg7:counter");

	// Set arg8: selected
	error = clSetKernelArg(kernel, 8, sizeof(selected_buf), &selected_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg8:selected");

	// Run kernel.
	error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &worksize, &worksize, 0, NULL, NULL);
	if (error != CL_SUCCESS) {
		std::cout << "error code = " << error << std::endl;
	}
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clEnqueueNDRangeKernel");

	// Read counter.
	error = clEnqueueReadBuffer(this->queue, counter_buf, CL_FALSE, 0, sizeof(counter), &counter, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clEnqueueReadBuffer:counter");

	// Read selected bitstring indexes.
	cl_uint *selected = (cl_uint *)malloc(sizeof(cl_uint)*this->addresses->sample);
	error = clEnqueueReadBuffer(this->queue, selected_buf, CL_FALSE, 0, sizeof(cl_uint)*this->addresses->sample, selected, 0, NULL, NULL);
	time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");

	// Wait until all queue is done.
	error = clFinish(queue);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clFinish");

	// Release counter buffer.
	error = clReleaseMemObject(counter_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clReleaseMemObject:counter_buf");

	// Fill result vector.
	assert(counter <= this->addresses->sample);
	unsigned int idx;
	for(int i=0; i<counter; i++) {
		idx = selected[i];
		result->push_back(this->addresses->addresses[idx]);
	}
	free(selected);
	time->mark("OpenCLScanner::scan Filling result vector");

	return 0;
}
