
#include "../../kernel/kernel.h"
#include "../../drivers/screen.h"
#include "reminder.h"

void reminder_main() {
	for (int i = 0; i < 1000000000; ++i) {
		kprint("\n+----------------------------+\n");
		kprint(  "|       -- REMINDER: --      |\n");
		kprint(  "| Check your sitting posture |\n");
		kprint(  "+----------------------------+\n");
		OS_Yield();
	}
}
