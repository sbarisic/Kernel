#pragma once

#define BITS_PER_WORD (sizeof(uint8_t) * 8)
#define MAP_OFFSET(m) ((m) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)

void bitmap_set(uint8_t* Map, uint32_t Bit);
void bitmap_clear(uint8_t* Map, uint32_t Bit);
BOOL bitmap_get(uint8_t* Map, uint32_t Bit);