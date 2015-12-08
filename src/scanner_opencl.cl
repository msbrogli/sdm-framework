__kernel void scan(
		__global const uchar *bitcount_table, 
		__global const ulong *bitstrings, 
		const uint bs_len, 
		const uint sample, 
		__global const ulong *bs,
		const uint radius, 
		__global uint *result)
{
	int id = get_global_id(0);
	
	ulong a;
	ushort *ptr;
	for(int i=0; i<sample; i++) {
		uint dist = 0;
		for(int j=0; j<bs_len; j++) {
			a = bitstrings[i*bs_len+j] ^ bs[j];
			ptr = (ushort *)&a;
			dist += bitcount_table[ptr[0]] + bitcount_table[ptr[1]] + bitcount_table[ptr[2]] + bitcount_table[ptr[3]];
		}
		if (dist <= radius) {
			*result++;
		}
	}
}

__kernel void hello(char base, __global char* buf)
{
	int x = get_global_id(0);
	buf[x] = base+x;
}
