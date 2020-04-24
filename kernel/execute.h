
#ifndef INC_EXECUTE_H
#define INC_EXECUTE_H

#include "../libc/rust_types.h"
#include "../cpu/isr.h"
#include "kernel.h"

typedef struct {
    PID pid;
    i32 arg;
    u32 semaphores[(MAXSEM - 1) / 32 + 1];
    u32 scheduling_level;
    i32 n;
    i32 yielded;
    registers_t regs;
} process_t;

typedef struct {
	i32 length;
	i32 idx;
	u32 data[FIFOSIZE];
} fifo_buf;

typedef struct pid_ll {
	struct pid_ll* next;
	PID pid;
	BOOL valid;
} pid_ll;

// TODO: Perhaps use the _Atomic type in C11
typedef struct {
	int count;
	pid_ll* tail;
	pid_ll* head;
} semaphore;

void check_schedule(u32 tick, registers_t* regs);

#endif //INC_23_FIXES_EXECUTE_H
