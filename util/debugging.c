#include "debugging.h"
#include "../libc/string.h"
#include "../drivers/screen.h"
#include "../libc/rust_types.h"

char debug_buffer[256];

void dbg(char *label, int number) {
//    kprint("\n");
	debug_buffer[0] = 0;
    kprint(label);
    int_to_ascii(number, debug_buffer);
    kprint(debug_buffer);
    kprint("\n");
}

void dbg_hex(char *label, int number) {
	debug_buffer[0] = 0;
    kprint(label);
    hex_to_ascii(number, debug_buffer);
    kprint(debug_buffer);
    kprint("\n");
}
