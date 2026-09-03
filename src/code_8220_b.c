#include "common.h"
#include "code_8220.h"

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018288);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", BasicClass__func_182cc);

void BasicClass__func_18350(void) {
}

void BasicClass__func_18358(BasicClass *self, void *arg1, s32 arg2)
{
    if (arg2 == 1) {
        self->methods->removeChild(self, (BasicClass *)arg1);
    }
}

BasicClassMethods *func_80018390(void)
{
    return &D_8006B58C;
}

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_800183A0);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_800183DC);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_8001844C);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018458);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018464);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_8001934C);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_800193C0);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_800194A4);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_800195EC);

void func_800196D4(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0xc(%0)\n\t"
        "swc2 $14, 0x10(%0)"
        : : "r" (dst) : "memory");
}

void func_800196E8(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x10(%0)\n\t"
        "swc2 $14, 0x18(%0)"
        : : "r" (dst) : "memory");
}

void func_800196FC(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x10(%0)\n\t"
        "swc2 $14, 0x18(%0)"
        : : "r" (dst) : "memory");
}

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80019710);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80019724);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_8001974C);
