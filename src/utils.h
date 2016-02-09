#ifndef SDM_UTILS_H
#define SDM_UTILS_H

inline
int is_little_endian() {
	volatile uint32_t i=0x01234567;
	return (*((uint8_t*)(&i))) == 0x67;
}

#endif
