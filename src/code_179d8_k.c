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
extern s32 func_8002FAC4(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);  /* code_179d8_m, not yet matched: local guess, matches code_179d8_j's independent reading of the same call shape */
extern s32 func_800300D0(s32 a0, s16 a1, s16 a2, u16 a3);   /* code_179d8_m, not yet matched: local guess, ditto */
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
    u8 pad8[0xC - 0x8];
    u8 *unkC;   /* +0xC: a saved backup of unk4, restored into it on a "resume" path */
    u8 unk10;   /* +0x10: one-shot latch, set once a "kind 1" retrigger fires */
    u8 pad11[0x12 - 0x11];
    u8 unk12;   /* +0x12: byte offset to the active embedded state block */
    u8 unk13;   /* +0x13 */
    u8 unk14;   /* +0x14 */
    u8 unk15;   /* +0x15: a cached byte, written from the "default kind" path */
    u8 unk16;   /* +0x16: event-kind selector (compared against 0x14/0x1E/0x28) */
    u8 pad17[0x27 - 0x17];
    u8 unk27;   /* +0x27: dispatch-mode selector (compared against 1) */
    u8 unk28;   /* +0x28: a cached byte, written from the "mode 1" latch path */
    u8 unk29;   /* +0x29: a retrigger/step counter */
    u8 unk2A;   /* +0x2A: a second, independent retrigger/step counter */
    u8 pad2B[0x4C - 0x2B];
    s16 unk4C;  /* +0x4C */
    u8 pad4E[0x6E - 0x4E];
    s16 unk6E;  /* +0x6E: a repeat/skip counter, decremented per catch-up tick */
    s16 unk70;  /* +0x70: next scheduling threshold, compared against unk88 */
    u8 pad72[0x74 - 0x72];
    u16 unk74;  /* +0x74: nonzero-gated dispatch enable flag */
    u8 pad76[0x80 - 0x76];
    s32 unk80;  /* +0x80: accumulated tick position */
    u8 pad84[0x88 - 0x84];
    s32 unk88;  /* +0x88: last-value scratch, overloaded per call site */
    u8 pad8C[0xA8 - 0x8C];
    s16 unkA8;  /* +0xA8: a masked-byte parameter cached from a dispatch call */
    u8 padAA[0xAC - 0xAA];
} Entry90902E8;
extern Entry90902E8 *D_800902E8[];

/* Shared VLQ-style delta-time decoder: reads a 7-bit-per-byte
 * little-endian... no, MIDI-style BIG-endian continuation-bit-first
 * encoding from rec->unk4 (advancing the cursor as it goes), scales the
 * decoded magnitude by 10, adds it to rec->unk80, and returns the scaled
 * delta.  A first byte of 0 is a sentinel for "no delta" -- returns 0
 * without touching rec->unk80 at all. */
extern s32 func_80035E80(s16 channel, s16 slot);

/* Forward declaration for a sibling function defined later in THIS unit
 * (func_8003424C, still INCLUDE_ASM) -- called from func_80034138's
 * catch-up loop below with the same (channel, slot) pair as every other
 * helper in this file; its own return/side effects are not yet
 * characterised since it has not been matched. */
extern void func_8003424C(s16 channel, s16 slot);

/* STALL -- see docs/match-reports/func_80034138.md. length exact 69/69,
 * 66/69 raw word-match, first real diff at word 22: two pure scheduling
 * residues (a load-pair order swap and one delay-slot filler choice),
 * not a logic or CFG difference. */
#if 0
void func_80034138(s16 a0, s16 a1, s16 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    s32 dead[2];
    s16 last = rec->unk70;
    s32 elapsed = rec->unk88;
    s32 delta = elapsed - last;
    s16 remain;
    s32 sum;
    s32 step;
    s16 last2;

    if (0) {
        dead[0] = 0;
        dead[1] = 0;
    }
    if (delta > 0) {
        remain = rec->unk6E;
        if (remain > 0) {
            rec->unk6E = remain - 1;
            return;
        }
        if (remain == 0) {
            rec->unk6E = a2;
            rec->unk88 = rec->unk88 - 1;
            return;
        }
        rec->unk88 = delta;
        return;
    }
    if (last < elapsed) {
        return;
    }
    sum = elapsed;
    for (;;) {
        func_8003424C(a0, a1);
        step = rec->unk88;
        if (step != 0) {
            last2 = rec->unk70;
            sum += step;
            if (sum < last2) {
                continue;
            }
            rec->unk88 = sum - last2;
            break;
        }
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034138);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_8003424C);

/* STALL -- see docs/match-reports/func_800344FC.md. length exact 70/70,
 * 44/70 raw word-match, first real diff at word 1: a consistent
 * register-identity rename ($t0<->$a2, $s0<->$s1, $t1/$t2/$t3 shifted by
 * one), not a logic or CFG difference -- CLAUDE.md's register-identity
 * STALL rule. */
#if 0
void func_800344FC(s16 a0, s16 a1, s32 a2, s32 a3)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 offset = rec->unk12;
    s16 speed = *(s16 *)((u8 *)rec + 0x4E + offset * 2);
    s32 divided = ((u8)a3 * (s32)speed) / 127;
    u8 *ptr = offset + (u8 *)rec;
    u16 flag = rec->unk74;
    u8 status = ptr[0x17];

    if (flag == 0) {
        return;
    }
    if ((u8)a3 != 0) {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        func_8002FAC4(packed, note, vol, (u8)a3, (u16)divided, status);
        rec->unkA8 = (u8)a3;
    } else {
        s16 packed = (a1 << 8) | a0;
        s16 note = rec->unk4C;
        u8 vol = ptr[0x2C];
        func_800300D0(packed, note, vol, (u8)a3);
    }
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800344FC);

void func_80034614(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;

    p[0x2C] = a2;
    rec->unk88 = func_80035E80(a0, a1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034690);

/* A stack-local buffer this function passes to three cross-unit callees:
 * `func_800334F0(ch, byte, out)` (established elsewhere as
 * `s16 func_800334F0(s16, s16, Entry8E968 *)` in code_179d8_i.c, a
 * different unit's own reduced view -- this unit's own view only needs
 * `unk0`, an item count) fills the FIRST 0x10 bytes; `func_80033260` and
 * `func_80036230` (both still unmatched, no established prototype
 * anywhere) are then called once per item with a pointer to the NEXT 0x28
 * bytes of the SAME object.  Modeled as one struct, not two separate
 * locals, because the loop's exit test re-reads `unk0` from memory on
 * every iteration even though nothing in this function's own source
 * writes it after the first call -- the per-item pointer passed to the
 * other two callees aliases the same object, so the compiler cannot prove
 * `unk0` is unchanged and must reload it. */
typedef struct {
    u8 unk0;    /* +0x00: item count, written by func_800334F0 */
    u8 pad1[0x10 - 0x1];
} NoteList_800349B0;

typedef struct {
    u8 pad0[0x8];
    u8 unk8;    /* +0x08: byte stamped between the two per-item calls */
    u8 pad9[0x28 - 0x9];
} Scratch_800349B0;

extern s16 func_800334F0(s16 a0, s16 a1, void *out);
extern void func_80033260(s16 a0, u8 a1, s16 a2, void *out);
extern void func_80036230(s16 a0, u8 a1, s16 a2, void *out);

/* STALL -- see docs/match-reports/func_800349B0.md. Compiled length ONE
 * WORD SHORT (78/79); frame size also 8 bytes larger than retail's 0x70
 * with this struct split (0x78) -- two related but distinct residues, both
 * register/stack-allocation artifacts, not logic differences. */
#if 0
void func_800349B0(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;
    Scratch_800349B0 scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        scratch.unk8 = a2;
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800349B0);

/* Same shape as func_800349B0 -- see that function's own struct comment.
 * Only the scratch byte's offset differs (0xB here vs 0x8 there). */
typedef struct {
    u8 pad0[0xB];
    u8 unkB;    /* +0x0B: byte stamped between the two per-item calls */
    u8 pad9[0x20 - 0xC];
} Scratch_80034AEC;

/* STALL -- see docs/match-reports/func_80034AEC.md (and func_800349B0.md,
 * the identically-shaped sibling this one shares its whole residue class
 * with). Compiled length ONE WORD SHORT (78/79); same register-rescue and
 * stack-allocation residues as func_800349B0. */
#if 0
void func_80034AEC(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;
    Scratch_80034AEC scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        scratch.unkB = a2;
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034AEC);

/* Same NoteList/callee shape as func_800349B0/func_80034AEC, plus a
 * range check on this function's own third parameter that picks a
 * one-byte flag written into the scratch buffer at relative offset 1. */
typedef struct {
    u8 pad0[0x1];
    u8 unk1;    /* +0x01: velocity-curve flag, 2 / 0 / left untouched */
    u8 pad2[0x20 - 0x2];
} Scratch_80034C28;

/* STALL -- see docs/match-reports/func_80034C28.md. Compiled length ONE
 * WORD SHORT (89/90). Two distinct residues: the family's register-rescue
 * class (same as func_800349B0/func_80034AEC) plus a NEW immediate-
 * constant canonicalization difference. */
#if 0
void func_80034C28(s16 a0, s16 a1, s32 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *p = (u8 *)rec + rec->unk12;
    NoteList_800349B0 list;
    Scratch_80034C28 scratch;
    s32 i;

    func_800334F0(rec->unk4C, p[0x2C], &list);
    for (i = 0; i < list.unk0; i++) {
        func_80033260(rec->unk4C, p[0x2C], (s16)i, &scratch);
        if ((u8)a2 < 0x40) {
            scratch.unk1 = 2;
        } else if ((u8)(a2 + 0xC0) < 0x40) {
            scratch.unk1 = 0;
        }
        func_80036230(rec->unk4C, p[0x2C], (s16)i, &scratch);
    }
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034C28);

/* STALL -- see docs/match-reports/func_80034D90.md. length exact 51/51,
 * 49/51 raw word-match, residue is the project's settled commutative-
 * operand-order canonicalization class (2 words). Near-miss body preserved
 * in the report; #if 0 body kept here too so it travels with this .c. */
void func_80034D90(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];

    func_80036044();
    func_80036518();

    *((u8 *)rec + rec->unk12 + 0x2C) = rec->unk12;
    rec->unk13 = 0;
    rec->unk14 = 0;
    *(s16 *)((u8 *)rec + 0x4E + rec->unk12 * 2) = 0x7F;
    *((u8 *)rec + rec->unk12 + 0x17) = 0x40;
    rec->unk88 = func_80035E80(a0, a1);
}

/* A per-(channel,slot) dispatch table of function pointers, row-major with
 * a 0x40 (64)-byte stride (16 pointers per row) -- also referenced from
 * code_179d8_c's func_8003221C. Not yet given a real element count; the
 * outer dimension is left open. */
typedef void (*Fn80090368)(s32 channel, u8 arg1);
extern Fn80090368 D_80090368[][16];

/* STALL -- see docs/match-reports/func_80034E5C.md. Compiled length ONE WORD
 * SHORT (76/77), 54/77 raw word-match, first real content diff at word 55/56:
 * GCC folds the "slot" array-index multiply into the sign-extension in one
 * shift pair because the slot value is used exactly once, where retail
 * materializes the sign-extended slot into its own register first and
 * multiplies separately (the same two-instruction shape retail also uses for
 * the "channel" index, which IS reused later and so never gets fused here
 * either) -- a register/instruction-count residue, not a logic difference. */
#if 0
void func_80034E5C(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 kind;
    Fn80090368 fn;

    if (rec->unk27 == 1) {
        if (rec->unk10 == 0) {
            rec->unk28 = a2;
            rec->unk10 = 1;
            goto check;
        }
    }
    kind = rec->unk16;
    if (kind != 0x1E && kind != 0x14) {
        rec->unk15 = a2;
        rec->unk2A = rec->unk2A + 1;
    }
check:
    if (rec->unk16 != 0x28) {
        goto skip_call;
    }
    {
        s16 ch = a0;
        s16 sl = a1;
        fn = D_80090368[ch][sl];
        if (fn != NULL) {
            fn(ch, a2 & 0xFF);
        }
    }
skip_call:
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80034E5C);

/* STALL -- see docs/match-reports/func_80034F90.md. Compiled length EXACT
 * (82/82), 77/82 raw word-match, first real diff at word 40: a single
 * independent instruction (`sltiu`) the compiler hoists into a branch
 * delay slot one branch earlier than retail places it -- a pure
 * instruction-scheduling residue, not a logic or CFG difference. */
#if 0
void func_80034F90(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 kind = a2;
    s32 result;

    switch (kind) {
    case 0x14:
        rec->unk16 = a2;
        rec->unk27 = 1;
        result = func_80035E80(a0, a1);
        rec->unk88 = result;
        rec->unkC = rec->unk4;
        return;
    case 0x1E:
        if (rec->unk28 == 0) {
            rec->unk16 = a2;
            rec->unk10 = 0;
            rec->unk88 = func_80035E80(a0, a1);
            return;
        }
        if (rec->unk28 < 0x7F) {
            rec->unk28--;
            result = func_80035E80(a0, a1);
            rec->unk88 = result;
            if (rec->unk28 != 0) {
                rec->unk4 = rec->unkC;
            } else {
                rec->unk10 = 0;
            }
            return;
        }
        func_80035E80(a0, a1);
        rec->unk4 = rec->unkC;
        rec->unk88 = 0;
        return;
    default:
        rec->unk16 = a2;
        rec->unk2A = rec->unk2A + 1;
        rec->unk88 = func_80035E80(a0, a1);
        return;
    }
}
#endif
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

void func_80035154(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 counter = rec->unk29;

    rec->unk14 = a2;
    counter = counter + 1;
    rec->unk29 = counter;
    rec->unk88 = func_80035E80(a0, a1);
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800351D0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_800357B0);

/* STALL -- see docs/match-reports/func_80035A7C.md. length exact 44/44,
 * 36/44 raw word-match, first real diff at word 23: a register/schedule
 * choice across four field reads, not yet reduced to one axis. */
#if 0
void func_80035A7C(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *cursor = rec->unk4;
    u8 b;
    u8 *p;

    rec->unk4 = cursor + 1;
    b = *cursor;
    p = (u8 *)rec + rec->unk12;
    func_8002F610((a1 << 8) | a0, rec->unk4C, p[0x2C], b);
    rec->unk88 = func_80035E80(a0, a1);
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035A7C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035B2C);

/* STALL -- see docs/match-reports/func_80035E80.md. length exact 47/47,
 * 44/47 raw word-match, first real diff at word 27: the settled
 * commutative-operand-order class plus one downstream register choice. */
#if 0
s32 func_80035E80(s16 a0, s16 a1)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 *cursor = rec->unk4;
    s32 acc;
    s32 val;
    s32 result;
    u8 nb;

    rec->unk4 = cursor + 1;
    acc = *cursor;
    if (acc == 0) {
        return 0;
    }
    val = acc * 4;
    if (acc & 0x80) {
        acc &= 0x7F;
        do {
            cursor = rec->unk4;
            rec->unk4 = cursor + 1;
            nb = *cursor;
            acc = (acc << 7) + (nb & 0x7F);
        } while (nb & 0x80);
        val = acc * 4;
    }
    result = (val + acc) * 2;
    rec->unk80 += result;
    return result;
}
#endif
INCLUDE_ASM("asm/nonmatchings/code_179d8_k", func_80035E80);
