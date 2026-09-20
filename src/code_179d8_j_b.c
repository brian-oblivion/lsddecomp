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
 * `D_8008EA22`'S OWN COMMENT WAS WRONG AND IS CORRECTED HERE (round 36): it
 * used to claim `SsUtPitchBend` and the deleted `func_80030648` were its
 * only readers in this family, and dropped the extern on that basis. That
 * was never true of this file's own two remaining stalls -- func_80030E90
 * and func_8003149C both WRITE it (`D_8008EA22 = 0x21;`) -- it just went
 * unnoticed because both were still INCLUDE_ASM and nothing failed to
 * link. Declared again below.
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
extern u16 D_8008EA22;

/* Base pointer for a table of 0x10-byte slots, indexed by the same <0x18
 * channel space func_80030E90 validates via SpuVmVSetUp. This unit's own
 * reduced view: only the fields func_80030E90 itself touches are named.
 * The sibling accessors that used to share this typedef (SpuVmSetProgVol
 * and friends, code_179d8_j.c's old `SlotE968`) are Sony's own object as
 * of round 34's split and never touched offset 0; func_80030E90 does, so
 * this unit's own copy of the type names it (see func_80030864.md, the
 * matched sibling's report, for the +0x1/+0x4 fields' provenance). */
typedef struct SlotE968 {
    u8 unk0; /* +0x0 */
    u8 unk1; /* +0x1 */
    u8 pad2[0x4 - 0x2];
    u8 unk4; /* +0x4 */
    u8 pad5[0x10 - 0x5];
} SlotE968;
extern SlotE968 *D_8008E968;

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
extern s32 SpuVmVSetUp(s16 a0, s16 a1);

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
extern Rec34D994 D_8008D990[];
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

/* Shared with func_8002D8E0/func_8002D1B4 in code_179d8_l.c (same
 * two-level entry table, same blend-cascade shape); this unit's own
 * reduced view, per the project's per-unit-local-view convention --
 * only the two fields func_80030980 itself touches are named. */
typedef struct {
    u8 pad0[0x74];
    u16 unk74;
    u16 unk76;
    u8 pad78[0xAC - 0x78];
} D800902E8Entry;
extern D800902E8Entry *D_800902E8[];

typedef struct {
    u8 pad0[0x18];
    u8 unk18; /* +0x18 */
} ObjE970;
extern ObjE970 *D_8008E970;

extern s16 D_8008E8C0;
extern u8 D_8008D7F0[];
extern u8 D_8008D970[];

/* STALL -- see docs/match-reports/func_80030980.md. Round 50: LENGTH-EXACT
 * (324/324 words, zero drift outside the function), 7/324 raw word-match,
 * best body preserved there in #if 0. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_80030980);
/* STALL -- see docs/match-reports/func_80030E90.md. Round 26: 239/252 words
 * (13 short). Round 31: 240/252 (12 short), the `s32 result` axis. Round 36:
 * rebuilt with SpuVmVSetUp's real name (was func_80032148 -- round 34's SDK
 * conversion renamed the callee, and the report's preserved body was never
 * corrected) plus the SlotE968/D_8008EA22 declarations round 34's carve
 * deleted from this family (see this file's header note above). Measured
 * 241/252 words this round (11 short) -- close to but not an exact
 * reproduction of round 31's 240/252, consistent with that round's own
 * caveat that its barrier placement could not be fully recovered from
 * prose. Three residues persist per the report: busy-lock guard polarity
 * (confirmed non-source-derivable by a round-26 head reproducer), missing
 * field-copy load-delay nops (confirmed not barrier-reachable), and the
 * func_8002CF18() return value's register identity -- all three exhausted
 * per this report's own extensive history (14+ hand reshapes across two
 * rounds plus a 9-probe head investigation). Round 40: rebuild confirms
 * 241/252 exactly (isolated from the sibling stall's own shortfall); first
 * real permuter search (70k+ iterations), one sub-base lead found and
 * confirmed NOT to reproduce on the real oracle -- see
 * docs/match-reports/func_80030E90.md's round-40 addendum. */
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

/* STALL -- see docs/match-reports/func_8003149C.md. func_80030E90's near
 * twin. Round 26: 236/253 words (17 short). Round 36: rebuilt with
 * SpuVmVSetUp's real name (was func_80032148 -- round 34's SDK conversion
 * renamed the callee, report's preserved body never corrected). Measured
 * 237/253 words this round (16 short), raw match 33/253 -- close to but
 * not an exact reproduction of round 26's 236/253, same barrier-placement
 * recovery caveat as func_80030E90's round-36 note. Same three residue
 * classes as func_80030E90 plus two more this function's own report
 * documents (a second, independent guard-polarity flip on the "note == 0"
 * check; a one-statement-shifted D_8008EA13 fresh-read position), all
 * confirmed non-source-derivable by the sibling report's exhaustive
 * investigation. Round 40: rebuild confirms 237/253 (33/253 raw) exactly,
 * isolated from the sibling stall's own shortfall; first real permuter
 * search (48k+ iterations), three sub-base leads found and all confirmed
 * NOT to reproduce on the real oracle -- see
 * docs/match-reports/func_8003149C.md's round-40 addendum. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_b", func_8003149C);

/* The "release channel" twin of func_80031280's else-branch above: same
 * D_8008E934 lock, same (mask0, mask1) split of a 0..0x17 channel across two
 * 16-bit mask words, same three per-channel field clears, same mask update.
 * MATCHED round 62 by writing it in exactly that sibling's idiom -- direct
 * global expressions with NO cached locals. Four earlier rounds carried four
 * cached locals here (old60/old64/e228/e22c) and filed the result as an
 * unreachable register-identity stall; the caching was the whole residue.
 * The only structural difference from the sibling is that the lock is
 * released BEFORE the mask block rather than after it (retail's
 * `sw zero, D_8008E934` sits at 0x80031950, between the D_8008E228 load and
 * the first `or`). See docs/match-reports/func_80031890.md. */
s32 func_80031890(s16 idx)
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
    D_8008E934 = 0;
    D_80090C60 = mask0 | D_80090C60;
    D_80090C64 |= mask1;
    D_8008E228 &= ~D_80090C60;
    D_8008E22C &= ~D_80090C64;
    return 0;

fail:
    D_8008E934 = 0;
fail_nolock:
    return -1;
}
