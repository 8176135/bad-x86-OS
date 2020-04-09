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

`make debug` to automatically attach gdb debugger. Beware that it will randomly lie to you, in both 16 bit and 32 bit mode.

Rex