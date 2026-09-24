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
 * code_179d8_j -- the FRONT quarter of what was functions 173..198 of the
 * original code_179d8 monolith, now 0x20ADC..0x20FF0 (vram
 * 0x800302DC..0x800307F0, eight functions).  Carved round 21 (2026-09-06) out
 * of the MIDDLE of the old code_179d8_mid_c by blocker density.
 *
 * ROUND 34 (2026-09-12): the slice was CUT IN TWO by a linked Sony object.
 * `libsnd/vm_prog.o` (Psy-Q 3.6 -- the only disc that carries the module)
 * covers 0x20FF0..0x21180, four functions that had all been MATCHED as C:
 *   0x800307F0 SpuVmSetProgVol   (was func_800307F0)
 *   0x80030864 SpuVmGetProgVol   (was func_80030864)
 *   0x800308B8 SpuVmSetProgPan   (was func_800308B8)
 *   0x8003092C SpuVmGetProgPan   (was func_8003092C)
 * Reclassifying four matched functions out of the game count is the correction
 * CLAUDE.md asks for, not a regression; their C is DELETED, not commented out.
 * A placed object cannot live inside a `c` segment, so the slice became
 * [c code_179d8_j][o libsnd/vm_prog][c code_179d8_j_b] and everything from
 * func_80030980 on moved to `src/code_179d8_j_b.c`.
 *
 * THE `SlotE968` LOCAL VIEW LEFT WITH THEM.  Those four accessors were this
 * file's only readers of `D_8008E968`, so the typedef and the extern went with
 * the deletion rather than being kept as dead declarations.  code_179d8_i.c
 * kept its own independent reading of the same table under a different name
 * (`Entry8E968`), per the project's multiple-local-views convention -- that is
 * where to look if you need the layout again.
 *
 * NO RODATA MOVED ANYWHERE: the old code_179d8_mid_c contains zero `jtbl_` and
 * zero `.word .L` across the whole monolith, so no unit in this family owns a
 * rodata attach, and the yaml's slot list names none of them.
 *
 * BLOCKER PROFILE: the carve-time census this header used to quote was stale
 * (it screened for `addiu_at`, resolved in round 21) and its WORKABLE list is
 * staler still -- eight of its eleven entries are now either matched or Sony's.
 * Derive the live queue from this file's own INCLUDE_ASM entries and screen
 * with `python3 tools/nearmiss.py`, which also runs `sdkstalls.py` for you.
 * ROUND 34 IS WHY THAT LAST CLAUSE MATTERS: func_800307F0, func_80030864,
 * func_800308B8 and func_8003092C were all on that WORKABLE list, all four
 * screened clean on every blocker, and all four were Sony's.
 *
 * func_800303FC is a two-instruction `jr $ra; nop` leaf that splat emitted as
 * C itself.  It was never work; do not count it as one.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */

#include "common.h"

/* ------------------------------------------------------------------------
 * Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype yet, so
 * these are local guesses, not authoritative.  (This note used to add "several
 * are themselves addiu-$at blocked in their own units"; that is stale as of
 * round 21 and was removed rather than left to be believed.)  See CLAUDE.md's note on this.
 * ------------------------------------------------------------------------ */
extern s32 StartNote(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
extern s32 StopNote(s32 a0, s16 a1, s16 a2, u16 a3);
extern s32 SpuVmVSetUp(s16 a0, s16 a1);
extern s16 SpuVmPBVoice(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
extern void SeAutoVol(s16 a0, s16 a1, s16 a2, s16 a3);
extern void BeginVoiceFade(s16 a0, s16 a1, s16 a2, s16 a3);

extern u16 D_8008EA22;

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by a packed argument (slot in the
 * high byte, screen in the low byte) -- see Sony's `Snd_pause`
 * (`libsnd/pause`, linked since round 34; it was code_179d8_i.c's matched
 * func_800339AC), which builds exactly this packing before calling into
 * this unit's SpuVmSeqKeyOff. Reduced local view: only the two leading s16
 * fields this unit's own accessors touch are named. See code_179d8_f.c /
 * code_179d8_i.c's own Entry90902E8 for a fuller layout of the same array;
 * each unit keeps its own independent reading, per project convention. */
typedef struct {
    u8 pad0[0x74];
    s16 unk74; /* +0x74 */
    s16 unk76; /* +0x76 */
    u8 pad78[0xAC - 0x78];
} Entry90902E8;
extern Entry90902E8 *D_800902E8[];

s32 SpuVmSeKeyOn(s32 p0, s32 p1, s32 p2, s32 p3, u16 p4, u16 p5)
{
    u16 outA;
    u16 outB;

    if (p4 == p5) {
        outB = 0x40;
        outA = p4;
    } else if (p5 < p4) {
        outA = p4;
        outB = (p5 << 6) / p4;
    } else {
        outA = p5;
        outB = 0x7F - ((p4 << 6) / p5);
    }
    return StartNote(0x21, (s16) p0, (s16) p1, (u16) p2, outA, outB);
}

s32 SpuVmSeKeyOff(s16 p0, s16 p1, u16 p2)
{
    return StopNote(0x21, p0, p1, p2);
}

void func_800303FC(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", SpuVmSetSeqVol);

s32 SpuVmGetSeqVol(s32 p0, s16 *out1, s16 *out2)
{
    Entry90902E8 *tbl = D_800902E8[(u8) p0];
    s16 *cur = (s16 *) &D_8008EA22;

    *cur = (s16) p0;
    *out1 = tbl[(p0 & 0xFF00) >> 8].unk74;
    *out2 = tbl[(p0 & 0xFF00) >> 8].unk76;
    return *cur;
}

s32 SpuVmGetSeqLVol(s32 p0)
{
    s32 channel = p0 & 0xFF;
    Entry90902E8 *tbl = D_800902E8[channel];
    s32 recIdx = (p0 & 0xFF00) >> 8;

    __asm__("");
    D_8008EA22 = channel;
    return tbl[recIdx].unk74;
}

s32 SpuVmGetSeqRVol(s32 p0)
{
    Entry90902E8 *tbl = D_800902E8[(u8) p0];

    __asm__("");
    D_8008EA22 = p0;
    return tbl[(p0 & 0xFF00) >> 8].unk76;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j", SpuVmSeqKeyOff);
