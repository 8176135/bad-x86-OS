#ifndef MEM_H
#define MEM_H

#include "rust_types.h"

#define BASE_MEM_LOCATION 0x200000
#define BASE_STACK_LOCATION 0x800000
// ~ 500Kb of stack... should be enough until I get paging working right?
#define MAX_STACK_SIZE 0x80000

// ~ 500mb
// Loads of free memory (~500mb), but don't overflow. TODO: Might increase this by 1
#define MAX_SINGLE_MALLOC_SIZE_SHIFT 0x1Du
#define MAX_SINGLE_MALLOC_SIZE (0x1u << MAX_SINGLE_MALLOC_SIZE_SHIFT)

#define MEMORY_IN_USE 0xF0
#define MEMORY_CLEAR 0x0F

void memory_copy(const u8 *source, u8 *dest, i32 nbytes);
void memory_set(u8 *dest, u8 val, u32 len);

usize calculate_stack_location(u32 pid);

#endif
