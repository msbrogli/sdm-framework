__kernel
void single_scan(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uchar *selected,
		__local uint *dist)
{

	uint global_id = get_global_id(0);
	uint local_id = get_local_id(0);

	if (global_id >= sample*bs_len) {
		return;
	}

	uint j = local_id % bs_len;

	ulong a = bitstrings[global_id] ^ bs[j];
	dist[local_id] = popcount(a);

	barrier(CLK_LOCAL_MEM_FENCE);

	if (j % bs_len == 0) {
		uint idx = global_id / bs_len;
		uint total_dist = 0;
		for(uint i=0; i<bs_len; i++) {
			total_dist += dist[local_id+i];
		}
		selected[idx] = (total_dist <= radius);
	}
}
