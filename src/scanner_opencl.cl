__kernel
void single_scan(
		__constant const uchar *bitcount_table,
		__global const ushort *bitstrings,
		const uint bs_len,
		const uint sample,
		__constant const ushort *bs,
		const uint radius,
		__global uint *counter,
		__global uchar *selected,
		__local uchar *dist)
{

	uint global_id = get_global_id(0);
	uint local_id = get_local_id(0);

	if (global_id >= sample*bs_len) {
		return;
	}

	uint j = local_id % bs_len;

	ulong a = bitstrings[global_id] ^ bs[j];
	dist[local_id] = bitcount_table[a];

	//barrier(CLK_LOCAL_MEM_FENCE);

	if (j % bs_len == 0) {
		atomic_inc(counter);
		//uint idx = local_id / bs_len;
		//selected[idx] = (idx % 2 == 0);
	}
	return;


	if (local_id % bs_len == 0) {
		uint total_dist = 0;
		for(uint i=0; i<bs_len; i++) {
			total_dist += dist[local_id+i];
		}
		selected[global_id] = (total_dist <= radius);
	}
}
