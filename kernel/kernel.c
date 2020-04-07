#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../libc/rust_types.h"
#include "../cpu/descriptor_tables.h"
#include "execute.h"

#include "../applications/counter/counter.h"

#define MAXSPORADIC MAXPROCESS
#define MAXDEVICE MAXPROCESS

char debug_buffer[64];

//// TODO: Dynamically allocate PPP, because increasing this will cause a bootloop lol
int PPP[256];
int PPPMax[256];
int PPP_index = 0;
int PPPLen;
//
///// Simple ring buffer queue of PIDs
u32 sporadic_buf[MAXSPORADIC];
usize sporadic_idx;
usize sporadic_length;
//
///// Simple ring buffer queue of PIDs
u32 device_buf[MAXDEVICE];
usize device_idx;
usize device_length;

// Keep track of the names available to prevent duplicate names.
u32 name_registry[(MAXPROCESS / 8 + 1)];

int register_name(u32 n) {
    usize idx = n / 32;
    if (name_registry[idx] >> (n % 32) != 0) { // Bit set
        return FALSE;
    } else {
        name_registry[idx] |= 0x1 << (n % 32);
        return TRUE;
    }
}

void unregister_name(u32 n) {
    usize idx = n / 32;
    name_registry[idx] &= 0x0 << (n % 32);
}

//// TODO: Dynamically allocate task list
process_t processes[MAXPROCESS];
u32 processes_length;
PID currently_running_process_pid = INVALIDPID;

///// This function won't return
void switch_process(PID new_pid, registers_t* old_regs) {
    if (currently_running_process_pid == new_pid) { // Nothing to do here.
        return;
    }

    // Do we need everything? Who knows! just copy them.
    processes[currently_running_process_pid - 1].regs = *old_regs;

    currently_running_process_pid = new_pid;
    registers_t *regs = &processes[currently_running_process_pid - 1].regs;

    // TODO: Use iret and set eflags to set interrupt flag. Though now that I think about it, it should be fine as it is
    // TODO: Add more stuff to restore
    // TODO: Pop eflags here too
    asm volatile("         \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%eax;       \
      mov %4, %%ecx;       \
      mov %5, %%edx;       \
      mov %6, %%ebx;       \
      mov %7, %%esi;       \
      mov %8, %%edi;       \
      sti;                 \
      jmp *%0           "
    : : "g"(regs->eip), "g"(regs->esp), "g"(regs->ebp), "g"(regs->eax), "g"(regs->ecx), "g"(regs->edx), "g"(regs->ebx), "g"(regs->esi), "g"(regs->edi));
}
//
PID pid_from_name(i32 n) {
    if (n == IDLE) {
        return INVALIDPID;
    }

    for (usize i = 0; i < processes_length; ++i) {
        if (processes[i].n == n) {
            return processes[i].pid;
        }
    }
    return INVALIDPID;
}

void kernel_main() {
    OS_Init();

    OS_Start();
    kprint("OS_Start returned... This really shouldn't happen\n");
    OS_Abort();
}

u32 last_change_tick = 0;
u32 PPP_debt = 0;
void check_schedule(u32 tick, registers_t* regs) {
//    // TODO: Interrupts should already be disabled here... probably should check at some point
    // Just clear it in case I'm wrong.
    asm("cli");

    for (usize i = 0; i < processes_length; ++i) {
        if (processes[i].scheduling_level == DEVICE && tick % processes[i].n == 0) {
            if (device_length >= MAXDEVICE) {
                kprint("Schedule ERROR: DEVICE queue full (devices taking a long time?)\n");
                // TODO: maybe don't abort, start deleting devices at the front of the queue.
                OS_Abort();
            }

            device_buf[(device_idx + device_length) % MAXDEVICE] = i;
            device_length++;
        }
    }

    if (device_length > 0 && device_buf[device_idx] != currently_running_process_pid) {
        last_change_tick = tick;
        switch_process(device_buf[device_idx], regs);
    }

    i32 delta = tick - last_change_tick;
    process_t* current_process = &processes[currently_running_process_pid - 1];
    switch (current_process->scheduling_level) {
        case DEVICE:
            if (delta > 2) {
                kprint("WARNING, devices hogging CPU for longer than expected\n");
            }
            break;
        case SPORADIC: // Is your idle time up?
        case PERIODIC:
            if (PPPMax[PPP_index] < delta) { // Times up
                last_change_tick = tick;
                PPP_index = (PPP_index + 1) % PPPLen;
                PID new_pid = pid_from_name(PPP[PPP_index]);
                if (new_pid == INVALIDPID) { // No process is taking this name yet, so just idle
                    if (sporadic_length == 0) { // No sporadic process scheduled either, just go and idle
                        return;
                    }
                    new_pid = sporadic_buf[sporadic_idx];
                }
                switch_process(new_pid, regs);
            }
            break;

        default:
            kprint("CORRUPTED process table, invalid scheduling level\n");
            OS_Abort();
    }
}

void OS_Init() {
    init_descriptor_tables();

    // NOTE: sizeof does not mean what I thought it meant
//    char stuff[64];
//    hex_to_ascii((u32)PPP, stuff);
//    kprint("\n");
//    kprint(stuff);
//    kprint("\n");
//    memory_set((u8*)stuff, 0, 64);
//    hex_to_ascii(sizeof(processes), stuff);
//    kprint("\n");
//    kprint(stuff);
//    kprint("\n");

    // Don't really need to
    memory_set((u8 *) PPP, 0, sizeof(PPP));
    memory_set((u8 *) PPPMax, 0, sizeof(PPPMax));

    PPP[0] = 1;
    PPPMax[0] = 10;

    PPP[1] = 2;
    PPPMax[1] = 5;

    PPP[2] = 3;
    PPPMax[2] = 10;

    PPP[3] = 1;
    PPPMax[3] = 5;

    PPP[4] = -1;
    PPPMax[4] = 5;

    memory_set((u8 *) processes, INVALIDPID, sizeof(processes));
    memory_set((u8 *) sporadic_buf, INVALIDPID, sizeof(sporadic_buf));
    memory_set((u8 *) name_registry, 0, sizeof(name_registry));

    PPP_index = 0;
    PPPLen = 0;
    processes_length = 0;
    sporadic_idx = 0;

    clear_screen();
}

void OS_Start() {
    irq_install();
    kprint("OS Started!!");
//    OS_Create((void*)counter_main, 0, PERIODIC, 1);

    while (1) {}
}

void OS_Abort() {
    kprint("CPU halting, Cya!\n");
    asm volatile("hlt");
}

void user_input(char *input) {
    if (strcmp(input, "END") == 0) {
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    } else if (strcmp(input, "PAGE") == 0) {
        uint32_t phys_addr;
        uint32_t page = kmalloc(1000, 1, &phys_addr);
        char page_str[16] = "";
        hex_to_ascii(page, page_str);
        char phys_str[16] = "";
        hex_to_ascii(phys_addr, phys_str);
        kprint("Page: ");
        kprint(page_str);
        kprint(", physical address: ");
        kprint(phys_str);
        kprint("\n");
    }

    kprint("You said: ");
    kprint(input);
    kprint("\n> ");
}

PID OS_Create(void (*f)(void), i32 arg, u32 level, u32 n) {
    asm("cli"); // Disable interrupts;

    PID new_pid = processes_length++ + 1;
    if (new_pid > MAXPROCESS) {
        kprint("OS_Create ERROR: Too many active processes");
        return INVALIDPID;
    } else if (level != DEVICE && !register_name(n)) {
        kprint("OS_Create ERROR: Name already taken");
        return INVALIDPID;
    }
    processes[new_pid - 1].scheduling_level = level;
    processes[new_pid - 1].arg = arg;
    processes[new_pid - 1].n = n;
    processes[new_pid - 1].pid = new_pid;
    // TODO: Write actual words
    static registers_t new_regs = {.ds = KERNEL_DS, .edi = 0, .esi = 0, .ebp = -1, .esp = -1, .ebx = KERNEL_DS, .edx = 0, .ecx = 0, .eax = 0, .int_no = 0, .err_code = 0, .eip = -1, .cs = KERNEL_CS, .ss = KERNEL_DS, .eflags = 0b00000000000000000000000100000000};
    new_regs.ebp = calculate_stack_location(new_pid);
    new_regs.esp = new_regs.ebp;
    new_regs.eip = (u32) f; // Does this work?

    processes[new_pid - 1].regs = new_regs;

    switch (level) {
        case DEVICE:
            // TODO: stuff keyboard interrupts here?
            break;
        case PERIODIC: // No need to do anything
            break;
        case SPORADIC:
            if (sporadic_length >= MAXSPORADIC) {
                kprint("OS_Create ERROR: SPORADIC FIFO queue full");
                return INVALIDPID;
            }

            sporadic_buf[(sporadic_idx + sporadic_length) % MAXSPORADIC] = new_pid;
            sporadic_length++;
            break;
        default:
            kprint("INVALID level passed to OS_Create");
            return INVALIDPID;
    }

    asm("sti"); // Reenable Interrupts
    return new_pid;
}