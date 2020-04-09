#include "../../kernel/kernel.h"
#include "../../drivers/screen.h"
#include "../../util/debugging.h"
#include "memory_wiper.h"

void memory_wiper_main() {
	MEMORY position1 = OS_Malloc(0x1000);
	dbg_hex("Position1: ", position1);
	MEMORY position2 = OS_Malloc(0x1000);
	dbg_hex("Position2: ", position2);
	MEMORY position3 = OS_Malloc(0x100);
	dbg_hex("Position3: ", position3);
	int res = OS_Free(position1);
	dbg("Result: ", res);
	MEMORY position4 = OS_Malloc(0x100);
	dbg_hex("Position4: ", position4);
	MEMORY position5 = OS_Malloc(0x100);
	dbg_hex("Position5: ", position5);
	res = OS_Free(position4);
	dbg("Result: ", res);
	res = OS_Free(position5);
	dbg("Result: ", res);
	MEMORY position6 = OS_Malloc(0x1000);
	dbg_hex("Position6: ", position6);
	res = OS_Free(position6);
	dbg("Result: ", res);
	MEMORY position7 = OS_Malloc(0x2000);
	dbg_hex("position7: ", position7);
	res = OS_Free(position2);
	dbg("Result: ", res);
	MEMORY position8 = OS_Malloc(0x2000);
	dbg_hex("position8: ", position8);
	OS_Terminate();
}
