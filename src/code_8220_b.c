#include "common.h"
#include "code_8220.h"

void func_80018288(BasicClassListNode **head)
{
    BasicClassListNode *node = *head;

    while (node != NULL) {
        BasicClassListNode *cur = node;
        node = node->next;
        func_80017CFC(cur);
    }
}

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

void func_800183A0(BasicClass **outValue, BasicClassListNode **cursor)
{
    if (*cursor != NULL) {
        *outValue = (*cursor)->value;
        *cursor = (*cursor)->next;
    } else {
        *outValue = NULL;
    }
}

void func_800183DC(BasicClass **array, s32 count)
{
    if (count-- > 0) {
        do {
            *array = (BasicClass *)(*array)->methods->release(*array);
            array++;
        } while (count-- > 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_8001844C);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018458);

INCLUDE_ASM("asm/nonmatchings/code_8220_b", func_80018464);

void func_8001934C(void *arg0, void *arg1)
{
    u8 *a = (u8 *)arg0;

    if (*(s32 *)((u8 *)arg1 + 0x1C) != 0) {
        a[7] = a[7] | 0x2;
    } else {
        a[7] = a[7] & 0xFD;
    }

    if (D_8008E248 != 0) {
        a[7] = a[7] | 0x1;
    } else {
        a[7] = a[7] & 0xFE;
    }

    *((u8 *)arg1 + 0x14) = a[3];
    *((u8 *)arg1 + 0x15) = a[7];
}

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

void func_80019710(void *dst)
{
    __asm__ volatile (
        "swc2 $12, 0x8(%0)\n\t"
        "swc2 $13, 0x14(%0)\n\t"
        "swc2 $14, 0x20(%0)"
        : : "r" (dst) : "memory");
}

void func_80019724(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x14;

    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0xc(%0)\n\t"
            "swc2 $14, 0x10(%0)"
            : : "r" (dst) : "memory");
    } else {
        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}

void func_8001974C(void *dst, s32 flag)
{
    char *p = (char *)dst + 0x20;

    if (flag) {
        __asm__ volatile (
            "swc2 $12, 0x8(%0)\n\t"
            "swc2 $13, 0x10(%0)\n\t"
            "swc2 $14, 0x18(%0)"
            : : "r" (dst) : "memory");
    } else {
        __asm__ volatile (
            "swc2 $14, 0x0(%0)"
            : : "r" (p) : "memory");
    }
}
