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

/* StreamTask's override of slot +0x040: resets the five fields also
 * reachable individually through func_8003BE5C..func_8003BE7C to their
 * default values. */
void func_8003BA38(StreamTask *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

/* StreamTask's override of slot +0x044: stashes three caller-supplied
 * values (the third from the stack, a 5th argument) into the object, then
 * chains into the base task class's own slot +0x044 with a literal 0 as
 * its own third argument. */
void func_8003BA58(StreamTask *self, s32 a1, s32 a2, s32 typeLookup, s32 flag) {
    self->unkB8 = a2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    func_8003DFBC()->slot44(self, a1, 0);
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BAB4);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BB5C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BC14);

/* StreamTask's override of slot +0x06C: always stores a1 verbatim, then
 * for non-negative a1 overwrites with a1*15 (sll by 4, then subu a1). */
void func_8003BCF4(StreamTask *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BD10);

/* StreamTask's override of slot +0x080: a pure passthrough to the base
 * task class's own copy of the same slot. */
s32 func_8003BD74(StreamTask *self) {
    return func_8003DFBC()->slot80(self);
}

/* StreamTask's override of slot +0x084: same passthrough shape as
 * func_8003BD74, one slot over. */
s32 func_8003BDAC(StreamTask *self) {
    return func_8003DFBC()->slot84(self);
}

void func_8003BDE4(void) {
}

void func_8003BDEC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BDF4);

/* StreamTask field setters, one per field also touched wholesale by
 * func_8003BA38's reset. */
void func_8003BE5C(StreamTask *self, s32 a1) {
    self->unkC4 = a1;
}

void func_8003BE64(StreamTask *self, s32 a1) {
    self->unkC8 = a1;
}

void func_8003BE6C(StreamTask *self, s32 a1) {
    self->unkCC = a1;
}

void func_8003BE74(StreamTask *self, s32 a1) {
    self->unkD0 = a1;
}

void func_8003BE7C(StreamTask *self, s32 a1) {
    self->unkD4 = a1;
}

/* Returns StreamTask's own class table, &D_8006E5F8 -- same shape as
 * code_171e0.h's func_80026C9C. Used by func_8003B854 (uncarved by this
 * runner) to fetch the ctor from slot +0x008. */
StreamTaskMethods *func_8003BE84(void) {
    return &D_8006E5F8;
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BE94);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BF10);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C008);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C11C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C1DC);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C238);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003C3D0);
