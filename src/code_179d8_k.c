/*
 * code_179d8_k -- functions 238..255 of the original 274-function code_179d8
 * monolith, 0x24938..0x2673C (vram 0x80034138..0x80035F3C).  Carved round 24
 * (2026-09-08) out of what had been the `code_179d8_tail` asm remainder,
 * which this unit consumes WHOLE -- there is no remainder left on either
 * side (code_179d8_i in front, code_179d8_f behind).
 *
 * WHY IT WAS UNCARVED FOR SIX ROUNDS, AND WHY THAT VERDICT IS DEAD.
 * The splat comment on the old remainder read "17 of its 18 functions are
 * addiu-$at blocked".  `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md), so that census measured
 * an obstruction that no longer exists.  Re-censused 2026-09-08 with
 * `python3 tools/nearmiss.py`'s four screens, canonical shell forms
 * (`grep -A2` FORWARD for nop_mflo_mfhi -- the direction is load-bearing):
 *
 *   18 of 18 CLEAN.  Zero gp_rel, zero nop_mflo_mfhi, zero `jr $t2`
 *   trampolines.  This is the single best carve left in the executable.
 *
 * Sizes, cheapest first -- six functions at 31..51 words, which is the
 * cheap seam the `fresh` queue had run out of:
 *   func_80034614  31w   func_800350D8  31w   func_80035154  31w
 *   func_80035A7C  44w   func_80035E80  47w   func_80034D90  51w
 *   func_80034138  69w   func_800344FC  70w   func_80034E5C  77w
 *   func_800349B0  79w   func_80034AEC  79w   func_80034F90  82w
 *   func_80034C28  90w   func_8003424C 172w   func_800357B0 179w
 *   func_80034690 200w   func_80035B2C 213w   func_800351D0 376w
 *
 * THIS UNIT OWNS THREE SWITCH JUMP TABLES, not the two the old remainder
 * comment claimed: func_80034690 -> jtbl_80010CF0, and func_800357B0 ->
 * jtbl_80010ED8 AND jtbl_80010F38 (a double switch).  The 0x14F0 rodata
 * slot holds exactly those three tables and nothing else, is referenced
 * from nowhere outside this unit, and is attached whole in the splat yaml.
 * You do not need to do anything about it -- but if you match either
 * function, remember a `%lo(jtbl_*)` load is ordinary matchable code now.
 *
 * Boundary checks at carve time, both sides: no function has more than one
 * `addiu $sp, $sp, -N`, every one ends in its own `jr $ra`, zero `alabel`,
 * and the one frameless function (func_80035E80) opens on
 * `sll $a0, $a0, 16` -- leaf argument narrowing, not a caller-frame read.
 *
 * Expect this slice to span more than one class; a ~20-function slice cut
 * at ROM-address boundaries has no reason to align with class boundaries.
 * Identify each with tools/classtable.py rather than assuming the unit has
 * one.  Expect low-level driver-shaped code rather than class-framework
 * code, as elsewhere in code_179d8; confirm, do not assume.
 */
#include "common.h"

/* Cross-unit calls, typed per-call-site from the registers loaded before
 * each `jal` -- none of these callees have an established prototype from
 * their own unit's side yet except where noted, so these are local
 * guesses, not authoritative.  Per this project's convention, a prototype
 * for a function ANOTHER unit defines stays in this .c, not in a shared
 * header. */
extern void func_8002F610(s32 a0, s16 a1, u8 a2, u8 a3);   /* code_179d8_m, not yet matched: local guess */
extern s32 func_80036044(void);                              /* code_179d8_f, MATCHED: return func_80038D74(0); */
extern void func_80036518(void);                              /* code_179d8_f, MATCHED: D_8008E84C = 0; */

/* A 172 (0xAC)-byte record; D_800902E8 is an array of pointers to arrays of
 * these, indexed [channel][slot]-style, same array documented from
 * code_179d8_f/_i/_j's own independent local readings -- see those units'
 * Entry90902E8 for a different reduced view of the same object; each unit
 * keeps its own per project convention.
 *
 * This unit's own reading is a low-level per-channel "sequencer voice"
 * playback record: unk4 is a cursor into a byte-encoded event stream,
 * unk80 an accumulated tick position, unk88 a scratch slot every function
 * in this file stores its own last computed value into (a pointer in one
 * caller, an updated counter in others -- genuinely overloaded, not a
 * misreading), and unk12 a BYTE OFFSET (already scaled, not an index) to
 * whichever of several embedded state blocks is presently active; several
 * functions here dereference `(u8 *)rec + rec->unk12` and then apply a
 * further FIXED displacement (0x17, 0x2C) from that computed base, which
 * is why those two fields are not named as fixed struct members below --
 * their address is only known at runtime, exactly the case this project's
 * pointer-arithmetic convention is for. */
typedef struct {
    u8 pad0[0x4];
    u8 *unk4;   /* +0x4: cursor into a byte-encoded (7-bit VLQ) event stream */
    u8 pad8[0x12 - 0x8];
    u8 unk12;   /* +0x12: byte offset to the active embedded state block */
    u8 unk13;   /* +0x13 */
    u8 unk14;   /* +0x14 */
    u8 pad15[0x29 - 0x15];
    u8 unk29;   /* +0x29: a retrigger/step counter */
    u8 pad2A[0x4C - 0x2A];
    s16 unk4C;  /* +0x4C */
    u8 pad4E[0x80 - 0x4E];
    s32 unk80;  /* +0x80: accumulated tick position */
    u8 pad84[0x88 - 0x84];
    s32 unk88;  /* +0x88: last-value scratch, overloaded per call site */
    u8 pad8C[0xAC - 0x8C];
} Entry90902E8;
extern Entry90902E8 *D_800902E8[];

/* Shared VLQ-style delta-time decoder: reads a 7-bit-per-byte
 * little-endian... no, MIDI-style BIG-endian continuation-bit-first
 * encoding from rec->unk4 (advancing the cursor as it goes), scales the
 * decoded magnitude by 10, adds it to rec->unk80, and returns the scaled
 * delta.  A first byte of 0 is a sentinel for "no delta" -- returns 0
 * without touching rec->unk80 at all. */
extern s32 func_80035E80(s16 channel, s16 slot);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034138);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_8003424C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800344FC);

void func_80034614(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;

    p[0x2C] = a2;
    rec->unk88 = func_80035E80(a0, a1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034690);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800349B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034AEC);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034C28);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034D90);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034E5C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034F90);

void func_800350D8(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 counter = rec->unk29;

    rec->unk13 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = func_80035E80(a0, a1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035154);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800351D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800357B0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035A7C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035B2C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035E80);
