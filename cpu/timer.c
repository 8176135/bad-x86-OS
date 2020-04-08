#include "timer.h"
#include "isr.h"
#include "ports.h"
#include "../kernel/execute.h"
#include "../libc/rust_types.h"

u32 tick = 0;
u32 fake_tick = 0;

static void timer_callback(registers_t *regs) {
    fake_tick++;
    if (fake_tick >= 100) {
        fake_tick = 0;
        tick++;
        check_schedule(tick, regs);
    }
}

void init_timer(u32 freq) {
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    u32 divisor = (1193180 / freq);
    u8 low  = (u8)(divisor & 0xFF);
    u8 high = (u8)( (divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0b00110110); // Mode 2 (rate generator), Channel 0, Access lo + hi byte
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

