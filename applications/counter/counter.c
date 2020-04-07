
#include "counter.h"
#include "../../drivers/screen.h"
#include "../../libc/string.h"

int counter_main() {
    char num[16];
    for (int i = 0; i < 10000000; ++i) {
        for (int i2 = 0; i2 < 100000000; ++i2) {
            // Waste some clocks
        }
        kprint("Stuff: ");
        int_to_ascii(i, num);
        kprint(num);
        kprint("\n");
    }
    return 0;
}
