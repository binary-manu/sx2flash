#include <sx2flash/payload.h>

void jump_to_application(void) {
    asm("jmp 0x0000");
}
