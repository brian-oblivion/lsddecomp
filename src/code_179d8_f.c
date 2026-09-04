/*
 * code_179d8_f -- functions 256..273 of the original 274-function code_179d8
 * monolith, 0x2673C..0x272C8 (vram 0x80035F3C..0x80036AC8), i.e. its very
 * tail.  Carved round 17 (2026-09-04) off the front of `code_179d8_tail`,
 * which keeps that name for the 36 functions still in front of this slice.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of the 18 clean, and NONE of the 18 is a trivial leaf -- every one is a
 * real body.  This is the densest-in-real-work unit carved this round.
 *
 * BLOCKED, stub reports already filed, do NOT spend attempts on these:
 *   addiu_at: func_80036230 (115 insn), func_80036528 (240 insn)
 * Both are large, which is the point of screening at carve time rather than
 * discovering it after a derivation.
 *
 * Owns NO switch jump table.  Both of this segment's jump-table owners
 * (func_80034690, func_800357B0) sit in the `code_179d8_tail` remainder in
 * FRONT of this slice -- that is where the cut was chosen, so that this unit
 * needs no rodata attach and the remainder carries that debt instead.
 *
 * Expect low-level driver-shaped code rather than class-framework code, as
 * elsewhere in code_179d8; confirm with tools/classtable.py, do not assume.
 */
#include "common.h"

/* 9 packed halfword fields, unpacked from two 16-bit-ish words (a0, a1).
 * Field semantics unknown -- named by offset per project convention. */
typedef struct {
    s16 unk0;  /* +0x0 */
    s16 unk2;  /* +0x2 */
    s16 unk4;  /* +0x4 */
    s16 unk6;  /* +0x6 */
    s16 unk8;  /* +0x8 */
    s16 unkA;  /* +0xA */
    s16 unkC;  /* +0xC */
    s16 unkE;  /* +0xE */
    s16 unk10; /* +0x10 */
} UnkStruct80035F3C;

void func_80035F3C(s32 a0, s32 a1, UnkStruct80035F3C *a2)
{
    int t;

    a2->unkA = a0 & 0x8000;
    t = a1 & 0x8000;
    a2->unkC = t;
    a2->unk10 = a1 & 0x4000;
    a2->unkE = a1 & 0x20;
    a2->unk0 = ((u16)a0 >> 8) & 0x7F;
    a2->unk2 = ((u16)a0 >> 4) & 0xF;
    a2->unk4 = a0 & 0xF;
    a2->unk6 = ((u32)a1 >> 6) & 0x7F;
    a2->unk8 = a1 & 0x1F;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80035F98);

/* func_80038D74 is defined in the uncarved psyq_SpuSetMute unit; a leading
 * `beqz $a0` / `beq $a0,1` / else dispatch on a mode, and it returns a real
 * value read from D_8006DD2C right before its own jr $ra. */
extern s32 func_80038D74(s32 a0);

s32 func_80036024(void)
{
    return func_80038D74(1);
}

s32 func_80036044(void)
{
    return func_80038D74(0);
}

/* Globals owned by the uncarved psyq_SpuSetMute unit, poked directly here. */
extern s32 D_8008E258;
extern s32 D_8008E25C;
extern void func_80036B20(s32 *a0);

s32 func_80036064(s32 a0)
{
    s32 neg = 0;
    u32 v1 = a0;
    s32 s0;
    s32 result;

    if ((s16)a0 < 0) {
        neg = 1;
        v1 = -a0;
    }
    if ((v1 & 0xFFFF) < 10) {
        D_8008E258 = 1;
        if (neg) {
            D_8008E25C = (s16)(v1 | 0x100);
        } else {
            D_8008E25C = (s16)v1;
        }
        s0 = (s16)v1;
        if (s0 == 0) {
            /* Both arms are the same call. Found by permuter search: a
             * plain `if (s0 == 0) func_80038D74(0);` merges a0's register
             * into v1's, dropping retail's extra `move v1,a0`. Keeping a0
             * referenced here (even in dead-equal branches) keeps the
             * allocator from reusing its register for v1, matching retail's
             * register footprint exactly. */
            if (a0) {
                func_80038D74(0);
            } else {
                func_80038D74(0);
            }
        }
        func_80036B20(&D_8008E258);
        result = s0;
    } else {
        result = -1;
    }
    return result;
}

/* D_8008E25C is stored as a full 32-bit sign-extended s16 value (see
 * func_80036064 above) but read back here through a 16-bit view. */
s32 func_80036108(void)
{
    return *(s16 *)&D_8008E25C;
}

extern s16 D_8008E260;
extern s16 D_8008E262;

/* a0, a1 are treated as signed 16-bit inputs (0..127-ish range going by the
 * /127 below) and rescaled to a signed 15-bit-ish range (*32767/127) before
 * being poked into the same SPU-ish struct func_80036064 above writes. */
void func_80036118(s32 a0, s32 a1)
{
    s16 sa0 = (s16)a0;
    s16 sa1 = (s16)a1;
    s32 *p = &D_8008E258;

    *p = 6;
    D_8008E260 = (s32)sa0 * 32767 / 127;
    D_8008E262 = (s32)sa1 * 32767 / 127;
    func_80036B20(p);
}

extern s32 D_8008E268;

void func_800361B0(s32 a0)
{
    s32 *p = &D_8008E258;

    *p = 0x10;
    D_8008E268 = (s16)a0;
    func_80036B20(p);
}

extern s32 D_8008E264;

void func_800361F0(s32 a0)
{
    s32 *p = &D_8008E258;

    *p = 8;
    D_8008E264 = (s16)a0;
    func_80036B20(p);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036230);

extern s16 D_8008E84C;

void func_800363FC(void)
{
    D_8008E84C = 2;
}

/* A 172 (0xAC)-byte record; only the fields this function touches are
 * named. D_800902E8 is an array of pointers to arrays of these, indexed
 * [screen/player][slot]-style by two signed 16-bit indices. */
typedef struct {
    u8 pad0[0x4];
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x2B - 0xC];
    u8 unk2B;
    u8 pad2C[0x46 - 0x2C];
    s16 unk46;
    s16 unk48;
    u8 pad4A[0x90 - 0x4A];
    s32 unk90;
    u8 pad94[0xAC - 0x94];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

void func_80036410(s32 a0, s32 a1)
{
    Entry90902E8 *p = &D_800902E8[(s16)a0][(s16)a1];

    p->unk46 = 1;
    p->unk48 = 0;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x100;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x8;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x2;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x4;
    D_800902E8[(s16)a0][(s16)a1].unk90 &= ~0x200;
    p->unk4 = p->unk8;
    p->unk2B = 1;
    D_800902E8[(s16)a0][(s16)a1].unk90 |= 1;
}

void func_80036518(void)
{
    D_8008E84C = 0;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036528);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_800368E8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036A54);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036A7C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_f", func_80036AA8);
