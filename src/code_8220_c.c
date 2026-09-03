#include "common.h"
#include "code_8220.h"

void func_80019774(void *dst, s32 flag)
{
    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x10(%0)\n\t"
            "swc2 $14, 0x18(%0)"
            : : "r" (dst) : "memory");
    } else {
        char *p = (char *)dst + 0x20;

        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}

void func_8001979C(void *dst, s32 flag)
{
    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x14(%0)\n\t"
            "swc2 $14, 0x20(%0)"
            : : "r" (dst) : "memory");
    } else {
        char *p = (char *)dst + 0x2c;

        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_800197C4);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001989C);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_800199EC);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019B24);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019C04);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019D84);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019EE4);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A064);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A224);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A268);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A380);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A3EC);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A4C0);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A54C);
