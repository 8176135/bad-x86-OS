cmake_minimum_required(VERSION 3.17)
project(my-x86-os)

set(CMAKE_C_STANDARD 11)

include_directories(cpu)
include_directories(util)
include_directories(applications)
include_directories(drivers)
include_directories(kernel)
include_directories(libc)

add_executable(my-x86-os
        boot/32bit_print.asm
        boot/bootsect.asm
        boot/disk.asm
        boot/gdt.asm
        boot/kernel_entry.asm
        boot/print.asm
        boot/print_hex.asm
        boot/switch_pm.asm
        cpu/interrupt.asm
        cpu/isr.c
        cpu/isr.h
        cpu/ports.c
        cpu/ports.h
        cpu/timer.c
        cpu/timer.h
        cpu/type.h
        drivers/keyboard.c
        drivers/keyboard.h
        drivers/screen.c
        drivers/screen.h
        kernel/kernel.c
        kernel/kernel.h
        libc/function.h
        libc/mem.c
        libc/mem.h
        libc/rust_types.h
        libc/string.c
        libc/string.h applications/counter/counter.c applications/counter/counter.h util/read_std.c util/read_std.h kernel/execute.c kernel/execute.h kernel/task.c kernel/task.h cpu/tss.c cpu/tss.h cpu/descriptor_tables.c cpu/descriptor_tables.h util/debugging.c util/debugging.h applications/reminder/reminder.c applications/reminder/reminder.h applications/idle/idle.c applications/idle/idle.h applications/memory_wiper/memory_wiper.c applications/memory_wiper/memory_wiper.h applications/sema/sema.c applications/sema/sema.h)
