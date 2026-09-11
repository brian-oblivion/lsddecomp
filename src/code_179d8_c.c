/*
 * code_179d8_c -- window [200..219] of the original 274-function code_179d8
 * monolith, 0x22948..0x23500 (vram 0x80032148..0x80032D00).
 *
 * Carved round 16 by blocker DENSITY (see code_179d8_b's header for the
 * full window census). This window screened 16/20 clean.
 *
 * STALE CLAIM REMOVED, round 23 (2026-09-07): this comment listed
 * func_80032148, func_80032588, func_80032BF0 and func_80032C28 as blocked,
 * "all addiu_at". **`addiu_at` was resolved in round 21** (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), and round 23 MATCHED
 * func_80032BF0 and func_80032C28 byte-exact and took the other two to
 * 48/53 and ~90/96 with characterised non-toolchain residues. Nothing in
 * this window is toolchain-blocked. The two live blockers are `gp_rel` and
 * `nop_mflo_mfhi`; screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps, and never for `addiu_at`.
 *
 * Round 23's runner bravo spotted this line as stale and correctly did not
 * edit it (parallel-mode rules); the head fixed it at consolidation.
 *
 * func_80032588 owns jtbl_80010CD8, whose sub-slot of the 0xFD8 rodata
 * region is ATTACHED to this unit in the splat yaml. Leave that alone.
 *
 * Note func_80032AD0/SetRCnt: this window holds what look like Psy-Q root
 * counter routines linked into game text rather than into a psyq_* segment.
 * They are ordinary work, but do not generalise a finding from them to the
 * game's own code.
 *
 * This slice was cut at ROM-address boundaries, so it has no reason to
 * align with class boundaries -- expect it to span more than one class,
 * and identify each with tools/classtable.py rather than assuming one.
 *
 * Declarations: keep anything that encodes THIS unit's reading of a class
 * next to the code, in this file. Do not create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032148);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_8003221C);

extern void func_8003221C(s32 arg0);

void func_80032368(void)
{
    func_8003221C(0);
}

void func_80032388(void)
{
    func_8003221C(1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_800323A8);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032588);

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032708);

extern void func_80032708(s32 arg0);

void func_80032998(void)
{
    func_80032708(1);
}

void func_800329B8(void)
{
    func_80032708(0);
}

extern void func_80024CE0(void);
extern void VSyncCallback(void (*cb)(void));
extern void (*InterruptCallback(s32 arg0, void (*callback)(void)))(void);
extern void func_80024CF0(void);
extern s32 D_8006DCA8;
extern s32 D_8006DC94;
extern s32 D_8006DC8C;
extern s32 D_8006DC90;
extern void (*D_8006DC9C)(void);

void func_800329D8(void)
{
    s32 v;

    if (D_8006DCA8 != 0) {
        return;
    }

    D_8006DC94 = 0;
    func_80024CE0();

    if (D_8006DC8C != 0) {
        VSyncCallback(0);
        D_8006DC8C = 0;
    } else {
        v = D_8006DC90;
        if (v != -1) {
            if (v != 0) {
                InterruptCallback(v, NULL);
            } else {
                InterruptCallback(0, D_8006DC9C);
            }
            D_8006DC90 = -1;
        }
    }

    func_80024CF0();
}

extern void SpuQuit(void);

void func_80032A7C(void)
{
    SpuQuit();
}

extern void func_80033738(void);
extern void (*D_8006DC9C)(void);

void func_80032A9C(void)
{
    if (D_8006DC9C != NULL) {
        D_8006DC9C();
    }
    func_80033738();
}

extern s32 D_8006DCA0;

void func_80032AD0(void)
{
    if (D_8006DCA0 == 0) {
        D_8006DCA0 = 1;
    } else {
        D_8006DCA0 = 0;
        func_80033738();
    }
}

/* Shadow copy of the three PSX root-counter register blocks (COUNT/MODE/
 * TARGET, each a hardware halfword, 0x10 apart -- matches the real
 * 0x1F801100/0x1F801110/0x1F801120 hardware spacing). D_8006DCB0 is a
 * pointer to this table, not the table itself. */
typedef struct {
    u16 count;              /* 0x0 */
    u8  pad2[0x4 - 0x2];
    u16 mode;                /* 0x4 */
    u8  pad6[0x8 - 0x6];
    u16 target;               /* 0x8 */
    u8  padA[0x10 - 0xA];
} RCntEntry;

extern RCntEntry *D_8006DCB0;

s32 SetRCnt(s32 n, s16 target, u32 mode)
{
    s32 idx = (u16)n;
    u16 md = 0x48;
    u32 isLow;

    if (idx >= 3) {
        return 0;
    }

    isLow = (u32)idx < 2;
    D_8006DCB0[idx].mode = 0;
    D_8006DCB0[idx].target = target;
    __asm__("");

    if (isLow) {
        if (mode & 0x10) {
            md = 0x49;
        }
        if (!(mode & 1)) {
            md |= 0x100;
        }
    } else if (idx == 2) {
        if (!(mode & 1)) {
            md = 0x248;
        }
    }

    if (mode & 0x1000) {
        md |= 0x10;
    }

    D_8006DCB0[idx].mode = md;
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032BB8);

/* Shadow of the PSX interrupt controller pair at 0x1F801070/0x1F801074
 * (I_STAT/I_MASK). D_8006DCAC is a pointer to this pair, not the pair
 * itself -- same "pointer-to-hardware-block" idiom as D_8006DCB0 above.
 * D_8006DCB4 holds the per-index IRQ mask bit (root counters 0/1/2 use
 * indices 0-2 -> Tmr0/Tmr1/Tmr2 IRQ bits 0x10/0x20/0x40; index 3 is the
 * 0x1 VBLANK bit). */
typedef struct {
    volatile u32 stat; /* 0x0, I_STAT */
    volatile u32 mask; /* 0x4, I_MASK */
} IrqRegs;

extern IrqRegs *D_8006DCAC;
extern u32 D_8006DCB4[4];

s32 func_80032BF0(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = D_8006DCAC;

    reg->mask |= D_8006DCB4[idx];
    return idx < 3;
}

s32 func_80032C28(u16 which)
{
    s32 idx = which;
    IrqRegs *reg = D_8006DCAC;

    reg->mask &= ~D_8006DCB4[idx];
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_c", func_80032C60);

extern s16 func_80032D34(void *a0, s16 a1, s32 a2, s32 a3);

s16 func_80032C98(void *a0, s16 a1)
{
    return func_80032D34(a0, a1, 0, 0);
}

s16 func_80032CCC(void *a0, s16 a1, s32 a2)
{
    return func_80032D34(a0, a1, 1, a2);
}
