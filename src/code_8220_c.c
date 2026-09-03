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
/* STALL snapshot round 2 -- see docs/match-reports/func_800197C4.md.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Residue is
 * two register-identity choices ($a2 vs $a1 for the OT high-byte mask,
 * cascading to the second reload's register) plus one load-delay-slot
 * filler: retail forms `$s1 + 0x14` (= arg0 + 0x14) and this body has
 * nothing there. VERIFIED across all 8 siblings: the filler's offset
 * always equals (highest self-relative offset touched anywhere in the
 * CALLS branch) + (the access width at that offset) -- i.e. one byte past
 * the last field of `self` the function ever reads. Tried: computing that
 * "one past" pointer as an unconditionally-live local (cross-branch) --
 * forces a 4th callee-saved register (drift, 0/54). Tried: computing it
 * only within the `if` branch with (void)-cast non-use -- eliminated by
 * -O2, matching func_8001A268's same finding for a genuinely unused local.
 * Not cracked.
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

/* A 2-s16 pair (alignment 2, not 4) -- see func_8001A268's stall report for
 * why this is needed even at accidentally-4-aligned offsets. */
typedef struct {
    s16 x, y;
} Vec2s16_98;

#if 0
/* STALL snapshot round 2 -- see docs/match-reports/func_8001989C.md.
 * Instruction-exact (asm-differ: zero inserted, zero deleted -- ONE
 * differing line, the delay-slot filler below). TWO residues stack here,
 * both already documented: (a) the shared family residue ($a2 vs $a1 for
 * the OT mask, plus missing `addiu $v0,$s1,0x1c` = self+0x1c, matching
 * the cross-sibling formula -- last touched self field is +0x18, a
 * PolyUV4, width 4, end 0x1c); (b) this function's OWN pre-existing
 * self/prim register swap ($s1=prim, $s2=self, opposite of retail and
 * every OTHER sibling), already investigated exhaustively in this
 * report's round-1 attempts -- re-tried swapping the local declaration
 * order here too (prim before self) and got the SAME regression as
 * round 1 (address drift), confirming it's not fixed by the bitfield
 * rewrite either. Not cracked.
 */
void func_8001989C(void *arg0, void *arg1) {
    u8 *self = (u8 *)arg0;
    u8 *prim = (u8 *)arg1;

    if (*(s32 *)(prim + 0x78) == 0) {
        ((OtTag *)self)->addr = (*(OtTag **)(prim + 0x30))->addr;
        (*(OtTag **)(prim + 0x30))->addr = (u32)self;
    } else {
        func_8001A380(D_8008ACD0, prim, self + 0x4, 0, 0, 0);
        func_8001A3EC((PolyVtx **)(prim + 0x88), (PolyVtx **)(prim + 0xA4),
                      (PolyUV4 *)(self + 0x8), (PolyUV4 *)(self + 0x10),
                      (PolyUV4 *)(self + 0x18));

        *(u16 *)(*(u8 **)(prim + 0x88) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x8C) + 0xA) = *(u8 *)(self + 0xF);
        *(u16 *)(*(u8 **)(prim + 0x90) + 0xA) = *(u8 *)(self + 0x17);

        *(Vec2s16_98 *)(*(u8 **)(prim + 0x88) + 0xC) = *(Vec2s16_98 *)(self + 0x4);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x8C) + 0xC) = *(Vec2s16_98 *)(self + 0xC);
        *(Vec2s16_98 *)(*(u8 **)(prim + 0x90) + 0xC) = *(Vec2s16_98 *)(self + 0x14);

        func_8001AD54(self, D_8008ACD0);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_8001989C);

#if 0
/* STALL snapshot round 2 -- see docs/match-reports/func_800199EC.md.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class as func_800197C4: $a2 vs $a1 for the OT high-byte mask,
 * plus one missing `addiu $v0,$s1,0x20` = arg0 + 0x20, one byte past the
 * last self field this function touches (arg0+0x1E, a u16). Matches the
 * verified cross-sibling formula in func_800197C4.md. Not cracked.
 */
void func_800199EC(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008ACD0, arg1, (u8 *)arg0 + 0x4, 1, *(u16 *)((u8 *)arg0 + 0xE), *(u16 *)((u8 *)arg0 + 0x16));
        func_8001A3EC((PolyVtx **)((u8 *)arg1 + 0x88), (PolyVtx **)((u8 *)arg1 + 0xA4),
                      (PolyUV4 *)((u8 *)arg0 + 0x8), (PolyUV4 *)((u8 *)arg0 + 0x10),
                      (PolyUV4 *)((u8 *)arg0 + 0x18));

        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x88) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x8C) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x90) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x88) + 0x8) = *(u16 *)((u8 *)arg0 + 0xC);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x8C) + 0x8) = *(u16 *)((u8 *)arg0 + 0x14);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x90) + 0x8) = *(u16 *)((u8 *)arg0 + 0x1C);

        func_8001B6B4(arg0, D_8008ACD0);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_800199EC);

#if 0
/* STALL snapshot round 2 -- see docs/match-reports/func_80019B24.md.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class as func_800197C4 in this unit: $a2 vs $a1 for the OT
 * high-byte mask (cascading register renames), plus one missing
 * load-delay-slot filler `addiu $v0,$s1,0x18` = arg0 + 0x18, which is
 * exactly one byte past uv3 (arg0+0x14, a PolyUV4, the last arg0 field
 * this function's calls branch touches) -- see func_800197C4.md for the
 * verified cross-sibling formula and the two ruled-out hypotheses for
 * reproducing it. Not cracked.
 */
void func_80019B24(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008AEE8, arg1, (u8 *)arg0 + 0x4, 0, 0, 0);
        func_8001A4C0((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0xC, (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x14);
        func_8001A8D4(arg0, D_8008AEE8);
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019B24);

INCLUDE_ASM("asm/nonmatchings/code_8220_c", func_80019C04);

#if 0
/* STALL snapshot round 2 -- see docs/match-reports/func_80019D84.md.
 * Instruction-exact (asm-differ: zero inserted, zero deleted). Same
 * residue class: $a2 vs $a1 for the OT high-byte mask, plus one missing
 * `addiu $v0,$s1,0x28`. This is the first sibling where the raw
 * "last-field-offset + access-width" formula from func_800197C4.md does
 * NOT land exactly on the filler: the highest arg0 field this function
 * touches is +0x24 (a u16 read, raw end 0x26), but the filler is 0x28.
 * 0x28 = align-up-to-4(0x26). Every other sibling's raw sum was already a
 * multiple of 4, so this is the first case that distinguishes "one byte
 * past the last field" from "start of the next 4-byte-aligned slot" --
 * the latter is what actually matches here. Worth re-checking the other
 * seven against this refined rule rather than the raw one. Not cracked.
 */
void func_80019D84(void *arg0, void *arg1) {
    if (*(s32 *)((u8 *)arg1 + 0x78) == 0) {
        ((OtTag *)arg0)->addr = (*(OtTag **)((u8 *)arg1 + 0x30))->addr;
        (*(OtTag **)((u8 *)arg1 + 0x30))->addr = (u32)arg0;
    } else {
        func_8001A380(D_8008AEE8, arg1, (u8 *)arg0 + 0x4, 1, *(u16 *)((u8 *)arg0 + 0xE), *(u16 *)((u8 *)arg0 + 0x16));
        func_8001A4C0((u8 *)arg1 + 0x94, (u8 *)arg1 + 0xA4, (u8 *)arg0 + 0x8,
                      (u8 *)arg0 + 0x10, (u8 *)arg0 + 0x18, (u8 *)arg0 + 0x20);

        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0xA) = *(u16 *)((u8 *)arg0 + 0x1E);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x94) + 0x8) = *(u16 *)((u8 *)arg0 + 0xC);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x98) + 0x8) = *(u16 *)((u8 *)arg0 + 0x14);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0x9C) + 0x8) = *(u16 *)((u8 *)arg0 + 0x1C);
        *(u16 *)(*(u8 **)((u8 *)arg1 + 0xA0) + 0x8) = *(u16 *)((u8 *)arg0 + 0x24);

        func_8001BAB4(arg0, D_8008AEE8);
    }
}
#endif

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
