#pragma once

#include <intrin.h>
#include <intrin0.h>

#include <string.h>
#include <stdint.h>

#define _WIN64
#include <Windows.h>
#undef _WIN64

#define BREAK __asm { xchg bx, bx }