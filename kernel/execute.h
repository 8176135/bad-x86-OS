//
// Created by elsecaller on 4/6/20.
//

#ifndef INC_EXECUTE_H
#define INC_EXECUTE_H

#include "../libc/rust_types.h"
#include "../cpu/isr.h"
#include "kernel.h"

typedef struct {
    PID pid;
    i32 arg;
    u32 scheduling_level;
    i32 n;
    i32 yielded;
    registers_t regs;
} process_t;

void check_schedule(u32 tick, registers_t* regs);

#endif //INC_23_FIXES_EXECUTE_H
