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

void BasicClass__func_182cc(BasicClass *self, s32 arg1)
{
    BasicClassListNode *cursor = self->parentRefs;
    BasicClass *value;

    for (func_800183A0(&value, &cursor); value != NULL; func_800183A0(&value, &cursor)) {
        value->methods->slot38(value, self, arg1);
    }
}

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

/*
 * GTE-heavy "handwritten function" per splat's own extraction (asm comment).
 * rtpt/nclip/avsz3 have no mnemonic support in this pinned binutils (it
 * knows swc2/lwc2/cfc2 generically but not the GTE "cofun" ops), so they
 * are emitted as raw .word encodings taken directly from retail's own
 * instruction bytes. There is no C form for this sequence at all -- see
 * docs/DECOMPILATION_LEARNINGS.md's GTE-store-leaf entries, of which this
 * is a larger, branching cousin. $a0/$a1 are used via their natural
 * calling-convention registers rather than asm operands (this is the
 * function's only statement, so they still hold the incoming parameters
 * unmodified when it starts); the trailing `jr $ra`/`nop` is deliberately
 * OMITTED from the asm text and left for GCC's own implicit epilogue to
 * generate, which is byte-identical to retail's and avoids a doubled
 * return sequence.
 */
s32 func_800195EC(void *arg0, void *arg1)
{
    (void)arg0;
    (void)arg1;
    __asm__ volatile (
        ".set\tnoreorder\n\t"
        "sw $zero, 0x78($5)\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4A280030\n\t"     /* rtpt */
        "lbu $2, 0x14($5)\n\t"
        "nop\n\t"
        "sb $2, 0x3($4)\n\t"
        "addiu $2, $5, 0x5c\n\t"
        "cfc2 $12, $31\n\t"
        "addi $13, $zero, 0x4\n\t"
        "sll $13, $13, 16\n\t"
        "and $12, $12, $13\n\t"
        "sw $12, 0x0($2)\n\t"
        "lw $3, 0x5c($5)\n\t"
        "nop\n\t"
        "beqz $3, 1f\n\t"
        " lui $2, 0x4\n\t"
        "bne $3, $2, 3f\n\t"
        " ori $2, $zero, 0x1\n\t"
        "sw $2, 0x78($5)\n\t"
        "1:\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4B400006\n\t"     /* nclip */
        "addiu $2, $5, 0x28\n\t"
        "swc2 $24, 0x0($2)\n\t"
        "lw $2, 0x28($5)\n\t"
        "nop\n\t"
        "blez $2, 2f\n\t"
        " addiu $2, $5, 0x24\n\t"
        "swc2 $8, 0x0($2)\n\t"
        "lw $2, 0x24($5)\n\t"
        "nop\n\t"
        "slti $2, $2, 0x1000\n\t"
        "beqz $2, 3f\n\t"
        " ori $2, $zero, 0x1\n\t"
        "nop\n\t"
        "nop\n\t"
        ".word 0x4B58002D\n\t"     /* avsz3 */
        "addiu $2, $5, 0x20\n\t"
        "swc2 $7, 0x0($2)\n\t"
        "addiu $4, $5, 0x60\n\t"
        "addiu $3, $5, 0x64\n\t"
        "addiu $2, $5, 0x68\n\t"
        "swc2 $12, 0x0($4)\n\t"
        "swc2 $13, 0x0($3)\n\t"
        "swc2 $14, 0x0($2)\n\t"
        "lw $3, 0x20($5)\n\t"
        "lw $4, 0x4($5)\n\t"
        "addu $2, $zero, $zero\n\t"
        "srav $3, $3, $4\n\t"
        "lw $4, 0x0($5)\n\t"
        "sll $3, $3, 2\n\t"
        "addu $3, $3, $4\n\t"
        "j 3f\n\t"
        " sw $3, 0x30($5)\n\t"
        "2:\n\t"
        "ori $2, $zero, 0x1\n\t"
        "3:\n\t"
        ".set\treorder\n\t"
        : : : "$2", "$3", "$4", "$5", "$12", "$13", "memory");
}

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
