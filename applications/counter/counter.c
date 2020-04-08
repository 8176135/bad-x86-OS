
#include "counter.h"
#include "../../drivers/screen.h"
#include "../../libc/string.h"
#include "../../kernel/kernel.h"

int counter_main() {

    char num[16];
    int arg = OS_GetParam();
    for (int i = 0; i < 100000000; ++i) {
        for (int i2 = 0; i2 < 10000000; i2 += 2) {
            // Waste some clocks
            i2 -= 1;
        }
        switch (arg) {
            case 0:
                kprint("Stuff: ");
                break;
            case 1:
                kprint("Other: ");
                break;
            default:
                kprint("Why: ");
        }
        int_to_ascii(i, num);
        kprint(num);
        kprint("\n");
    }
    return 0;
}
