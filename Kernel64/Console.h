#pragma once

#define _str2(s) #s
#define _str(s) _str2(s)
#define AT "[" __FUNCTION__ "]" __FILE__ ":" _str(__LINE__)
#define TRACE(msg) console_write(AT "  " msg "\n")

void console_init();
void console_write(const char* Str);
void console_writehex(int32_t Num);