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

void func_8001A224(void *arg0, void *arg1, s32 kind)
{
    u8 *src = (u8 *)arg1 + 0x18;
    u8 *dst0 = (u8 *)arg0;
    u8 *dst1 = (kind == 4) ? (u8 *)arg1 + 0xF0 : (u8 *)arg1 + 0xA8;

    while (kind-- > 0) {
        *(void **)dst1 = src;
        *(void **)dst0 = src;
        src += 0x18;
        dst1 += 4;
        dst0 += 4;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A268);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A380);

/*
 * Copies three unaligned 8-byte fields (arg1[0]/[4]/[8] -> arg0[0]/[4]/[8])
 * and three unaligned 4-byte fields (arg2/arg3/arg4 -> arg0[i]+0x10) via
 * lwl/lwr+swl/swr. No prologue/frame in retail (frameless leaf), and the
 * whole body is straight-line with no branches, so this is written as one
 * raw-register __asm__ block (same technique as func_800195EC in
 * code_8220_b, minus the noreorder bracket that function needed for its
 * internal branches -- none needed here). arg4 arrives on the stack per
 * the o32-ish calling convention (5th integer arg) and is read directly
 * from 0x10($sp) rather than through a C-level operand.
 */
void func_8001A3EC(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    __asm__ volatile (
        "lw $3, 0x0($4)\n\t"
        "lw $2, 0x0($5)\n\t"
        "lw $8, 0x10($sp)\n\t"
        "lwl $9, 0x3($2)\n\t"
        "lwr $9, 0x0($2)\n\t"
        "lwl $10, 0x7($2)\n\t"
        "lwr $10, 0x4($2)\n\t"
        "swl $9, 0x3($3)\n\t"
        "swr $9, 0x0($3)\n\t"
        "swl $10, 0x7($3)\n\t"
        "swr $10, 0x4($3)\n\t"
        "lw $3, 0x4($4)\n\t"
        "lw $2, 0x4($5)\n\t"
        "nop\n\t"
        "lwl $9, 0x3($2)\n\t"
        "lwr $9, 0x0($2)\n\t"
        "lwl $10, 0x7($2)\n\t"
        "lwr $10, 0x4($2)\n\t"
        "swl $9, 0x3($3)\n\t"
        "swr $9, 0x0($3)\n\t"
        "swl $10, 0x7($3)\n\t"
        "swr $10, 0x4($3)\n\t"
        "lw $3, 0x8($4)\n\t"
        "lw $2, 0x8($5)\n\t"
        "nop\n\t"
        "lwl $5, 0x3($2)\n\t"
        "lwr $5, 0x0($2)\n\t"
        "lwl $9, 0x7($2)\n\t"
        "lwr $9, 0x4($2)\n\t"
        "swl $5, 0x3($3)\n\t"
        "swr $5, 0x0($3)\n\t"
        "swl $9, 0x7($3)\n\t"
        "swr $9, 0x4($3)\n\t"
        "lw $2, 0x0($4)\n\t"
        "lwl $3, 0x3($6)\n\t"
        "lwr $3, 0x0($6)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        "lw $2, 0x4($4)\n\t"
        "lwl $3, 0x3($7)\n\t"
        "lwr $3, 0x0($7)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        "lw $2, 0x8($4)\n\t"
        "lwl $3, 0x3($8)\n\t"
        "lwr $3, 0x0($8)\n\t"
        "nop\n\t"
        "swl $3, 0x13($2)\n\t"
        "swr $3, 0x10($2)\n\t"
        : : : "$2", "$3", "$5", "$8", "$9", "$10", "memory");
}

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A4C0);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A54C);
