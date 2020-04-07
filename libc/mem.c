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
    u8 *temp = (u8 *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

/* This should be computed at link time, but a hardcoded
 * value is fine for now. Remember that our kernel starts
 * at 0x1000 as defined on the Makefile */
u32 free_mem_addr = 0x200000;
/* Implementation is just a pointer to some free memory which
 * keeps growing */
u32 kmalloc(usize size, int align, u32 *phys_addr) {
    /* Pages are aligned to 4K, or 0x1000 */
    if (align == 1 && (free_mem_addr & 0xFFFFF000)) {
        free_mem_addr &= 0xFFFFF000;
        free_mem_addr += 0x1000;
    }
    /* Save also the physical address */
    if (phys_addr) *phys_addr = free_mem_addr;

    u32 ret = free_mem_addr;
    free_mem_addr += size; /* Remember to increment the pointer */
    return ret;
}

#define BASE_STACK_LOCATION 0x800000
// ~ 500Kb of stack... should be enough until I get paging working right?
#define MAX_STACK_SIZE 0x80000

usize calculate_stack_location(u32 pid) {
    // Remember minimum PID is 1
    return BASE_STACK_LOCATION + MAX_STACK_SIZE * pid;
}