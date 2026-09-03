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

#if 0
/* STALL snapshot -- see docs/match-reports/func_800197C4.md. Head pass,
 * round 13: this reaches the CORRECT INSTRUCTION COUNT AND LENGTH (no
 * inserted or deleted instructions anywhere in the function). The residue
 * is two register-identity choices plus one delay-slot filler retail has
 * and this does not. Two things got it here and both generalise to the
 * seven sibling functions in this family:
 *   1. The OT splice is a 24-BIT BITFIELD write (`OtTag.addr`), not
 *      hand-written `& 0xFF000000` / `& 0x00FFFFFF` masking. Same value,
 *      different register allocation.
 *   2. The OT expression must be RE-EVALUATED, not cached in a local --
 *      retail re-reads `arg1->unk30` for the second store. That is macro
 *      argument semantics (Psy-Q's `addPrim(ot, p)` expands `ot` twice).
 * Still open: retail fills a load-delay slot with `addiu $v0, $s1, 0x14`,
 * i.e. it computes `arg0 + 0x14` somewhere in this branch; this body has
 * nothing to schedule there and GCC emits a `nop`.
 */
void func_800197C4(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008ACD0, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        func_8001A3EC((PolyVtx **)((u8 *)arg1 + 0x88), (PolyVtx **)((u8 *)arg1 + 0xA4),
                      (PolyUV4 *)((u8 *)arg0 + 0x8), (PolyUV4 *)((u8 *)arg0 + 0xC),
                      (PolyUV4 *)((u8 *)arg0 + 0x10));
        func_8001A564(arg0, D_8008ACD0);
    }
}
#endif

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
void func_8001A3EC(PolyVtx **dst, PolyVtx **src, PolyUV4 *uv0, PolyUV4 *uv1,
                   PolyUV4 *uv2) {
    dst[0]->xy = src[0]->xy;
    dst[1]->xy = src[1]->xy;
    dst[2]->xy = src[2]->xy;
    dst[0]->uv = *uv0;
    dst[1]->uv = *uv1;
    dst[2]->uv = *uv2;
}


INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A4C0);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001A54C);
