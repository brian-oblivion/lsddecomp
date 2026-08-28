#include "common.h"
#include "class_16334.h"

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025B34);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025BA0);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025C30);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025C84);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025CC4);

INCLUDE_ASM("asm/nonmatchings/class_16334", func_80025D10);

void func_80025E14(void) {
}

void func_80025E1C(void) {
    Block64 local;
    u32 *dst;
    u32 *src;
    s32 i;

    dst = D_8008B388;
    local = D_80010764;
    i = 0;
    src = local.w;
    for (; i < 16; i++) {
        *dst++ = *src++;
    }
}

void func_80025E94(void) {
}

PadMethods *func_80025E9C(void) {
    return &D_8006D370;
}
