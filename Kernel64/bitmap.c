#include "stdafx.h"
#include "bitmap.h"

void bitmap_set(uint8_t* Map, uint32_t Bit) {
	Map[MAP_OFFSET(Bit)] |= 1 << BIT_OFFSET(Bit);
}

void bitmap_clear(uint8_t* Map, uint32_t Bit) {
	Map[MAP_OFFSET(Bit)] &= ~(1 << BIT_OFFSET(Bit));
}

BOOL bitmap_get(uint8_t* Map, uint32_t Bit) {
	return (Map[MAP_OFFSET(Bit)] & (1 << BIT_OFFSET(Bit))) != 0;
}