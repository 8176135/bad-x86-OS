//
// Created by elsecaller on 4/6/20.
//

#include "task.h"

/// DOESN'T WORK.... yet
void switch_to_user_mode() {
    asm volatile("\
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
                   \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pop %eax; \
     or $0x200, %eax; \
     push %eax; \
     pushl $0x1B; \
     push $1f; \
     iret; \
   1: \
     ");
}
