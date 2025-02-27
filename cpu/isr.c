#include "isr.h"
#include "descriptor_tables.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../libc/string.h"
#include "../util/debugging.h"
#include "../kernel/execute.h"
#include "timer.h"
#include "ports.h"

isr_t interrupt_handlers[256];

/* To print the message which defines every exception */
char *exception_messages[] = {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",

        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",

        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",

        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
};

// Doesn't flush idt
void isr_install() {
    set_idt_gate(0,  (u32)isr0 , KERNEL_CS, 0x8E );
    set_idt_gate(1,  (u32)isr1 , KERNEL_CS, 0x8E );
    set_idt_gate(2,  (u32)isr2 , KERNEL_CS, 0x8E );
    set_idt_gate(3,  (u32)isr3 , KERNEL_CS, 0x8E );
    set_idt_gate(4,  (u32)isr4 , KERNEL_CS, 0x8E );
    set_idt_gate(5,  (u32)isr5 , KERNEL_CS, 0x8E );
    set_idt_gate(6,  (u32)isr6 , KERNEL_CS, 0x8E );
    set_idt_gate(7,  (u32)isr7 , KERNEL_CS, 0x8E );
    set_idt_gate(8,  (u32)isr8 , KERNEL_CS, 0x8E );
    set_idt_gate(9,  (u32)isr9 , KERNEL_CS, 0x8E );
    set_idt_gate(10, (u32)isr10, KERNEL_CS, 0x8E);
    set_idt_gate(11, (u32)isr11, KERNEL_CS, 0x8E);
    set_idt_gate(12, (u32)isr12, KERNEL_CS, 0x8E);
    set_idt_gate(13, (u32)isr13, KERNEL_CS, 0x8E);
    set_idt_gate(14, (u32)isr14, KERNEL_CS, 0x8E);
    set_idt_gate(15, (u32)isr15, KERNEL_CS, 0x8E);
    set_idt_gate(16, (u32)isr16, KERNEL_CS, 0x8E);
    set_idt_gate(17, (u32)isr17, KERNEL_CS, 0x8E);
    set_idt_gate(18, (u32)isr18, KERNEL_CS, 0x8E);
    set_idt_gate(19, (u32)isr19, KERNEL_CS, 0x8E);
    set_idt_gate(20, (u32)isr20, KERNEL_CS, 0x8E);
    set_idt_gate(21, (u32)isr21, KERNEL_CS, 0x8E);
    set_idt_gate(22, (u32)isr22, KERNEL_CS, 0x8E);
    set_idt_gate(23, (u32)isr23, KERNEL_CS, 0x8E);
    set_idt_gate(24, (u32)isr24, KERNEL_CS, 0x8E);
    set_idt_gate(25, (u32)isr25, KERNEL_CS, 0x8E);
    set_idt_gate(26, (u32)isr26, KERNEL_CS, 0x8E);
    set_idt_gate(27, (u32)isr27, KERNEL_CS, 0x8E);
    set_idt_gate(28, (u32)isr28, KERNEL_CS, 0x8E);
    set_idt_gate(29, (u32)isr29, KERNEL_CS, 0x8E);
    set_idt_gate(30, (u32)isr30, KERNEL_CS, 0x8E);
    set_idt_gate(31, (u32)isr31, KERNEL_CS, 0x8E);

    // Remap the PIC, because the original conflicts with the CPU exceptions
    // (Why have this issue in the first place??)
    // See: http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs%20and%20the%20PIT.html
    // And: https://wiki.osdev.org/PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0); 

    // Install the IRQs
    set_idt_gate(32, (u32)irq0 ,KERNEL_CS, 0x8E);
    set_idt_gate(33, (u32)irq1 ,KERNEL_CS, 0x8E);
    set_idt_gate(34, (u32)irq2 ,KERNEL_CS, 0x8E);
    set_idt_gate(35, (u32)irq3 ,KERNEL_CS, 0x8E);
    set_idt_gate(36, (u32)irq4 ,KERNEL_CS, 0x8E);
    set_idt_gate(37, (u32)irq5 ,KERNEL_CS, 0x8E);
    set_idt_gate(38, (u32)irq6 ,KERNEL_CS, 0x8E);
    set_idt_gate(39, (u32)irq7 ,KERNEL_CS, 0x8E);
    set_idt_gate(40, (u32)irq8 ,KERNEL_CS, 0x8E);
    set_idt_gate(41, (u32)irq9 ,KERNEL_CS, 0x8E);
    set_idt_gate(42, (u32)irq10,KERNEL_CS, 0x8E);
    set_idt_gate(43, (u32)irq11,KERNEL_CS, 0x8E);
    set_idt_gate(44, (u32)irq12,KERNEL_CS, 0x8E);
    set_idt_gate(45, (u32)irq13,KERNEL_CS, 0x8E);
    set_idt_gate(46, (u32)irq14,KERNEL_CS, 0x8E);
    set_idt_gate(47, (u32)irq15,KERNEL_CS, 0x8E);
}

void isr_handler(registers_t *r) {
//    dbg_hex("received interrupt: ", r->int_no);
    if (r->int_no == 0x13) {
        check_schedule(get_tick(), r);
    } else {
		dbg_hex("received interrupt: ", r->int_no);
        kprint(exception_messages[r->int_no]);
    }
}

void register_interrupt_handler(u8 n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// End of Interrupt command
#define PIC_EOI 0x20

void irq_handler(registers_t *r) {
    if (r->int_no >= 40) port_byte_out(0xA0, PIC_EOI); /* slave */
    port_byte_out(0x20, PIC_EOI); /* master */

    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
}

void irq_install() {
    /* Enable interruptions */
    asm volatile("sti");
    /* IRQ0: timer */
    init_timer(1);
    /* IRQ1: keyboard */
//    init_keyboard();
}
