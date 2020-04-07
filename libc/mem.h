#ifndef MEM_H
#define MEM_H

#include "rust_types.h"

void memory_copy(const u8 *source, u8 *dest, i32 nbytes);
void memory_set(u8 *dest, u8 val, u32 len);

/* At this stage there is no 'free' implemented. */
u32 kmalloc(usize size, int align, u32 *phys_addr);

usize calculate_stack_location(u32 pid);

#endif
