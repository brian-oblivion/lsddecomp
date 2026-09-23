/*
 * code_179d8_i -- what is LEFT of functions 220..237 of the original
 * code_179d8 monolith after round 34 gave fourteen of its sixteen functions
 * back to Sony.  Now 0x24490..0x247B8 (vram 0x80033C90..0x80033FB8), a
 * ONE-function unit holding func_80033C90 alone.
 *
 * ROUND 34 (2026-09-12): 0x2397C..0x24490 is TEN linked `libsnd` objects
 * (all Psy-Q 3.3) covering ELEVEN functions, every one of which had been
 * MATCHED as C:
 *   0x8003317C SsUtGetVabHdr         libsnd/ut_gvh    (already carried Sony's name)
 *   0x80033260 SsUtGetVagAtr         libsnd/ut_gva    (was func_80033260)
 *   0x800334A0 SsSetMVol             libsnd/scsmvol
 *   0x800334F0 SsUtGetProgAtr        libsnd/ut_gpa    (was func_800334F0)
 *   0x800335FC SsVabTransBody        libsnd/vs_vtb
 *   0x800336CC SsSetMute             libsnd/scsmute
 *   0x8003370C SsVabTransCompleted   libsnd/vs_vtc
 *   0x80033738 SsSeqCalledTbyT       libsnd/sscall    (157w)
 *   0x800339AC Snd_pause             libsnd/pause
 *   0x80033A4C Snd_nextpause         libsnd/pause
 *   0x80033AB0 Snd_tempo             libsnd/tempo     (120w)
 * Their C is DELETED, not commented out.  Reclassifying eleven matched
 * functions out of the game count is the correction CLAUDE.md asks for, not a
 * regression -- they were Sony library code the whole time.  Do not write C
 * for any of them again; `python3 tools/sdkstalls.py` and
 * `.venv/bin/python3 tools/psyq_sdk.py coverage` are the evidence.
 *
 * A pure PREFIX trim, so the unit kept its name and the `c` line simply moved
 * to 0x24490.
 *
 * A SECOND RUN in the same round then took the other end.  `libsnd/replay` and
 * `libsnd/vs_vab` (0x247B8..0x2490C) are `Snd_replay`, `SsVabClose` and
 * `SsVabOpen` -- func_80033FB8, func_80034020 and func_800340B0, all three
 * previously MATCHED as C and all three now deleted from here.  That run sat
 * in the MIDDLE of what the prefix trim had left, so the slice became
 * [c][o][o][c] and the tail half became the one-function unit
 * `src/code_179d8_i_b.c` (func_8003410C).  Nothing moved with it: this unit
 * never owned a rodata attach.
 *
 * `libsnd/pause` is taken from the 3.3 disc ON PURPOSE: 3.5/3.6 split that
 * module into `pause` (0xA0) + `npause` (0x64), which is the same two
 * functions but does not tile as one object.  `runs` says the same thing as
 * "libsnd/pause supersedes libsnd/npause".
 *
 * A STANDING NOTE THIS FILE CARRIED FOR SEVERAL ROUNDS IS NOW RESOLVED.  The
 * func_80033260 comment said it could not be renamed to `SsUtGetVagAtr`
 * because the name had no `= 0x8003....;` alias in
 * config/symbols.slps01556.lsdde.txt, so INCLUDE_ASM'd callers in
 * code_179d8_k.c and code_179d8_e.c carried a literal `jal func_80033260`,
 * and "config/ is not this unit's to edit".  An SDK-object conversion edits
 * exactly that file: Sony's names for all eleven are now in the symbols file
 * and `make extract` rewrote every caller's `.s`.  The rename was mechanical.
 *
 * ROUND 33 CORRECTION, KEPT BECAUSE THE LESSON OUTLIVES ITS EXAMPLE.  This
 * file's carve-time census certified func_80032D34 as "ordinary large fresh
 * ground, not blocked" -- correctly, against both live blocker screens -- and
 * it was Sony's `SsVabOpenHeadWithMode` all along; a 232-line derivation went
 * into it.  A screen measures the obstruction it was built for and says
 * nothing about the ones it was not, and a carve-time census recorded as a
 * DIRECTIVE outlives the thing it was measured against.  Round 34 is the same
 * finding at eleven times the scale: every function above passed every blocker
 * screen and every one was unmatchable by construction.  Screen with
 * `python3 tools/nearmiss.py` (which runs `sdkstalls.py` for you); do not
 * trust a transcribed census, this comment included.
 *
 * Owns no rodata: zero `jtbl_` in its disassembly, and the yaml's rodata slot
 * list names no `.rodata, code_179d8_i` line.  Both of the old
 * code_179d8_tail's jump tables went to code_179d8_k.
 *
 * Keep every function in strict ROM-address order.
 */

#include "common.h"

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [screen][slot]-style by two signed 16-bit indices. This is
 * a reduced LOCAL view -- only the fields this unit's functions touch are
 * named. See code_179d8_f.c's own Entry90902E8 for a fuller layout of the
 * same array; each unit keeps its own independent reading, per project
 * convention (multiple local views of one struct are expected here). */
typedef struct {
    u8 pad0[0x2B];
    u8 unk2B;
    u8 pad2C[0x3E - 0x2C];
    s16 unk3E;
    s16 unk40;
    s16 unk42;
    s16 unk44;
    u8 pad46[0x4A - 0x46];
    s16 unk4A;
    u8 pad4C[0x70 - 0x4C];
    s16 unk70;
    u8 pad72[0x78 - 0x72];
    s16 unk78;
    s16 unk7A;
    u8 pad7C[0x8C - 0x7C];
    u32 unk8C;
    s32 unk90;
    u8 pad94[0x98 - 0x94];
    s32 unk98;
    u8 pad9C[0xA0 - 0x9C];
    s32 unkA0;
    u32 unkA4;
    u8 padA8[0xAC - 0xA8];
} Entry90902E8;

extern Entry90902E8 *D_800902E8[];

/* unk3E, unk40, unk42, unk78, unk7A, unk98 added to Entry90902E8 above,
 * in place of existing padding -- no existing field's offset changed.
 * unk40 is loaded with `lhu` (declared u16) but sign-checked via an
 * explicit `(s16)` cast at every comparison site -- matches retail's
 * `sll 16`/`bltz` idiom for checking a 16-bit value's sign without a
 * plain `lh`.
 *
 * STALL -- see docs/match-reports/func_80033C90.md for the full
 * algorithm derivation (correct, byte-verified block-by-block against
 * the asm) and the best C body reached (18/202 words, first diff at
 * word 1 -- the prologue's own `-0x40` vs `-0x38` frame size). The
 * residue is register/stack allocation, not logic. */
extern s32 SpuVmSetSeqVol(s16 a0, u16 a1, u16 a2, s32 a3);
extern s32 SpuVmGetSeqVol(s32 p0, s16 *out1, s16 *out2);

#if 0
void func_80033C90(s16 a0, s16 a1)
{
    Entry90902E8 **row = &D_800902E8[a0];
    s32 off = a1 * sizeof(Entry90902E8);
    Entry90902E8 *p = (Entry90902E8 *)((u8 *)*row + off);
    s32 c42 = p->unk42;
    s32 cnt = p->unk98 - 1;
    s16 pk;
    u16 sp10, sp12;
    u16 lo, hi;

    p->unk98 = cnt;
    do {
    if (c42 > 0) {
        if ((u32)cnt % (u32)c42 != 0) {
            goto tailFinal;
        }
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 -= 1;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if (sp10 == 0) {
            goto clearHandler;
        }
        if (sp12 == 0) {
            goto clearHandler;
        }
        lo = sp10 + (u16)-1;
        hi = sp12 + (u16)-1;
    } else {
        if (p->unk3E <= 0) {
            goto tailCheck;
        }
        p->unk40 += c42;
        if (p->unk40 < 0) {
            goto negHandler;
        }
        pk = (s16)(a0 | (a1 << 8));
        SpuVmGetSeqVol(pk, (s16 *)&sp10, (s16 *)&sp12);
        if ((s32)sp10 < -(s32)p->unk42) {
            goto clearHandler;
        }
        if ((s32)sp12 < -(s32)p->unk42) {
            goto clearHandler;
        }
        lo = sp10 + p->unk42;
        hi = sp12 + p->unk42;
    }
    } while (0);
    SpuVmSetSeqVol(pk, lo, hi, 0);
    goto tailCheck;

clearHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    D_800902E8[a0][a1].unk90 &= ~0x20;
    goto tailCheck;

negHandler:
    SpuVmSetSeqVol((s16)(a0 | (a1 << 8)), 0, 0, 0);
    ((Entry90902E8 *)((u8 *)*row + off))->unk90 &= ~0x20;

tailCheck:
    if (p->unk98 == 0 || p->unk40 == 0) {
        D_800902E8[a0][a1].unk90 &= ~0x20;
    }

tailFinal:
    SpuVmGetSeqVol((s16)(a0 | (a1 << 8)), &p->unk78, &p->unk7A);
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_179d8_i", func_80033C90);
