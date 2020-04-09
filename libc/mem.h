#ifndef MEM_H
#define MEM_H

#include "rust_types.h"

// Literally total addressable space
#define MAX_SINGLE_MALLOC 0x100000000
#define BASE_MEM_LOCATION 0x2000000
#define BASE_STACK_LOCATION 0x800000

void memory_copy(const u8 *source, u8 *dest, i32 nbytes);
void memory_set(u8 *dest, u8 val, u32 len);

usize calculate_stack_location(u32 pid);

#endif
