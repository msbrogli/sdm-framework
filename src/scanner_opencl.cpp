
#include <iostream>
#include <cassert>
#include <OpenCL/cl.h>

#include "scanner_opencl.h"

// Links:
// - https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html
// - https://code.google.com/p/simple-opencl/

OpenCLScanner::OpenCLScanner(AddressSpace *addresses) {
	this->addresses = addresses;

	cl_int error;

	// =================
	// Calculate bs_len.
	// =================
	this->bs_len = this->addresses->bits / 64;
	if (this->addresses->bits % 64 > 0) {
		this->bs_len++;
	}


	// =============================
	// Set local and group worksize.
	// =============================
	this->local_worksize = 0;

	if (this->local_worksize == 0) {
		this->global_worksize = this->addresses->sample;
	} else {
		this->global_worksize = this->addresses->sample/this->local_worksize;
		if (this->addresses->sample%this->local_worksize > 0) {
			this->global_worksize++;
		}
		this->global_worksize *= this->local_worksize;
	}

	// ================
	// Set kernel name.
	// ================
	this->kernel_name = "single_scan";

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

	// ==============================
	// Split work between work-items.
	// ==============================
	cl_ulong local_mem_size;
	clGetDeviceInfo(this->device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &local_mem_size, 0);

	size_t max_work_group_size[1];
	clGetDeviceInfo(this->device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), max_work_group_size, 0);

	uint bitstrings_by_workgroup = std::min(local_mem_size / bs_len, (unsigned long long)max_work_group_size[0] / bs_len);
	this->local_worksize = bitstrings_by_workgroup * bs_len;

	uint sample_adjusted = this->addresses->sample / bitstrings_by_workgroup;
	if (this->addresses->sample % bitstrings_by_workgroup > 0) {
		sample_adjusted++;
	}
	sample_adjusted *= bitstrings_by_workgroup;
	this->global_worksize = sample_adjusted * bs_len;

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

	// ====================
	// Generate bitstrings.
	// ====================
	size_t bs_size = sizeof(cl_ulong)*this->bs_len*this->addresses->sample;
	this->bitstrings = (cl_ulong*) malloc(bs_size);
	int k = 0;
	for(int i=0; i<this->addresses->sample; i++) {
		Bitstring *bs = this->addresses->addresses[i];
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
	this->selected_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uchar)*this->addresses->sample, NULL, &error);
	assert(error == CL_SUCCESS);
	this->counter_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(cl_uint), NULL, &error);
	assert(error == CL_SUCCESS);
	this->selected = (cl_uchar *)malloc(sizeof(cl_uchar)*this->addresses->sample);

	error = clFinish(queue);
	assert(error == CL_SUCCESS);
}

OpenCLScanner::~OpenCLScanner() {
	free(this->bitstrings);
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
	cl_kernel kernel = clCreateKernel(program, this->kernel_name.c_str(), &error);
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

	// Set arg4: bs
	error = clEnqueueWriteBuffer(this->queue, this->bs_buf, CL_FALSE, 0, sizeof(cl_ulong)*this->bs_len, bs->data, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clEnqueueWriteBuffer:bs");
	error = clSetKernelArg(kernel, 4, sizeof(this->bs_buf), &this->bs_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg5:bs");

	// Set arg5: radius
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 5, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg6:radius");

	// Set arg6: counter
	cl_uint counter = 0;
	error = clEnqueueWriteBuffer(this->queue, this->counter_buf, CL_FALSE, 0, sizeof(counter), &counter, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	error = clSetKernelArg(kernel, 6, sizeof(this->counter_buf), &this->counter_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg7:counter_buf");

	// Set arg7: selected
	error = clSetKernelArg(kernel, 7, sizeof(this->selected_buf), &this->selected_buf);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clSetKernelArg8:selected");

	// Set arg8: __local dist
	error = clSetKernelArg(kernel, 8, sizeof(cl_uint)*this->bs_len, NULL);
	assert(error == CL_SUCCESS);

	// Wait until all queue is done.
	//error = clFinish(queue);
	//assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clFinish (before running)");

	// Run kernel.
	if (this->local_worksize > 0) {
		error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &this->global_worksize, &this->local_worksize, 0, NULL, NULL);
	} else {
		error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &this->global_worksize, NULL, 0, NULL, NULL);
	}
	if (error != CL_SUCCESS) {
		std::cout << "error code = " << error << std::endl;
	}
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clEnqueueNDRangeKernel");

	// Wait until all queue is done.
	//error = clFinish(queue);
	//assert(error == CL_SUCCESS);
	//time->mark("OpenCLScanner::scan clFinish (after running)");

	// Read counter.
	error = clEnqueueReadBuffer(this->queue, this->counter_buf, CL_FALSE, 0, sizeof(cl_uint), &counter, 0, NULL, NULL);
	time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");

	// Read selected bitstring indexes.
	error = clEnqueueReadBuffer(this->queue, this->selected_buf, CL_FALSE, 0, sizeof(cl_uchar)*this->addresses->sample, this->selected, 0, NULL, NULL);
	time->mark("OpenCLScanner::scan clEnqueueReadBuffer:selected");

	// Wait until all queue is done.
	error = clFinish(queue);
	assert(error == CL_SUCCESS);
	time->mark("OpenCLScanner::scan clFinish (after reading)");

	// Fill result vector.
	unsigned int idx;
	std::cout << "@@ counter=" << counter << std::endl;
	for(int i=0; i<this->addresses->sample; i++) {
		if (selected[i]) {
			result->push_back(this->addresses->addresses[i]);
		}
	}
	clReleaseKernel(kernel);
	time->mark("OpenCLScanner::scan Filling result vector");

	return 0;
}
