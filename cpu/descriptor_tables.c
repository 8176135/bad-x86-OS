//
// Created by elsecaller on 4/7/20.
//

#include "descriptor_tables.h"
#include "../libc/mem.h"
#include "isr.h"

// Private
static void init_gdt();
static void init_idt();
static void gdt_set_gate(usize,u32,u32,u8,u8);

extern void gdt_flush(u32);
extern void idt_flush(u32);

gdt_entry_t gdt_entries[5];
gdt_ptr_t   gdt_ptr;
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

// Initialisation routine - zeroes all the interrupt service routines,
// initialises the GDT and IDT.
void init_descriptor_tables()
{
    // Initialise the global descriptor table.
    init_gdt();
    init_idt();
}

static void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (u32)&idt_entries;

    memory_set((u8*)&idt_entries, 0, sizeof(idt_entry_t)*256);

    isr_install();

    idt_flush((u32)&idt_ptr);
}

static void init_gdt()
{
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (u32)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0b10011010, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0b10010010, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0b11111010, 0xCF); // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0b11110010, 0xCF); // User mode data segment

    gdt_flush((u32)&gdt_ptr);
}

// Set the value of one GDT entry.
static void gdt_set_gate(usize num, u32 base, u32 limit, u8 access, u8 gran)
{
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

void set_idt_gate(u8 num, u32 base, u16 sel, u8 flags)
{
    idt_entries[num].low_offset = base & 0xFFFF;
    idt_entries[num].high_offset = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags   = flags /* | 0x60 */;
}