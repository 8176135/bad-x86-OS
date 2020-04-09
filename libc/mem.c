#include "mem.h"

/// Works with overlapping copies
void memory_copy(const u8 *source, u8 *dest, i32 nbytes) {
    i32 i;

    if (source > dest) {    // Copying ip
        for (i = 0; i < nbytes; i++) {
            *(dest + i) = *(source + i);
        }
    } else if (source < dest) { // Copying down
        for (i = nbytes - 1; i > 0; i--) {
            *(dest + i) = *(source + i);
        }
    }
}

void memory_set(u8 *dest, u8 val, u32 len) {
    while (len --> 0) *dest++ = val; // :D
}

usize calculate_stack_location(u32 pid) {
    // Remember minimum PID is 1
    return BASE_STACK_LOCATION + MAX_STACK_SIZE * pid;
}
