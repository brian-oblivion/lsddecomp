/*
 * code_179d8_j_b -- the MIDDLE third of the old code_179d8_j slice, after
 * round 34 (2026-09-12) linked TWO Sony objects into what used to be one unit.
 * Now 0x21180..0x221B4 (vram 0x80030980..0x800319B4), five functions
 * (func_80030980 .. func_80031890).
 *
 * WHY THIS UNIT EXISTS, IN TWO STEPS, BOTH IN ROUND 34.
 *   1. `libsnd/vm_prog.o` (Psy-Q 3.6 -- the only disc carrying the module)
 *      covers 0x20FF0..0x21180: SpuVmSetProgVol, SpuVmGetProgVol,
 *      SpuVmSetProgPan, SpuVmGetProgPan, all four previously MATCHED as C.
 *      That split the old code_179d8_j into [c][o][c] and created this file.
 *   2. `libsnd/ut_pb.o` (Psy-Q 3.6 only, 0x90) covers 0x221B4..0x22244:
 *      `SsUtPitchBend`, which was `func_800319B4`, also previously MATCHED.
 *      That split THIS file again into [c][o][c], and everything from
 *      func_80031A44 on moved to `src/code_179d8_j_c.c`.
 * Reclassifying five matched functions out of the game count across the two
 * steps is the correction CLAUDE.md asks for, not a regression; their C is
 * DELETED, not commented out.
 *
 * `D_8008EA22` LEFT WITH `SsUtPitchBend`: that function and the deleted
 * func_80030648 were its only readers in this family, so the extern went with
 * the deletion rather than being kept as a dead declaration.
 *
 * NO RODATA ATTACH IS OWNED BY ANY UNIT IN THIS FAMILY, and that is measured,
 * not assumed: the old code_179d8_mid_c monolith contains zero `jtbl_` and
 * zero `.word .L` across its whole extent, and the splat yaml's rodata slot
 * list names none of `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's
 * code_179d8_c_b there was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.  Several externs below are read only by
 * functions still carried as INCLUDE_ASM (the `func_80030E90` scratch globals
 * in particular); they are knowledge about those functions, not dead code, and
 * were re-homed here deliberately rather than dropped.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * `nearmiss.py` runs `tools/sdkstalls.py` for you, and round 34 is why that
 * matters here: all five functions the two splits gave back to Sony were on
 * the old unit's carve-time WORKABLE list, screened clean on every blocker,
 * and were unmatchable by construction.
 *
 * Expect this slice to span more than one class; identify each with
 * tools/classtable.py rather than assuming the unit has one.  Keep every
 * function in strict ROM-address order.
 */

#include "common.h"

/* 0x20-byte-stride record indexed by `D_8008EA18 + D_8008EA13*16`
 * (func_80030E90's own computed index, not a channel id). Every field
 * this unit's own accessor touches is named; offsets are exact (read
 * from func_80030E90's own lbu/lhu immediates), field names are not. */
typedef struct {
    u8 unk0;  /* +0x0 */
    u8 unk1;  /* +0x1 */
    u8 unk2;  /* +0x2 */
    u8 unk3;  /* +0x3 */
    u8 unk4;  /* +0x4 */
    u8 unk5;  /* +0x5 */
    u8 unk6;  /* +0x6 */
    u8 unk7;  /* +0x7 */
    u8 pad8[0x16 - 0x8];
    u16 unk16; /* +0x16 */
    u8 pad18[0x20 - 0x18];
} RecordE978;
extern RecordE978 *D_8008E978;

/* Base pointer for a table of 0x10-byte entries, indexed by a 0..0x17
 * id.  Only the two leading s16 fields this unit's own accessors touch
 * are named. */
typedef struct EntryDAD4 {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    s16 unk4; /* +0x4 -- read by func_80031280, entry index 25 only */
    s16 unk6; /* +0x6 -- read by func_80031280, entry index 25 only */
    u8 pad8[0x10 - 0x8];
} EntryDAD4;
extern EntryDAD4 *D_8006DAD4;

/* Reentrancy lock, same identifier/type as the sibling reading in
 * Sony's `SsSeqCalledTbyT` (`libsnd/sscall`, linked since round 34; it was
 * code_179d8_i.c's matched func_80033738) -- "if already busy, return/skip;
 * set; ...; clear before returning" guarding a per-channel operation. */
extern s32 D_8008E934;

/* "Currently selected channel" scratch globals: written as a side
 * effect and then re-read from the global (not from the parameter) by
 * the same and sibling functions -- same idiom as this file's own
 * D_8008EA22 above. */
extern volatile u16 D_8008EA26;
extern volatile u8 D_8008EA18;

/* func_80030E90's own scratch globals -- a "start channel" setup
 * routine that stages its parameters and a couple of table lookups
 * into a block of one/two-byte globals before registering a new
 * active-channel record.  Offsets are exact (this unit's own field
 * accesses); names are opaque placeholders per the reduced-local-view
 * convention. */
extern u8 D_8008EA0C;
extern u8 D_8008EA0E;
extern u8 D_8008EA0F;
extern u8 D_8008EA10;
extern u8 D_8008EA11;
extern u8 D_8008EA13;
extern u8 D_8008EA16;
extern u8 D_8008EA17;
extern u8 D_8008EA19;
extern u8 D_8008EA1A;
extern u8 D_8008EA1B;
extern u8 D_8008EA1C;
extern u8 D_8008EA1D;
extern u8 D_8008EA1E;
extern u8 D_8008EA1F;
extern u8 D_8008EA20;
extern u16 D_8008EA24;

extern s32 func_8002CF18(void);
extern void func_8002D6A4(void);
extern void func_8002D8E0(s32 a0);
extern s32 func_8002E038(u16 a0, u16 a1);
extern void func_8002D1B4(s32 a0, u16 a1);

/* Loop bound for a small table of active "objects" (screen/slot
 * pairs); see code_179d8_i.c's D_80090B68/6C for the sibling reading of
 * an analogous count. */
extern u8 D_8008E9D0;

/* A pair of 16-bit bitmasks split across a 0..0x1F channel space
 * (low 16 channels in the first word, next 16 in the second), each
 * paired with an "active mask" word that is cleared wherever the
 * channel mask bit is set. */
extern u16 D_80090C60;
extern u16 D_80090C64;
extern u16 D_8008E228;
extern u16 D_8008E22C;

/* 52 (0x34)-byte-stride channel-configuration record, referenced by
 * several unrelated top-level symbols 2 bytes apart (D_8008D994,
 * D_8008D996, D_8008D99A, D_8008D99C, D_8008D99E, ...) -- each function
 * in this unit only touches the one or two fields it actually reads,
 * per this project's reduced-local-view convention. Only the leading
 * s16 is named; every array below shares this one shape. */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34D994;
extern Rec34D994 D_8008D994[];
extern Rec34D994 D_8008D996[];
extern Rec34D994 D_8008D998[];
extern Rec34D994 D_8008D99A[];
extern Rec34D994 D_8008D99C[];
extern Rec34D994 D_8008D99E[];

/* Same 0x34 stride, byte-sized "in use" flag at offset 0 (retail always
 * clears it with `sb`, never `sh`). */
typedef struct {
    u8 unk0; /* +0x0 */
    u8 pad1[0x34 - 0x1];
} Rec34Byte;
extern Rec34Byte D_8008D9A3[];

/* Same 0x34 stride, halfword field at offset 0 (retail always clears it
 * with `sh`). Three independent arrays share this shape (D_8008D988,
 * D_8008D98A and D_8008D98C, each 2 bytes apart in the data section --
 * same "several unrelated top-level symbols" convention as the
 * Rec34D994 group above). D_8008D988's field is read signed
 * (func_80031280 compares it against 0xFF with `lh`, not `lhu`). */
typedef struct {
    s16 unk0; /* +0x0 */
    u8 pad2[0x34 - 0x2];
} Rec34Half;
extern Rec34Half D_8008D988[];
extern Rec34Half D_8008D98A[];
extern Rec34Half D_8008D98C[];

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80030980);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80030E90);

s32 func_80031280(s16 idx, s16 p1, s16 p2, s16 p3, s16 p4)
{
    u16 chan;
    u32 mask0;
    u16 mask1;

    if (D_8008E934 == 1) {
        goto fail_nolock;
    }
    D_8008E934 = 1;
    if ((u16) idx >= 0x18) {
        goto fail;
    }
    if (D_8008D99E[idx].unk0 != p1
     || D_8008D99A[idx].unk0 != p2
     || D_8008D99C[idx].unk0 != p3
     || D_8008D994[idx].unk0 != p4) {
        goto fail;
    }
    if (D_8008D988[idx].unk0 == 0xFF) {
        D_8008D9A3[(u8) idx].unk0 = 0;
        D_8008D98C[(u8) idx].unk0 = 0;
        D_8006DAD4[25].unk4 = 0;
        D_8006DAD4[25].unk6 = 0;
    } else {
        D_8008EA26 = idx;
        chan = D_8008EA26;
        if (chan < 0x10) {
            mask0 = 1 << chan;
            mask1 = 0;
        } else {
            mask0 = 0;
            mask1 = 1 << (chan - 0x10);
        }
        D_8008D9A3[chan].unk0 = 0;
        D_8008D98C[chan].unk0 = 0;
        D_8008D988[chan].unk0 = 0;
        D_80090C60 = mask0 | D_80090C60;
        D_80090C64 |= mask1;
        D_8008E228 &= ~D_80090C60;
        D_8008E22C &= ~D_80090C64;
    }
    D_8008E934 = 0;
    return 0;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_8003149C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80031890);
