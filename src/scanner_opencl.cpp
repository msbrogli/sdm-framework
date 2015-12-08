
#include <iostream>
#include <cassert>
#include <OpenCL/cl.h>

#include "scanner_opencl.h"

// Links:
// - https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/scalarDataTypes.html
// - https://code.google.com/p/simple-opencl/

// TODO Create a destructor with clReleaseContext(this->context), free(this->bitstrings).

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
	const int MAX_SOURCE_SIZE = 1<<10;
	char *source_str = (char *)malloc(sizeof(char)*MAX_SOURCE_SIZE);
	size_t source_size;
	FILE *fp = fopen("scanner_opencl.cl", "r");
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	source_str[source_size] = '\0';
	fclose(fp);

	this->program = clCreateProgramWithSource(this->context, 1, (const char **)&source_str, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clBuildProgram(this->program, 1, &device_id, NULL, NULL, NULL);
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
	cl_int error;

	// Create kernel.
	cl_kernel kernel = clCreateKernel(program, "scan", &error);
	assert(error == CL_SUCCESS);

	// Set arg0: bitcount_table
	error = clSetKernelArg(kernel, 0, sizeof(this->bitcount_table_buf), &this->bitcount_table_buf);
	assert(error == CL_SUCCESS);

	// Set arg1: bitstrings
	error = clSetKernelArg(kernel, 1, sizeof(this->bitstrings_buf), &this->bitstrings_buf);
	assert(error == CL_SUCCESS);

	// Set arg2: bs_len
	error = clSetKernelArg(kernel, 2, sizeof(this->bs_len), &this->bs_len);
	assert(error == CL_SUCCESS);

	// Set arg3: sample
	error = clSetKernelArg(kernel, 3, sizeof(this->addresses->sample), &this->addresses->sample);
	assert(error == CL_SUCCESS);

	// Set arg4: bs
	cl_mem bs_buf = clCreateBuffer(this->context, CL_MEM_READ_ONLY, sizeof(cl_ulong)*this->bs_len, NULL, &error);
	assert(error == CL_SUCCESS);
	error = clEnqueueWriteBuffer(this->queue, bs_buf, CL_FALSE, 0, sizeof(cl_ulong)*this->bs_len, bs->data, 0, NULL, NULL);
	assert(error == CL_SUCCESS);
	error = clSetKernelArg(kernel, 4, sizeof(bs_buf), &bs_buf);
	assert(error == CL_SUCCESS);

	// Set arg5: radius
	cl_uint arg_radius = radius;
	error = clSetKernelArg(kernel, 5, sizeof(arg_radius), &arg_radius);
	assert(error == CL_SUCCESS);

	// Seg arg6: output
	cl_uint output;
	cl_mem output_buf;
	output_buf = clCreateBuffer(this->context, CL_MEM_WRITE_ONLY, sizeof(output), NULL, &error);
	assert(error == CL_SUCCESS);
	error = clSetKernelArg(kernel, 6, sizeof(output_buf), &output_buf);

	// Run kernel.
	size_t worksize = 1;
	error = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &worksize, &worksize, 0, NULL, NULL);
	assert(error == CL_SUCCESS);

	// Read output.
	error = clEnqueueReadBuffer(this->queue, output_buf, CL_FALSE, 0, sizeof(output), &output, 0, NULL, NULL);
	assert(error == CL_SUCCESS);

	// Wait until all queue is done.
	error = clFinish(queue);
	assert(error == CL_SUCCESS);

	for(int i=0; i<output; i++) {
		result->push_back(this->addresses->addresses[i]);
	}

	return 0;
}
