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
	if (global_id >= sample*bs_len) {
		return;
	}

	uint local_id = get_local_id(0);
	uint j = local_id % bs_len;

	ulong a = bitstrings[global_id] ^ bs[j];
	dist[local_id] = popcount(a);

	barrier(CLK_LOCAL_MEM_FENCE);

	uint offset = bs_len;
	while(true) {
		if (offset == 1) {
			break;
		}
		if (offset&1 && j == 0) {
			dist[local_id] += dist[offset];
		}
		offset >>= 1;
		if (j < offset) {
			dist[local_id] += dist[local_id+offset];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (j == 0) {
		uint idx = global_id / bs_len;
		selected[idx] = (dist[local_id] <= radius);
	}
}
