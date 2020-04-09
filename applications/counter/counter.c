
#include "counter.h"
#include "../../drivers/screen.h"
#include "../../libc/string.h"
#include "../../kernel/kernel.h"

int counter_main() {

    char num[16];
    int arg = OS_GetParam();
    for (int i = 0; i < 100; ++i) {
        for (int i2 = 0; i2 < 20000000; i2 += 2) {
            // Waste some clocks
            i2 -= 1;
        }
        switch (arg) {
            case 0:
                kprint("Adolin: ");
                break;
            case 1:
                kprint("Bashin: ");
                break;
			case 2:
				kprint("Chanarach: ");
				break;
            case 3:
				kprint("Dalinar: ");
				break;
            case 4:
				kprint("Evi: ");
				break;
            case 5:
				kprint("Fleet: ");
				break;
            default:
                kprint("Shallan: ");
        }
        int_to_ascii(i, num);
        kprint(num);
        kprint("\n");
        if (i % 25 == 0 && i) {
			OS_Yield();
        }
    }
    OS_Terminate();
    return 0;
}
