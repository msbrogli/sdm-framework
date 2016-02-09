__kernel
void single_scan(
		__constant const uchar *bitcount_table,
		__global const ulong *bitstrings,
		const uint bs_len,
		const uint sample,
		/*const uint worksize,*/
		__constant const ulong *bs,
		const uint radius,
		__global uint *counter,
		__global uchar *selected)
{
	uint id = get_global_id(0);

	ulong a;
	uint dist;

	dist = 0;
	for(uint j=0; j<bs_len; j++) {
		a = bitstrings[id*bs_len+j] ^ bs[j];
		dist += popcount(a);
	}
	selected[id] = (dist <= radius);
}
