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
void single_scan(
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
	for (uint id = get_group_id(0); id < sample; id += get_num_groups(0)) {

		const __global ulong *row = bitstrings + id*bs_len;

		uint dist = 0;
		for(uint j = get_local_id(0); j < bs_len; j += get_local_size(0)) {
			ulong a = row[j] ^ bs[j];
			dist += popcount(a);
		}
		partial_dist[get_local_id(0)] = dist;

		barrier(CLK_LOCAL_MEM_FENCE);

		if (get_local_id(0) == 0) {
			uint total = 0;
			for(uint t = 0; t < get_local_size(0); t++) {
				total += partial_dist[t];
			}
			if (total <= radius) {
				selected[atomic_inc(counter)] = id;
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}
}
