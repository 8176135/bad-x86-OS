Got this x86 OS working with an amalgamation of a bunch of different books, tutorials and modified code by me.

The following directories contain a combination of edited code from different books and tutorials: 
Mostly bootstrap code needed to get the OS booting
- `boot/`
- `cpu/`
- `drivers/`

The following directories contain code that is 100% mine. (Exceptions are mentioned at the top of files):
- `kernel/`
- `libc/` - Except `strings.c`
- `util/`
- `applciation/`

50 source files in total + Makefile :D.
Start in `boot/bootsect.asm` if you want to read in the same order as the CPU 
Start in `kernel/kernel.c (kernel_main)` to just get straight to stuff relevant to this milestone

---

`make run` to run the CPU, requires `qemu-system-i386`, `gcc` and 32 bit stuff for `ld`
 (Whatever is needed to run this: `ld -m elf_i386`). And `nasm` 

Default run will start multiple processes to test process switching.
Look in `OS_Start` and comment out the OS_Creates for `counter_main`, and uncomment the `memory_wipe_main` line to test memory allocation algorithm
(You can have both running, but it will be difficult to see what memory_wipe_main outputted)

`make debug` to automatically attach gdb debugger. Beware that it will randomly lie to you, in both 16 bit and 32 bit mode.

`make clean` to remove compiled files, sometimes needed otherwise things might bug out
Rex