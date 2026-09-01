#include "common.h"
#include "code_2c054.h"

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B854);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B8E4);

/* StreamTask's dtor override (StreamTaskMethods slot +0x00C). Ticks the
 * sub-object at +0xB4 (its own slot +0x004, self-only), then chains into
 * the base task class's own dtor (func_8003DFBC()->dtor), the same
 * "ownDtorChain" shape documented in code_171e0.h. */
void *func_8003B9DC(StreamTask *self) {
    self->unkB4->methods->slot4(self->unkB4);
    return func_8003DFBC()->dtor(self);
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BA38);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BA58);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BAB4);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BB5C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BC14);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BCF4);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BD10);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BD74);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BDAC);

void func_8003BDE4(void) {
}

void func_8003BDEC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BDF4);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE5C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE64);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE6C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE74);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE7C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE84);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE94);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BF10);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C008);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C11C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C1DC);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C238);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C3D0);
