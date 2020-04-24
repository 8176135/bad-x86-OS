
#include "../../util/debugging.h"
#include "sema.h"
#include "../../kernel/kernel.h"

int sema_main() {

	int arg = OS_GetParam();

	if (arg == 0) { // Is parent
		FIFO fifo_handler = OS_InitFiFo();
		OS_Create((void *) sema_main, fifo_handler, PERIODIC, 1);
		OS_InitSem(2, 1);
		OS_Wait(2);
		for (int i = 0; i < 200; ++i) {
			for (int j = 0; j < 10000000; ++j) {
			}
			dbg("STEP: ",i);
		}
//		OS_Yield();
		OS_Write(fifo_handler, 111222333);
		OS_Signal(2);
	} else {
		int val;
		OS_Wait(2);
//		while (!OS_Read(arg, &val)) { // Keep reading until we get something
////			OS_Yield();
//		}
		if (!OS_Read(arg, &val)) {
			dbg("\n\n\n\nFAIL@!!!!#@!@!@!@!\n\n\n\n", 1);
		}
		dbg("\nFrom child: ", val);
	}

	OS_Terminate();
	return 0;
}