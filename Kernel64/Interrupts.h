#pragma once

#define INT_FLAG_PRESENT (1 << 7)

#define INT_FLAG_PRIV_0 (0 << 5)
#define INT_FLAG_PRIV_1 (1 << 5)
#define INT_FLAG_PRIV_2 (2 << 5)
#define INT_FLAG_PRIV_3 (3 << 5)

#define INT_FLAG_INTGATE (0b1110)
#define INT_FLAG_TRAPGATE (0b1111)
#define INT_FLAG_TASKGATE (0b0101)

void init_interrupts();
void int_handler();
void int_handler2(int32_t Error, int32_t IntNum);