
#include "idle.h"
#include "../../util/debugging.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
void idle_main() {
	unsigned long stuff = 0;
	while (1) {
		for (int i2 = 0; i2 < 80000000; i2 += 2) {
			// Waste some clocks
			i2 -= 1;
		}
		stuff++;
		dbg("IDLEING TIME: ", (int)stuff);
	}
}
#pragma clang diagnostic pop