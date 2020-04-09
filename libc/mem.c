#include "mem.h"
#include "kernel.h"

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

///* This should be computed at link time, but a hardcoded
// * value is fine for now. Remember that our kernel starts
// * at 0x1000 as defined on the Makefile */
//u32 free_mem_addr = 0x200000;
///* Implementation is just a pointer to some free memory which
// * keeps growing */
//u32 kmalloc(usize size, int align, u32 *phys_addr) {
//    /* Pages are aligned to 4K, or 0x1000 */
//    if (align == 1 && (free_mem_addr & 0xFFFFF000)) {
//        free_mem_addr &= 0xFFFFF000;
//        free_mem_addr += 0x1000;
//    }
//    /* Save also the physical address */
//    if (phys_addr) *phys_addr = free_mem_addr;
//
//    u32 ret = free_mem_addr;
//    free_mem_addr += size; /* Remember to increment the pointer */
//    return ret;
//}

// ~ 500Kb of stack... should be enough until I get paging working right?
#define MAX_STACK_SIZE 0x80000

usize calculate_stack_location(u32 pid) {
    // Remember minimum PID is 1
    return BASE_STACK_LOCATION + MAX_STACK_SIZE * pid;
}

// Tells you the free address space for each of the 30 different sizes (32 - 2 (1 bytes, 2 bytes))
MEMORY free_space[30];

void init_memory() {
	for (int i = 0; i < 30; ++i) {
		free_space[i] = BASE_MEM_LOCATION;
	}
}

MEMORY mem_alloc(u8 size) {
	MEMORY ans = free_space[size];
	for (int i = 0; i < 30; ++i) {
		if (ans == free_space[i]) {

		}
	}
	return ans;
}