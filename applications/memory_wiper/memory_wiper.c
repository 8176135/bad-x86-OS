#include "../../kernel/kernel.h"
#include "../../drivers/screen.h"
#include "memory_wiper.h"

void memory_wiper_main() {
	int position = OS_Malloc(0x1000);
	kprint("Stuff");
}
