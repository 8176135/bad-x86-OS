#ifndef TIMER_H
#define TIMER_H

#include "../libc/rust_types.h"

void init_timer(uint32_t freq);
u32 get_tick();

#endif
