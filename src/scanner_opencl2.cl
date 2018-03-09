/* See https://github.com/sschaetz/nvidia-opencl-examples/blob/master/OpenCL/src/oclMatVecMul/oclMatVecMul.cl */
/*

single_scan0: global_worksize must equal address_space->sample.
single_scan1: no constraints.
single_scan2: no constraints (same as single_scan3, but less optimized).
single_scan3: local_worksize must be a power of 2.
single_scan3_16: local_worksize equal 16.
single_scan4: no constraints.
single_scan5: assume WARP_SIZE=32 and local_worksize must be a power of 2.
single_scan5_unroll: unrolls the last loop of single_scan5.
single_scan6: assume WARP_SIZE=32 and has no constraints.

*/

__kernel
void single_scan0(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint id = get_global_id(0);

	if (id < sample) {
		ulong a;
		uint dist;

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		for(uint j=0; j<bs_len; j++) {
			a = row[j] ^ bs[j];
			dist += popcount(a);
		}
		if (dist <= radius) {
			selected[atomic_inc(counter)] = id;
		}
	}
}

__kernel
void single_scan1(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint id;
	ulong a;
	uint dist;
	const __global ulong *row;
	
	for (id=get_global_id(0); id < sample; id += get_global_size(0)) {

		row = bitstrings + id*bs_len;

		dist = 0;
		for(uint j=0; j<bs_len; j++) {
			a = row[j] ^ bs[j];
			dist += popcount(a);
		}
		if (dist <= radius) {
			selected[atomic_inc(counter)] = id;
		}

	}
}

__kernel
void single_scan2(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist += popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		barrier(CLK_LOCAL_MEM_FENCE);

		if (get_local_id(0) == 0) {
			dist = 0;
			for(uint t = 0; t < bs_len; t++) {
				dist += partial_dist[t];
			}
			if (dist <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

__kernel
void single_scan3(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		// Parallel reduction to sum all partial_dist array.
		// The first barrier is in the beginning because it is needed after
		// partial_dist[get_local_id(0)] = dist, but it is not needed in the
		// last loop of the for (because only one partial_dist will be
		// updated).
		for(uint stride = get_local_size(0)/2; stride > 0; stride /= 2) {
			barrier(CLK_LOCAL_MEM_FENCE);
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
		}

		if (get_local_id(0) == 0) {
			if (partial_dist[0] <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

__kernel
void single_scan3_16(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;
		barrier(CLK_LOCAL_MEM_FENCE);

		// We do not need to sync because they all run in the same warp.
		if (get_local_id(0) < 8) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 8];
		}
		if (get_local_id(0) < 4) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 4];
		}
		if (get_local_id(0) < 2) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 2];
		}

		if (get_local_id(0) == 0) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 1];
			if (partial_dist[0] <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

__kernel
void single_scan4(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		// Parallel reduction to sum all partial_dist array.
		// The first barrier is in the beginning because it is needed after
		// partial_dist[get_local_id(0)] = dist, but it is not needed in the
		// last loop of the for (because only one partial_dist will be
		// updated).
		uint old_stride = get_local_size(0);
		__local uint extra;
		extra = 0;
		for(uint stride = get_local_size(0)/2; stride > 0; stride /= 2) {
			barrier(CLK_LOCAL_MEM_FENCE);
			if ((old_stride&1) == 1 && get_local_id(0) == old_stride-1) {
				extra += partial_dist[get_local_id(0)];
			}
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
			old_stride = stride;
		}

		if (get_local_id(0) == 0) {
			if (partial_dist[0] + extra <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

__kernel
void single_scan5(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		// Parallel reduction to sum all partial_dist array.
		// The first barrier is in the beginning because it is needed after
		// partial_dist[get_local_id(0)] = dist, but it is not needed in the
		// last loop of the for (because only one partial_dist will be
		// updated).
		uint stride;
		for(stride = get_local_size(0)/2; stride > 32; stride /= 2) {
			barrier(CLK_LOCAL_MEM_FENCE);
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		for(/**/; stride > 0; stride /= 2) {
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
		}

		if (get_local_id(0) == 0) {
			if (partial_dist[0] <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}
__kernel
void single_scan5_unroll(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		// Parallel reduction to sum all partial_dist array.
		// The first barrier is in the beginning because it is needed after
		// partial_dist[get_local_id(0)] = dist, but it is not needed in the
		// last loop of the for (because only one partial_dist will be
		// updated).
		for(uint stride = get_local_size(0)/2; stride > 32; stride /= 2) {
			barrier(CLK_LOCAL_MEM_FENCE);
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
		}

		// We do not need to sync because they all run in the same warp.
		if (get_local_id(0) < 32 && get_local_size(0) >= 64) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 32];
		}
		if (get_local_id(0) < 16 && get_local_size(0) >= 32) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 16];
		}
		if (get_local_id(0) < 8 && get_local_size(0) >= 16) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 8];
		}
		if (get_local_id(0) < 4 && get_local_size(0) >= 8) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 4];
		}
		if (get_local_id(0) < 2 && get_local_size(0) >= 4) {
			partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + 2];
		}

		if (get_local_id(0) == 0) {
			partial_dist[0] += partial_dist[1];
			if (partial_dist[0] <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

__kernel
void single_scan6(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uint *selected,
		__local uint *partial_dist)
{
	uint dist;
	ulong a;
	uint j;

	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		dist = 0;
		j = get_local_id(0);
		if (j < bs_len) {
			a = row[j] ^ bs[j];
			dist = popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		// Parallel reduction to sum all partial_dist array.
		// The first barrier is in the beginning because it is needed after
		// partial_dist[get_local_id(0)] = dist, but it is not needed in the
		// last loop of the for (because only one partial_dist will be
		// updated).
		uint old_stride = get_local_size(0);
		uint stride;
		__local uint extra;
		extra = 0;
		for(stride = get_local_size(0)/2; stride > 32; stride /= 2) {
			barrier(CLK_LOCAL_MEM_FENCE);
			if ((old_stride&1) == 1 && get_local_id(0) == old_stride-1) {
				extra += partial_dist[get_local_id(0)];
			}
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
			old_stride = stride;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		for(/**/; stride > 0; stride /= 2) {
			if ((old_stride&1) == 1 && get_local_id(0) == old_stride-1) {
				extra += partial_dist[get_local_id(0)];
			}
			if (get_local_id(0) < stride) {
				partial_dist[get_local_id(0)] += partial_dist[get_local_id(0) + stride];
			}
			old_stride = stride;
		}

		if (get_local_id(0) == 0) {
			if (partial_dist[0] + extra <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}
