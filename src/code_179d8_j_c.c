/*
 * code_179d8_j_c -- the TAIL of the old code_179d8_j slice, split off in round
 * 34 (2026-09-12) when Sony's `libsnd/ut_pb.o` was linked into the middle of
 * `code_179d8_j_b`.  Now 0x22244..0x2273C (vram 0x80031A44..0x80031F3C), eight
 * functions (func_80031A44 .. func_80031EE8).
 *
 * WHY THE SPLIT EXISTS.  `func_800319B4` is Sony's `SsUtPitchBend`
 * (`libsnd/ut_pb`, Psy-Q 3.6 -- the only disc carrying the module; 0x90 text
 * covering exactly that one function).  It had been MATCHED as C;
 * reclassifying it out of the game count is the correction CLAUDE.md asks for,
 * not a regression.  A placed object cannot live inside a `c` segment, so
 * `code_179d8_j_b` became [c][o][c] and this half needed its own name.
 *
 * This is the SECOND split of the same original slice in the same round --
 * `libsnd/vm_prog` took 0x20FF0..0x21180 first, which is what created
 * `code_179d8_j_b`.  Hence the `_c` suffix: `_b` was already taken.  The
 * precedent for a second-generation split name is the yaml's own note on
 * `<unit>_b` / `<unit>_c`.
 *
 * NO RODATA ATTACH CAME WITH THIS HALF, and that is measured, not assumed: the
 * old code_179d8_mid_c monolith contains zero `jtbl_` and zero `.word .L`
 * across its whole extent, and the splat yaml's rodata slot list names none of
 * `code_179d8_j`, `_j_b` or `_j_c`.  Unlike round 33's code_179d8_c_b there
 * was nothing to move, and a link failure of the form
 * `undefined reference to '.L8003....'` would mean something else.
 *
 * DECLARATIONS: this file carries its own copy of what its functions use,
 * split out of the old shared block.  Keep it that way -- do NOT create a
 * shared code_179d8*.h.  The sibling slices are staffed independently and a
 * shared header is what makes their merges collide; see
 * `python3 tools/headercontention.py`.
 *
 * BLOCKER PROFILE: screen with `python3 tools/nearmiss.py`, never by
 * re-implementing the greps and never for `addiu_at` (resolved round 21).
 * `nearmiss.py` runs `tools/sdkstalls.py` for you.  func_80031A44 carries a
 * HEAD SALVAGE body in its report (84/88 words, round 31) -- read the report
 * before starting, it is not cold ground.
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
extern s32 func_8002FAC4(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4, u16 a5);
extern s32 func_800300D0(s32 a0, s16 a1, s16 a2, u16 a3);
extern s32 SpuVmVSetUp(s16 a0, s16 a1);
extern s16 func_8002F3E8(s16 a0, s32 a1, s16 a2, s16 a3, u16 a4);
extern void func_8002E308(s16 a0, s16 a1, s16 a2, s16 a3);
extern void func_8002E874(s16 a0, s16 a1, s16 a2, s16 a3);

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

/* 16 (0x10)-byte-stride record with two s16 fields 2 bytes apart --
 * modeled as ONE struct here (not two independent arrays) because
 * func_80030404 computes the second field's address as the first
 * field's cached base register plus a compile-time +0x2, which only
 * happens when the compiler knows both offsets belong to the same
 * object. */
typedef struct {
    s16 unk0; /* +0x0 */
    s16 unk2; /* +0x2 */
    u8 pad4[0x10 - 0x4];
} Rec16D7F0;
extern Rec16D7F0 D_8008D7F0[];
extern Rec16D7F0 D_8008D7F4[]; /* independent array, same shape */

/* Per-slot flag byte, same 0..0x17 id as several of the tables above. */
extern u8 D_8008D970[];

/* STALL -- see docs/match-reports/func_80031A44.md.  HEAD SALVAGE, round 31,
 * confirmed round 32 (permuter, ~54k iterations, not closed). Round 36:
 * rebuilt with SpuVmVSetUp's real name (was func_80032148 in the report's
 * preserved body -- round 34's SDK conversion renamed the callee, and the
 * report's body was never corrected). Measured 84/88 words, exact length,
 * matching the prior figure exactly; a barrier between the D_8008EA26/
 * D_8008EA22 stores (the one untested lever the report flagged) blows the
 * function up drastically instead of fixing the swap -- see this round's
 * report update. */
INCLUDE_ASM("asm/nonmatchings/code_179d8_j_c", func_80031A44);

/* Rec34D994 (D_8008D994/99A/99E) and Rec16D7F0 (D_8008D7F8/D_8008D7FA
 * below) are declared once, near the top of this file, and shared by
 * every function in this unit that needs them -- see the comment there.
 * See func_80031CF0's report for why D_8008D7F8/D_8008D7FA are modeled
 * as independent arrays rather than fields of one struct (each access
 * computes its own address), unlike D_8008D7F0/D_8008D7F4 above. */
extern Rec16D7F0 D_8008D7F8[];
extern Rec16D7F0 D_8008D7FA[];

/* `dead` is never read and the write is unreachable; it exists to make GCC
 * allocate retail's empty 8-byte frame, which is what puts the two
 * stack-passed arguments at 0x18/0x1C($sp) instead of 0x10/0x14.  See this
 * function's match report -- the frame is the ONLY thing the idiom is for,
 * and adding anything else on top of it breaks the scheduling. */
s32 func_80031BA4(s16 idx, s16 p1, s16 p2, s16 p3, u16 p4, u16 p5) {
    s32 dead[2];

    if ((u16)idx < 0x18) {
        if (D_8008D99E[idx].unk0 != p1) {
            return -1;
        }
        if (D_8008D99A[idx].unk0 != p2) {
            return -1;
        }
        if (D_8008D994[idx].unk0 != p3) {
            return -1;
        }
        D_8008D7F8[idx].unk0 = p4;
        __asm__("");
        D_8008D7FA[idx].unk0 = p5;
        __asm__("");
        D_8008D970[idx] |= 0x30;
        return 0;
    }
    if (0) {
        dead[0] = 1;
    }
    return -1;
}

s32 func_80031C98(s16 idx, s16 *out1, s16 *out2)
{
    if ((u16) idx < 0x18) {
        *out1 = D_8006DAD4[idx].unk0;
        *out2 = D_8006DAD4[idx].unk2;
        return 0;
    }
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_c", func_80031CF0);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_c", func_80031D6C);

INCLUDE_ASM("asm/nonmatchings/code_179d8_j_c", func_80031DF8);

s32 func_80031E94(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        func_8002E308(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}

s32 func_80031EE8(s16 p0, s16 p1, s16 p2, s16 p3)
{
    if ((u16) p0 < 0x18) {
        func_8002E874(p0, p1, p2, p3);
        return 0;
    }
    return -1;
}
