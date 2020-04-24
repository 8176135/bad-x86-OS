#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../libc/rust_types.h"
#include "../cpu/descriptor_tables.h"
#include "../cpu/timer.h"
#include "execute.h"
#include "../util/debugging.h"

#include "../applications/counter/counter.h"
#include "../applications/reminder/reminder.h"
#include "../applications/idle/idle.h"
#include "../applications/memory_wiper/memory_wiper.h"
#include "../applications/sema/sema.h"

//#include <stdlib.h>

#define MAXSPORADIC MAXPROCESS
#define MAXDEVICE MAXPROCESS
#define INVALID_IDX 999999999
#define IDLE_PRIORITY 3

int idle_pid;

//// TODO: Dynamically allocate PPP
int PPP[128];
int PPPMax[128];
int PPP_index;
int PPPLen;

fifo_buf fifo_collection[MAXFIFO];

///// Simple ring buffer queue of PIDs
u32 sporadic_buf[MAXSPORADIC];
usize sporadic_idx;
// Technically not necessary, nice to have a double ended queue though
usize sporadic_length;
//
///// Simple ring buffer queue of PIDs
u32 device_buf[MAXDEVICE];
usize device_idx;
// Technically not necessary, nice to have a double ended queue though
usize device_length;

/// Contains only PERIODIC items
semaphore semaphore_buf[MAXSEM];

// Keep track of the names available to prevent duplicate names.
u32 name_registry[(MAXPROCESS / 8 + 1)];

fifo_buf *get_fifo(FIFO handler) {
	if (handler == INVALIDFIFO || handler >= MAXFIFO) {
		kprint("Invalid handler passed to OS\n");
		// TODO: Maybe change this to terminate
		OS_Abort();
	}

	return &fifo_collection[handler - 1];
}

int register_bit(u32 *bitmap, u32 n) {
	usize idx = n / 32;
	if (bitmap[idx] & (1u << (n % 32))) { // Bit set
		return FALSE;
	} else {
		bitmap[idx] |= 0x1u << (n % 32);
		return TRUE;
	}
}

int unregister_bit(u32 *bitmap, u32 n) {
	usize idx = n / 32;
	if (bitmap[idx] & (1u << (n % 32))) { // Bit set
		bitmap[idx] &= 0x0u << (n % 32);
		return TRUE;
	} else {
		return FALSE;
	}
}

int register_name(u32 n) {
	return register_bit(name_registry, n);
}

void unregister_name(u32 n) {
	unregister_bit(name_registry, n);
}

//// TODO: Dynamically allocate task list
process_t processes[MAXPROCESS];
PID processes_yielded[MAXPROCESS];
int processes_yielded_length;
//u32 processes_length;
u32 pid_counter = 1;
PID currently_running_process_pid = INVALIDPID;

PID pid_from_name(i32 n) {
	if (n == IDLE) {
		return INVALIDPID;
	}
//	dbg("Process length: ", processes_length);
	for (usize i = 0; i < MAXPROCESS; ++i) {
		if (processes[i].pid != INVALIDPID && processes[i].n == n) {
			return processes[i].pid;
		}
	}
	return INVALIDPID;
}

usize process_index_from_pid(PID pid) {
	if (pid == INVALIDPID) { // Catch error early
		kprint("Panic: passed INVALIDPID to process_index_from_pid");
		OS_Abort();
	}

	for (usize i = 0; i < MAXPROCESS; ++i) {
		if (processes[i].pid == pid) {
			return i;
		}
	}

	dbg("Passed PID not found: ", pid);
	OS_Abort();
	// Will never run
	return -1;
}

usize find_available_process_index() {
	for (usize i = 0; i < MAXPROCESS; ++i) {
		if (processes[i].pid == INVALIDPID) {
			return i;
		}
	}
	return INVALID_IDX;
}

///// This function won't return
void switch_process(PID new_pid, registers_t *old_regs) {
	if (currently_running_process_pid == new_pid) { // Nothing to do here.
		return;
	} else if (new_pid == INVALIDPID) {
		kprint("Invalid PID to switch to\n");
		return;
	}
	kprint("\nCurrently switching process to: ");

	if (currently_running_process_pid != INVALIDPID) {
		// Do we need everything? Who knows! just copy them.
		usize process_idx = process_index_from_pid(currently_running_process_pid);
		processes[process_idx].regs = *old_regs;
		processes[process_idx].yielded = FALSE;
		// TODO: Get iret working I suppose, return this back to 0x08
		processes[process_idx].regs.esp += 0x14; // MAGIC! (not really, cleans up the stack so when we return it is at the right place)
		dbg("", processes[process_idx].n);
	}

	currently_running_process_pid = new_pid;
	registers_t *regs = &processes[process_index_from_pid(currently_running_process_pid)].regs;

	// TODO: Use iret and set eflags to set interrupt flag. Though now that I think about it, it should be fine as it is
	// TODO: Add more stuff to restore
	// TODO: Pop eflags here too

	int registers[] = {regs->eip, regs->edx, regs->ecx, regs->eax, regs->ebp, regs->esp, regs->edi, regs->esi,
					   regs->ebx};

	// Restores every register without needing an extra one :D
	asm volatile("		            \
		movl %0, %%ebx;	            \
		movl 0x4 (%%ebx), %%edx;	\
		movl 0x8 (%%ebx), %%ecx;	\
		movl 0xc (%%ebx), %%eax;	\
		movl 0x10(%%ebx), %%ebp;	\
		movl 0x14(%%ebx), %%esp;	\
		movl 0x18(%%ebx), %%edi;	\
		movl 0x1c(%%ebx), %%esi;	\
		push (%%ebx);				\
		movl 0x20(%%ebx), %%ebx;	\
		sti;						\
		ret			   "
	: : "a"((int) registers));
	kprint("You can't see this\n");
	OS_Abort(); // If we got here something's gone terribly wrong
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

void kernel_main() {
	OS_Init();
	clear_screen();
	OS_Start();
	kprint("OS_Start returned... This really shouldn't happen\n");
	OS_Abort();
}

#pragma clang diagnostic pop

u32 last_change_tick = 0;
// TODO: do we really need this? answer, no, since if something lasts multiple ticks, the problem kind of solves itself
//u32 literally_the_most_recent_tick_number = 999999;
u32 PPP_debt = 0;

void check_schedule(u32 tick, registers_t *regs) {
//	// TODO: Interrupts should already be disabled here... probably should check at some point
	// Just clear it in case I'm wrong.
	asm("cli");
	kprint("Tick");
	i32 delta = tick - last_change_tick;
	if (delta) {
//		literally_the_most_recent_tick_number = tick;
		for (usize i = 0; i < MAXPROCESS; ++i) {
			if (processes[i].pid == INVALIDPID) {
				continue;
			}
			if (processes[i].scheduling_level == DEVICE && tick % processes[i].n == 0) {
				if (device_length >= MAXDEVICE) {
					kprint("Schedule ERROR: DEVICE queue full (devices taking a long time?)\n");
					// TODO: maybe don't abort, start deleting devices at the front of the queue. as they could just be stuck
					OS_Abort();
				}
				// NOTE: Assuming that no duplicates will appear because of how "fast" the devices will complete
				device_buf[(device_idx + device_length) % MAXDEVICE] = processes[i].pid;
				device_length++;
			}
		}
	}

	if (device_length > 0 && device_buf[device_idx] != currently_running_process_pid) {
		PPP_debt += delta;
		last_change_tick = tick;
		switch_process(device_buf[device_idx], regs);
		return;
	}

	process_t *current_process = NULL;

	if (PPP_index == INVALID_IDX) { // OS just started.
		last_change_tick = tick;
		PPP_index = 0;
		// TODO: This is such a bad idea
		goto LAUNCH_CURRENT_PPP;
	}

	if (delta < 0) { // Reset clock if we overflowed
		delta = 1;
	}
	PID next_pid = currently_running_process_pid;
	if (currently_running_process_pid == INVALIDPID) { // Some process terminated early
		// TODO: really need to restructure this at some point
		goto PERIODIC_CHECK;
	}

	current_process = &processes[process_index_from_pid(currently_running_process_pid)];
	switch (current_process->scheduling_level) {

		case DEVICE:
			if (current_process->yielded) {
				last_change_tick = tick - PPP_debt;
				PPP_debt = 0;
				delta = tick - last_change_tick;
				if (delta < 0) { // Reset clock if we overflowed
					delta = 1;
				}
				next_pid = pid_from_name(PPP[PPP_index]);
				current_process = 0;
				// Skips Sporadic TODO: This goto thing really is a slippery slope, planned on using 1, now I have 3 and counting
				goto PERIODIC_CHECK;
			}
			if (delta > 2) {
				kprint("WARNING, devices hogging CPU for longer than expected\n");
			}
			break;
		case SPORADIC:
			if (current_process->yielded) {
				if (sporadic_length == 0) {
					kprint("Assert failed: sporadic buffer length 0.\n");
					OS_Abort();
				}

				// NOTE: Not setting old position to INVALID_PID, (To save using 1 temporary variable lol)
				// if we are no longer using sporadic_length, this needs to be updated.
				sporadic_buf[(sporadic_idx + sporadic_length) % MAXSPORADIC] = sporadic_buf[sporadic_idx];
				sporadic_idx = (sporadic_idx + 1) % MAXSPORADIC;
			}
			// fall through
		case IDLE_PRIORITY:
		case PERIODIC:
		PERIODIC_CHECK:;
			if (PPPMax[PPP_index] < delta) { // Times up
				last_change_tick = tick;
				PPP_index = (PPP_index + 1) % PPPLen;
				LAUNCH_CURRENT_PPP:;
				next_pid = pid_from_name(PPP[PPP_index]);
				dbg("Currently at PPP_index: ", PPP_index);
			}
			if (next_pid == INVALIDPID || (current_process &&
										   current_process->yielded)) { // No process is taking this name yet, or gave the time slot up, or was sporadic
//				dbg("Sporadic Length: ", sporadic_length);
				if (sporadic_length == 0) { // No sporadic process scheduled either, just go and idle
					kprint(" -- CPU Idling \n");
					switch_process(idle_pid, regs);
					return;
				}
				next_pid = sporadic_buf[sporadic_idx];
			}
			switch_process(next_pid, regs);
			break;

		default:
			kprint("CORRUPTED process table, invalid scheduling level\n");
			OS_Abort();
	}
}

void OS_Init() {
	init_descriptor_tables();
	// Don't really need to
	memory_set((u8 *) PPP, 0, sizeof(PPP));
	memory_set((u8 *) PPPMax, 0, sizeof(PPPMax));

	PPP[0] = 1;
	PPPMax[0] = 7;

	PPP[1] = 2;
	PPPMax[1] = 5;

	PPP[2] = 3;
	PPPMax[2] = 3;

	PPP[3] = 1;
	PPPMax[3] = 5;

	PPP[4] = IDLE;
	PPPMax[4] = 3;

	for (int i = 0; i < MAXPROCESS; ++i) {
		processes[i].pid = INVALIDPID;
	}
	memory_set((u8 *) sporadic_buf, INVALIDPID, sizeof(sporadic_buf));
	memory_set((u8 *) fifo_collection, 0, sizeof(fifo_collection));

	for (int i = 0; i < MAXFIFO; ++i) {
		fifo_collection[i].length = -1;
	}

	memory_set((u8 *) semaphore_buf, 0, sizeof(semaphore_buf));
	memory_set((u8 *) name_registry, 0, sizeof(name_registry));

	PPP_index = INVALID_IDX;
	PPPLen = 5;
//	processes_length = 0;
	sporadic_idx = 0;
	processes_yielded_length = 0;

	OS_InitMemory();
}

void OS_Start() {
	// Mandatory:
	idle_pid = OS_Create((void *) idle_main, 0, IDLE_PRIORITY, 0);

	OS_Create((void *) sema_main, 0, PERIODIC, 2);

	// Uncomment this to test out memory management:
//	OS_Create((void *) memory_wiper_main, 0, PERIODIC, 1);

//	OS_Create((void *) counter_main, 0, PERIODIC, 1);
//	OS_Create((void *) counter_main, 1, PERIODIC, 2);
//	OS_Create((void *) counter_main, 2, PERIODIC, 3);
//	OS_Create((void *) counter_main, 3, PERIODIC, 4);
//	OS_Create((void *) counter_main, 4, SPORADIC, 5);
//	OS_Create((void *) counter_main, 5, SPORADIC, 6);
//	OS_Create((void *) reminder_main, 5, DEVICE, 10);
//	OS_Create((void *) reminder_main, 5, DEVICE, 20);

	irq_install();
	kprint("OS Started!!");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {} // Wait for a tick to take control away
#pragma clang diagnostic pop
}

void OS_Abort() {
	kprint("CPU halting, Cya!\n");
	asm volatile("hlt");
}

int OS_GetParam(void) {
	return processes[process_index_from_pid(currently_running_process_pid)].arg;
}

PID OS_Create(void (*f)(void), i32 arg, u32 level, u32 n) {
	asm volatile ("cli"); // Disable interrupts;

	usize process_index = find_available_process_index();

	if (process_index == INVALID_IDX) {
		kprint("OS_Create ERROR: Too many active processes");
		return INVALIDPID;
	} else if (level != DEVICE && level != IDLE_PRIORITY && !register_name(n)) {
		kprint("OS_Create ERROR: Name already taken");
		return INVALIDPID;
	}

	PID new_pid = pid_counter++;

	processes[process_index].scheduling_level = level;
	processes[process_index].arg = arg;
	processes[process_index].n = n;
	processes[process_index].pid = new_pid;
	processes[process_index].yielded = FALSE;
	// TODO: Write a comment here
	static registers_t new_regs = {.ds = KERNEL_DS, .edi = 0, .esi = 0, .ebp = -1, .esp = -1, .ebx = KERNEL_DS, .edx = 0, .ecx = 0, .eax = 0, .int_no = 0, .err_code = 0, .eip = -1, .cs = KERNEL_CS, .ss = KERNEL_DS, .eflags = 0b00000000000000000000000100000000};
	new_regs.ebp = calculate_stack_location(new_pid);
	new_regs.esp = new_regs.ebp;
	new_regs.eip = (u32) f;

	processes[process_index].regs = new_regs;

	switch (level) {
		case IDLE_PRIORITY:
			break;
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

	asm volatile ("sti"); // Reenable Interrupts
	return new_pid;
}

// TODO: Doesn't automatically terminate applications, but shouldn't be too hard,
//   just push OS_Terminate onto the stack right before starting
//   Does require `switch process` to know when a process is first starting though.
//   Solves the iret problem too.
// TODO: Clean up memory after an application has terminated.
void OS_Terminate(void) {
	asm volatile ("cli");
	usize idx = process_index_from_pid(currently_running_process_pid);
	dbg("Terminating process: ", processes[idx].n);
	switch (processes[idx].scheduling_level) {
		case DEVICE:
			if (processes[idx].pid != device_buf[device_idx]) {
				dbg("ASSERT FAIL: OS_Terminate Device PID doesn't match: ", processes[idx].pid);
				dbg("!= ", device_buf[device_idx]);
				OS_Abort();
			}
			device_buf[device_idx] = INVALIDPID;
			device_idx = (device_idx + 1) % MAXDEVICE;
			device_length--;
			break;
		case SPORADIC:
			if (processes[idx].pid != sporadic_buf[sporadic_idx]) {
				dbg("ASSERT FAIL: OS_Terminate Sporadic PID doesn't match: ", processes[idx].pid);
				dbg("!= ", sporadic_buf[sporadic_idx]);
				OS_Abort();
			}
			sporadic_buf[sporadic_idx] = INVALIDPID;
			sporadic_idx = (sporadic_idx + 1) % MAXSPORADIC;
			sporadic_length--;
			// fall through
		case PERIODIC:
			unregister_name(processes[idx].n);
			break;
		case IDLE_PRIORITY:
			kprint("Idle process is being terminated??????\n");
			// fall through
		default:
			kprint("PANIC: INVALID level in OS_Terminate, corrupted?");
			OS_Abort();
	}
	processes[idx].pid = INVALIDPID;
	currently_running_process_pid = INVALIDPID;
	asm volatile ("sti");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {} // Wait for the check_schedule to execute again.
#pragma clang diagnostic pop
}

void OS_Yield(void) {
	// TODO: Do we need to stop interrupts here? Same with above
	asm volatile ("cli");
	usize idx = process_index_from_pid(currently_running_process_pid);
	if (processes[idx].yielded) {
		asm volatile ("sti");
		return;
	}
	processes[idx].yielded = TRUE;
	if (processes[idx].scheduling_level == DEVICE) {
		device_buf[device_idx] = INVALIDPID;
		device_idx = (device_idx + 1) % MAXDEVICE;
		device_length--;
	}
	dbg("This process yielded, Name: ", processes[idx].n);
	asm volatile ("sti"); // Enable interrupts
	asm volatile ("int $0x13"); // Trigger a reserved interrupt number (Are you suppose to use reserved stuff?)
}

// TODO: Replace this with a binary tree or hashset, also can keep track of PID of which process owns what memory
#define MAX_NUMBER_OF_MALLOCS 128
usize address_book[MAX_NUMBER_OF_MALLOCS];

void OS_InitMemory() {
	// I just spent 1.5 hours debugging why this memory set function isn't not working.
	// Turns out that GDB just decided to randomly shift the address of `address_book` by 0x1000,
	// So when I printed out address_book in the debugger it was giving me completely unrelated data.
	// like WTF.
	//
	// So basically gdb is lying to you, address book is nice and empty after this memory_set function thank you.
	memory_set((uint8_t *) address_book, 0, sizeof(address_book));
	*(u8 *) BASE_MEM_LOCATION = MEMORY_CLEAR;
	*(u8 *) (BASE_MEM_LOCATION + 1) = MAX_SINGLE_MALLOC_SIZE_SHIFT;
}

/// Malloc that walks through all the allocated memory regions, finding one big enough for the requested memory to be given
/// Some protection against the user application from wrongly freeing stuff, kind of
/// TODO: Have a hashset or binary tree to keep track of all the addresses that are currently allocated.
MEMORY OS_Malloc(int val) {
	if (val < 2) {
		return 0;
	}
	u32 actual_size = val + 1;
	// Round up to the next power of 2
	// From: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
//	actual_size--;
	actual_size |= actual_size >> 1u;
	actual_size |= actual_size >> 2u;
	actual_size |= actual_size >> 4u;
	actual_size |= actual_size >> 8u;
	actual_size |= actual_size >> 16u;
	actual_size++;

	if (actual_size > MAX_SINGLE_MALLOC_SIZE) {
		return 0;
	}

	int total_available_space = 0;
	usize current_base = BASE_MEM_LOCATION;
	while (current_base < BASE_STACK_LOCATION - 2) {
		usize memory_space = 0x1u << (*(u8 *) (current_base + 1));
		if (*(u8 *) current_base == MEMORY_IN_USE) {
			current_base += memory_space;
			total_available_space = 0;
		} else if (*(u8 *) current_base == MEMORY_CLEAR) {
			total_available_space += memory_space;
			if (current_base + actual_size >= BASE_STACK_LOCATION - 2) {
				return 0;
			}
			if (total_available_space >= actual_size) { // Yay, got enough space, allocate it
				current_base -= total_available_space - memory_space;

				// TODO: maybe use this?
				// (8 - __builtin_clz(actual_size))
				for (u8 ii = 0; ii < 32; ++ii) {
					if (actual_size >> ii != 1) continue;
					u8 *next_val = (u8 *) (current_base + actual_size);
					int temp = -1;
					int should_set_next_val = *next_val != MEMORY_IN_USE && *next_val != MEMORY_CLEAR;
					if (*next_val != MEMORY_IN_USE && *next_val != MEMORY_CLEAR) {
						*(u8 *) (current_base + actual_size) = MEMORY_CLEAR;
						*(u8 *) (current_base + actual_size + 1) = MAX_SINGLE_MALLOC_SIZE_SHIFT;
					}

					for (int iii = 0; iii < MAX_NUMBER_OF_MALLOCS; ++iii) {
						if (address_book[iii] !=
							0) { // Double check that when we set the next free zone, we aren't stepping on some other memory
							if ((u8 *) address_book[iii] == next_val) {
								should_set_next_val = TRUE;
							}
						} else if (temp == -1) {
							temp = iii;
							address_book[iii] = current_base;
							if (should_set_next_val == TRUE) {
								break;
							}
						}
					}
					if (temp == -1) {
						kprint("Out of address book space\n");
						return 0;
					}

					(*(u8 *) (current_base + 1)) = ii;
					*(u8 *) current_base = 0xF0;
					return current_base + 2;
				}
			} else {
				current_base += memory_space;
			}
		} else {
			kprint("Memory corrupted\n");
			OS_Abort();
		}
	}

	return 0;
}

BOOL OS_Free(MEMORY m) {
	m -= 2;
	u8 *current_base = (u8 *) m;
	if (*current_base != MEMORY_IN_USE) return FALSE;
//	int size = 0x1 << (*(u8 *) (current_base + 1));
	int is_in_address_book = FALSE;
	for (int i = 0; i < MAX_NUMBER_OF_MALLOCS; ++i) {
		if (address_book[i] == (usize) current_base) {
			is_in_address_book = TRUE;
			address_book[i] = 0;
			break;
		}
	}
	if (!is_in_address_book) return FALSE;
	*current_base = MEMORY_CLEAR;
	return TRUE;
}

void OS_InitSem(int s, int n) {
	semaphore_buf[s] = (semaphore) {.count = n, .tail = NULL, .head = NULL};
}

/// Going to assume that DEVICEs must always reserve semaphores first, so that they don't block.
void OS_Wait(int s) {
	// The interrupts look really weird... Basically we can't interrupt between checking for semaphore number and setting it
	// Actually we can, just need a check afterwards TODO: maybe remove the interrupts

	process_t *process = &processes[process_index_from_pid(currently_running_process_pid)];

	asm volatile ("cli");
	pid_ll *queue_pos = NULL;
	if (semaphore_buf[s].count <= 0) { // Put ourselves in line

		if (process->scheduling_level == DEVICE) {
			// Well well well, someone messed up
			kprint("Well well well, someone messed up, ur device didn't grab the semaphore in time, now everything breaks.\n");
			OS_Abort();
		} else if (process->scheduling_level == PERIODIC) {
			// No need to be put on queue if sporadic, just wait till it's your turn again
			queue_pos = (pid_ll *) OS_Malloc(sizeof(pid_ll));
			queue_pos->next = NULL;
			queue_pos->pid = currently_running_process_pid;
			queue_pos->valid = TRUE;
			if (semaphore_buf[s].tail != NULL) {
				semaphore_buf[s].tail->next = queue_pos;
			} else { // If tail is NULL, head must also be NULL
				semaphore_buf[s].head = queue_pos;
			}
			semaphore_buf[s].tail = queue_pos;
		}

		do {
			asm volatile ("sti");
			OS_Yield();
			asm volatile ("cli");
		} while (semaphore_buf[s].count <= 0);
	}
	__atomic_fetch_sub(&semaphore_buf[s].count, 1, __ATOMIC_SEQ_CST);
	if (queue_pos != NULL) {
		queue_pos->valid = FALSE;
	}
	asm volatile ("sti");
	// It's fine if it is interrupted here
	register_bit(process->semaphores, s);

}

void cleanup_semaphore_buf(int s) {
	// Cleanup semaphore
	pid_ll *curr = semaphore_buf[s].head;
	pid_ll *old = NULL;
	while (curr != NULL) {
		if (curr->valid == FALSE) {
			if (old != NULL) {
				old->next = curr->next;
				OS_Free((MEMORY) curr);
				curr = old->next;
			} else {
				semaphore_buf[s].head = curr->next;
				OS_Free((MEMORY) curr);
				curr = semaphore_buf[s].head;
			}
			continue;
		}
		old = curr;
		curr = curr->next;
	}
	if (semaphore_buf[s].head == NULL) {
		semaphore_buf[s].tail = NULL;
	}
}

void OS_Signal(int s) {
	process_t *process = &processes[process_index_from_pid(currently_running_process_pid)];
	if (unregister_bit(process->semaphores, s)) { // Owns the semaphore before
		asm volatile ("cli");
		__atomic_fetch_add(&semaphore_buf[s].count, 1, __ATOMIC_SEQ_CST);

		// Maybe we don't need to loop twice. It is clearer and easier to reason this way though
		cleanup_semaphore_buf(s);
		asm volatile ("sti");
		if (process->scheduling_level <= PERIODIC) { // Nothing is higher priority than us. (Devices never wait)
			return;
		}

		pid_ll *curr = semaphore_buf[s].head;
		while (curr != NULL) { // Someone valid is waiting
			process_t *other = &processes[process_index_from_pid(curr->pid)];
			if (PPP[PPP_index] == other->n) { // If the current periodic process is in queue for resource
				curr->valid = FALSE;
				OS_Yield();
				return;
			}
			curr = curr->next;
		}
	}
}

FIFO OS_InitFiFo() {
	for (int i = 0; i < MAXFIFO; ++i) {
		if (fifo_collection[i].length == -1) {
			fifo_collection[i].length = 0;
			return i + 1;
		}
	}
	return INVALIDFIFO;
}

void OS_Write(FIFO f, int val) {
	fifo_buf *buf = get_fifo(f);
	int next_elem_idx = (buf->idx + buf->length) % FIFOSIZE;
	if (buf->length < FIFOSIZE) { // Not at full capacity
		buf->length++;
	} else {
		buf->idx++;
	}
	buf->data[next_elem_idx] = val;
}

BOOL OS_Read(FIFO f, int *val) {
	fifo_buf *buf = get_fifo(f);
	if (buf->length > 0) {
		*val = buf->data[buf->idx];
		buf->idx = (buf->idx + 1) % FIFOSIZE;
		buf->length--;
		return TRUE;
	} else {
		return FALSE;
	}
}
