
#ifndef INC_GDT_H
#define INC_GDT_H

#include "../libc/rust_types.h"

/* Segment selectors */
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS 0x18
#define USER_DS 0x20

void set_idt_gate(u8,u32,u16,u8);

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct
{
    u16 limit_low;           // The lower 16 bits of the limit.
    u16 base_low;            // The lower 16 bits of the base.
    u8 base_middle;         // The next 8 bits of the base.
    u8 access;              // Access flags, determine what ring this segment can be used in.
    u8 granularity;
    u8 base_high;           // The last 8 bits of the base.
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
    u16 limit;               // The upper 16 bits of all selector limits.
    u32 base;                // The address of the first gdt_entry_t struct.
}__attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

/* How every interrupt gate (handler) is defined */
typedef struct {
    uint16_t low_offset; /* Lower 16 bits of handler function address */
    uint16_t sel; /* Kernel segment selector */
    uint8_t always0;
    /* First byte
     * Bit 7: "Interrupt is present"
     * Bits 6-5: Privilege level of caller (0=kernel..3=user)
     * Bit 4: Set to 0 for interrupt gates
     * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
    uint8_t flags;
    uint16_t high_offset; /* Higher 16 bits of handler function address */
} __attribute__((packed)) idt_entry_t ;

/* A pointer to the array of interrupt handlers.
 * Assembly instruction 'lidt' will read it */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// Initialisation function is publicly accessible.
void init_descriptor_tables();

#endif //INC_GDT_H
