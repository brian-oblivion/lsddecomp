/*
 * ROUND 42 CORRECTION (2026-09-15) -- READ BEFORE ANY "BLOCKED" LINE BELOW:
 * every claim in this comment that a function is BLOCKED by `gp_rel`,
 * `nop_mflo_mfhi` or `addiu_at` is STALE.  All three constructs are RESOLVED
 * by pinned maspsx flags (CLAUDE.md, "Open toolchain blockers");
 * `tools/nearmiss.py` reports them tagged (RESOLVED-not-a-blocker) and counts
 * none of them.  Any "do NOT spend attempts on these" directive below is
 * therefore RETRACTED: those functions are ordinary matching work, and most
 * carry a mechanism-correct partial derivation already.  The rest of this
 * comment still stands -- only the blocker verdicts are withdrawn.
 * Screen: `python3 tools/nearmiss.py`, round 43 (2026-09-15).
 *
 * code_179d8_c -- window [200..219] of the original 274-function code_179d8
 * monolith, now 0x22A1C..0x22BA8 (vram 0x8003221C..0x800323A8).
 *
 * ROUND 33: this slice lost TWO functions to Sony and was split in half.
 *   - func_80032148, its old FIRST function, is `SpuVmVSetUp`
 *     (`libsnd/vm_vsu.o`, Psy-Q 3.3), linked from the object.
 *   - func_800323A8 (120w) is `SsSetTableSize` (`libsnd/sstable.o`, Psy-Q
 *     3.5), linked from the object. It sat in the MIDDLE, so the slice became
 *     [c code_179d8_c][o sstable][c code_179d8_c_b] and everything from
 *     func_80032588 on now lives in `src/code_179d8_c_b.c`.
 * Neither was ever matchable as C; both stall reports are kept, re-titled
 * CONVERTED. This unit is now three functions: func_8003221C and its two
 * one-line callers.
 *
 * THE 0x14D8 RODATA ATTACH IS NO LONGER OURS. It belongs to func_80032588
 * (jtbl_80010CD8), which went to code_179d8_c_b, and the yaml attach moved
 * with it. Do not move it back.
 *
 * Carved round 16 by blocker DENSITY (see code_179d8_b's header for the
 * full window census). This window screened 16/20 clean.
 *
 * STALE CLAIM REMOVED, round 23 (2026-09-07): this comment listed
 * func_80032148, func_80032588, func_80032BF0 and func_80032C28 as blocked,
 * "all addiu_at". **`addiu_at` was resolved in round 21** (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), and round 23 MATCHED
 * func_80032BF0 and func_80032C28 byte-exact and took the other two to
 * 48/53 and ~90/96 with characterised non-toolchain residues. (The 48/53 was
 * func_80032148 -- round 32 then found it is Sony's, so that effort was spent
 * on library code; see docs/match-reports/func_80032148.md.) Nothing in
 * this window is toolchain-blocked. The two live blockers are `gp_rel` and
 * `nop_mflo_mfhi`; screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps, and never for `addiu_at`.
 *
 * Round 23's runner bravo spotted this line as stale and correctly did not
 * edit it (parallel-mode rules); the head fixed it at consolidation.
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


extern void ResetCallback(s32 arg0);
extern void func_80038D54(void);
extern void func_80036AA8(void);
extern void InitSpuDriver(s32 arg0);
extern s32 GetVideoMode(void);
extern u16 D_8006DC5C[8];
extern u16 D_8006DC6C[0x10];
extern s32 D_8009024C;
extern s32 D_8008EA00;
extern s32 D_8006DC8C;
extern s32 D_8006DC90;
extern s32 D_8006DC94;
extern void (*D_8006DC9C)(void);
extern s32 D_8006DC98;
extern s32 D_8008E934;

typedef struct {
    s32 pad0[0x10];
} VoiceState80090368;

extern VoiceState80090368 D_80090368[0x20];

void func_8003221C(s32 arg0)
{
    s32 i, j;
    u16 *base;
    u16 *src;
    u16 *reg;

    ResetCallback(arg0);

    if (arg0 == 0) {
        func_80038D54();
    } else {
        func_80036AA8();
    }

    reg = (u16 *)0x1F801C00;
    base = D_8006DC5C;
    for (i = 0; i < 0x18; i++) {
        j = 0;
        src = base;
        for (; j < 8; j++) {
            *reg = *src;
            src++;
            reg++;
        }
    }

    reg = (u16 *)0x1F801D80;
    i = 0;
    src = D_8006DC6C;
    for (; i < 0x10; i++) {
        *reg = *src;
        src++;
        reg++;
    }

    InitSpuDriver(0x18);

    for (i = 0; i < 0x20; i++) {
        for (j = 15; j >= 0; j--) {
            D_80090368[i].pad0[j] = 0;
        }
    }

    D_8009024C = 0x3C;
    D_8008EA00 = 0;
    D_8006DC8C = 0;
    D_8006DC90 = -1;
    D_8006DC94 = 0;
    D_8006DC9C = NULL;
    D_8006DC98 = GetVideoMode();
    D_8008E934 = 0;
}

void func_80032368(void)
{
    func_8003221C(0);
}

void func_80032388(void)
{
    func_8003221C(1);
}
