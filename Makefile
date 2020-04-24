C_SOURCES = $(wildcard kernel/*.c drivers/*.c cpu/*.c util/*.c libc/*.c applications/counter/*.c applications/reminder/*.c applications/idle/*.c applications/memory_wiper/*.c applications/sema/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h cpu/*.h util/*.h libc/*.h applications/counter/*.h applications/reminder/*.h   applications/idle/*.h applications/memory_wiper/*.h applications/sema/*.h)
# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o cpu/interrupt.o cpu/gdt_from_c.o}

# Change this if your cross-compiler is somewhere else
CC = /usr/bin/gcc
GDB = /usr/bin/gdb
LD = /usr/bin/ld -m elf_i386
# -g: Use debugging symbols in gcc
CFLAGS = -g -m32 -ffreestanding -fno-pie -Wall -Wextra -fno-exceptions

# First rule is run by default
os-image.bin: boot/bootsect.bin kernel.bin
	cat $^ > os-image.bin
	qemu-img resize os-image.bin +10M

# '--oformat binary' deletes all symbols as a collateral, so we don't need
# to 'strip' them manually on this case
kernel.bin: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x10000 $^ --oformat binary

# Used for debugging purposes
kernel.elf: boot/kernel_entry.o ${OBJ}
	${LD} -o $@ -Ttext 0x10000 $^

run: os-image.bin
	qemu-system-i386 -drive file=os-image.bin,format=raw,index=0,media=disk -m 256M

# Open the connection to qemu and load our kernel-object file with symbols
debug: os-image.bin kernel.elf
	qemu-system-i386 -S -s -drive file=os-image.bin,format=raw,index=0,media=disk -m 256M &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

%.bin: %.asm
	nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf kernel/*.o boot/*.bin drivers/*.o boot/*.o cpu/*.o util/*.o libc/*.o applications/*/*.o
