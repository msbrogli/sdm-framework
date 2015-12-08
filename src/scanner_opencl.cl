__kernel
void scan(
		__global const uchar *bitcount_table, 
		__global const ulong *bitstrings, 
		const uint bs_len, 
		const uint sample, 
		const uint worksize,
		__global const ulong *bs,
		const uint radius, 
		__global uint *result)
{
	uint id = get_global_id(0);

	uint start, end;
	{
		const uint qty = sample/worksize;
		const uint extra = sample%worksize;
		const uint length = qty + (id < extra ? 1 : 0);
		start = id*qty + min(id, extra);
		end = min(sample, start + length);
	}

	*result = 0;

	ulong a;
	uint dist;
	ushort *ptr;
	for(uint i=start; i<end; i++) {
		dist = 0;
		for(uint j=0; j<bs_len; j++) {
			a = bitstrings[i*bs_len+j] ^ bs[j];
			ptr = (ushort *)&a;
			dist += bitcount_table[ptr[0]] + bitcount_table[ptr[1]] + bitcount_table[ptr[2]] + bitcount_table[ptr[3]];
		}
		if (dist <= radius) {
			atomic_inc(result);
		}
	}
}
