#include "common.h"
#include "code_2c054.h"

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B854);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003B8E4);

void func_8003B9DC(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    func_8003DFBC()->slot0C(self);
}

void func_8003BA38(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}

void func_8003BA58(StreamTaskObj *self, s32 a1, s32 arg2, s32 typeLookup, s32 flag) {
    self->unkB8 = arg2;
    self->unkBC = typeLookup;
    self->unkC0 = flag;
    func_8003DFBC()->slot44(self, a1, 0);
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BAB4);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BB5C);

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BC14);

void func_8003BCF4(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BD10);

void func_8003BD74(StreamTaskObj *self) {
    func_8003DFBC()->slot80(self);
}

void func_8003BDAC(StreamTaskObj *self) {
    func_8003DFBC()->slot84(self);
}

void func_8003BDE4(void) {
}

void func_8003BDEC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2c054", func_8003BDF4);

void func_8003BE5C(StreamTaskObj *self, s32 a1) {
    self->unkC4 = a1;
}

void func_8003BE64(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}

void func_8003BE6C(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}

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
