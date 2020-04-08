
#include "counter.h"
#include "../../drivers/screen.h"
#include "../../libc/string.h"

int counter_main() {

    char num[16];
    for (int i = 0; i < 100000000; ++i) {
        for (int i2 = 0; i2 < 10000000; i2 += 2) {
            // Waste some clocks
            i2 -= 1;
        }
        kprint("Stuff: ");
        int_to_ascii(i, num);
        kprint(num);
        kprint("\n");
    }
    return 0;
}
