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
 * class_3bb8c_t -- functions 96..112 of the 113-function `class_3bb8c_n`
 * remainder, 0x48738..0x48F74 (vram 0x80057F38..0x80058774).  Carved
 * MID-round 17 (2026-09-04) to re-staff a runner whose own unit was
 * exhausted.  This is the LAST slice of the class_3bb8c block.
 *
 * Blocker census, three-grep screen run per function at carve time:
 * 16 of 17 clean.
 *
 * NOT BLOCKED.  This unit's one "blocked" function was blocked on `addiu_at`
 * ALONE, and `addiu_at` was RESOLVED in round 21 (maspsx `--addiu-at`;
 * docs/research/addiu-at-blocker.md).
 *   func_800585B4 (56w)  MATCHED round 41 (2026-09-14) from a first-ever
 *                        permuter search seeded on round 24's stall (was
 *                        55/56 words, 1 short, 15/56 raw).  See its match
 *                        report for the derivation -- the fix was a pair
 *                        of local pointer caches that change how cc1
 *                        strength-reduces the D_80087BD4[i] access; the
 *                        day-log struct round 24 established is unchanged.
 * The previous version of this comment read "BLOCKED, stub report already
 * filed, do NOT spend attempts on it" -- a stale DIRECTIVE over workable
 * ground, and the fifth of its kind found in round 24.
 *
 * Four of the 17 are 2-instruction leaves that splat matched itself.
 *
 * This slice holds D_8001176C and D_80011778, the two strings left
 * STANDALONE when the 0x1EF4 rodata slot was split -- referenced from
 * func_80057FEC and func_80058084, both of which are in this unit.  A
 * string is referenced by SYMBOL and a standalone rodata object resolves
 * that fine (the `code_8220` / 0xA8C precedent in Gate 2), so no attach was
 * needed and the link came up green, which is the check that settles it.
 * The slice owns no `jtbl_` reference either.
 *
 * EXPECT THIS SLICE TO SPAN MORE THAN ONE CLASS.  It is cut at ROM
 * addresses, not class boundaries.  Identify each with tools/classtable.py.
 */
#include "common.h"

/* The class allocated by this unit's own func_80057F68, table D_800879C4
 * (49 slots, resolved via tools/classtable.py). Its ctor (func_80057D10)
 * and its own funcs 80057F38/40/48/50 live in the neighbouring
 * `class_3bb8c_p` unit (round 2026-09-04 earlier this round), which
 * already carries its own local view of this table
 * (`D_800879C4Methods`/`D_800879C4Obj` in that file). This unit's own
 * view is kept separate per the multiple-independent-local-views
 * convention -- func_80057F58 itself needs no fields, only the address. */
typedef struct D_800879C4Table D_800879C4Table;
extern D_800879C4Table D_800879C4;

extern void *func_80017B34(s32 size);

typedef struct D_80087AACObj D_80087AACObj;

/* This unit's own local view of the shared base-class table returned by
 * Get_vtable_TaskCore() (a plain no-argument getter, established elsewhere --
 * e.g. include/code_2c054.h, include/class_3bb8c.h -- as returning
 * &gTaskCoreMethods). Only the slots this unit's own functions dispatch
 * through are typed, per the project's "per-call-site signature"
 * convention (multiple units already carry independent local views of
 * this same table with different slot arities). */
typedef struct D_8006E730Methods {
    u8 pad00[0x8];
    /* +0x008, called by this unit's own func_80057FC8 as (self, 0, str,
     * 0). */
    void (*slot8)(D_80087AACObj *self, s32 arg1, char *str, s32 arg2);
    u8 pad0C[0x44 - 0xC];
    /* +0x044, called by this unit's own func_80058390 as (self, arg1,
     * arg2). */
    void (*slot44)(D_80087AACObj *self, void *arg1, void *arg2);
    u8 pad48[0x5C - 0x48];
    /* +0x05C, called by this unit's own func_800580E0 as (self, arg1,
     * arg2). */
    void (*slot5C)(D_80087AACObj *self, void *arg1, void *arg2);
    u8 pad60[0xDC - 0x60];
    /* +0x0DC, called by this unit's own func_80058308 as (self) -- the
     * base-class dtor step. */
    void (*slotDC)(D_80087AACObj *self);
    /* +0x0E0, called by this unit's own func_80058404 as (self, arg1) --
     * the FIRST thing that function does, before touching anything else
     * (round 19). */
    void (*slotE0)(D_80087AACObj *self, void *arg1);
} D_8006E730Methods;
extern D_8006E730Methods *Get_vtable_TaskCore(void);

void func_80057F38(void) {
}

void func_80057F40(void) {
}

void func_80057F48(void) {
}

void func_80057F50(void) {
}

D_800879C4Table *func_80057F58(void) {
    return &D_800879C4;
}

/* The class allocated below, table D_80087AAC (73 slots, resolved via
 * tools/classtable.py). This unit owns the whole class -- ctor, dtor and
 * every slot referenced from within it are all in this file. Only the
 * fields/slots each function actually touches are typed; the rest stay
 * opaque so the struct keeps the right size without requiring every
 * method to be named up front. */
typedef struct D_80087AACMethods {
    u8 pad00[0x8];
    /* +0x008, this unit's own ctor (func_80057FC8). */
    D_80087AACObj *(*ctor)(D_80087AACObj *self, void *arg1);
    u8 pad0C[0x40 - 0xC];
    /* +0x040, called by this unit's own func_80057FC8 as (self, arg1) --
     * a TAIL CALL, its return value forwarded as func_80057FC8's own. */
    void *(*slot40)(D_80087AACObj *self, void *arg1);
    u8 pad44[0x6C - 0x44];
    /* +0x06C, called by this unit's own func_80058078 as (self, flag). */
    void (*slot6C)(D_80087AACObj *self, s32 arg1);
    /* +0x070, called by this unit's own func_800581C4 as (self, size). */
    void (*slot70)(D_80087AACObj *self, s32 arg1);
    u8 pad74[0x94 - 0x74];
    /* +0x094, called by this unit's own func_800581C4 as (self). */
    void (*slot94)(D_80087AACObj *self);
    u8 pad98[0xD4 - 0x98];
    /* +0x0D4, called by this unit's own func_80058078 as (self, str,
     * 0). */
    void (*slotD4)(D_80087AACObj *self, char *str, s32 arg2);
    /* +0x0D8, called by this unit's own func_80057FC8 as (self, 0). */
    void (*slotD8)(D_80087AACObj *self, s32 arg1);
    u8 padDC[0x124 - 0xDC];
    /* +0x124, this unit's own func_80058694, called by func_800580E0 as
     * (self). */
    void (*slot124)(D_80087AACObj *self);
} D_80087AACMethods;
extern D_80087AACMethods *func_80058764(void);

/* This unit's own view of one entry of D_80087AACObj::unk_0xA8 -- only
 * the one slot this unit's own func_80058694 dispatches through is
 * typed. */
typedef struct D_80087AACEntry D_80087AACEntry;
typedef struct D_80087AACEntryMethods {
    u8 pad00[0x4];
    /* +0x004, called by this unit's own func_80058308 as (self) -- a
     * per-entry destructor, in a 100-iteration loop over unk_0xA8. */
    void (*slot4)(D_80087AACEntry *self);
    u8 pad08[0x60 - 0x8];
    /* +0x060, called by this unit's own func_800580E0 as (self, flag). */
    void (*slot60)(D_80087AACEntry *self, s32 arg1);
    u8 pad64[0xB8 - 0x64];
    /* +0x0B8, called by this unit's own func_80058694 as (self, 1,
     * &global). */
    void (*slotB8)(D_80087AACEntry *self, s32 arg1, void *arg2);
    u8 padBC[0xC4 - 0xBC];
    /* +0x0C4, called by this unit's own func_80058404 as (self, arg1,
     * &point, 0), where `point` is a 2-word {x, y}-shaped local (round
     * 19). */
    void (*slotC4)(D_80087AACEntry *self, void *arg1, s32 *point, s32 arg3);
} D_80087AACEntryMethods;
struct D_80087AACEntry {
    D_80087AACEntryMethods *methods;
};

/* Object pointed to by D_80087AACObj::unk_0x48 -- only the one slot
 * this unit's own func_80057FC8 dispatches through is typed. */
typedef struct D_80087AACUnk48Obj D_80087AACUnk48Obj;
typedef struct D_80087AACUnk48Methods {
    u8 pad00[0x9C];
    /* +0x09C, called by this unit's own func_80057FC8 as (self, -1). */
    void (*slot9C)(D_80087AACUnk48Obj *self, s32 arg1);
} D_80087AACUnk48Methods;
struct D_80087AACUnk48Obj {
    D_80087AACUnk48Methods *methods;
};

/* Object pointed to by D_80087AACObj::unk_0xA4 -- passed in as this
 * unit's own ctor's `arg1` (func_80057FC8) and dispatched through by
 * func_800580E0/func_80058404 (both still queued at slot +0x1B0). Only
 * that one slot is typed. */
typedef struct D_80087AACUnkA4Obj D_80087AACUnkA4Obj;
/* Return type of D_80087AACUnkA4Methods::slot1B0.
 *
 * It is a DAY-LOG object, established by func_800585B4 (round 24): the two
 * fields func_800580E0 reads at +0x4/+0x8 are a mode flag and a live day
 * count, and +0x18 is a 365-entry halfword year ring whose length is fixed
 * by func_800585B4's wrap constant (the index resets to 0x16C == 364 when
 * it goes negative, so 365 entries).  Extended ADDITIVELY -- +0x4 and +0x8
 * keep their offsets, so func_800580E0's codegen is unaffected.
 *
 * LEAD, not a claim: a 365-entry log of 2-byte points is the shape of
 * `MoodGraphPoint moodPreviousDays[365]` in include/DreamSys.h, and the
 * `lh` accesses here are consistent with MoodGraphPoint being 2 bytes.  But
 * the OFFSETS do not line up -- DreamSys puts that array far deeper than
 * +0x18 -- so this is a different object keeping its own year log, not
 * DreamSys under another name.  Kept LOCAL to this unit; do not include
 * DreamSys.h to chase the resemblance, it would create header contention
 * this unit does not currently have. */
typedef struct D_80087AACUnkA4Result {
    u8 pad00[0x4];
    /* +0x004, nonzero means "scan the full 100-day window regardless of how
     * many days are actually logged". */
    s32 unk_0x4;
    /* +0x008, days logged so far; also the ring's write cursor. */
    s32 unk_0x8;
    u8 pad0C[0x18 - 0xC];
    /* +0x018, the year ring, walked backwards from unk_0x8 - 1. */
    s16 days[365];
    u8 pad2F2[0x467 - 0x2F2];
    /* +0x467, set once func_800585B4's scan has succeeded. */
    s8 scored;
} D_80087AACUnkA4Result;
typedef struct D_80087AACUnkA4Methods {
    u8 pad00[0x1B0];
    /* +0x1B0, called by this unit's own func_800580E0/func_80058404 as
     * (self, 0). */
    D_80087AACUnkA4Result *(*slot1B0)(D_80087AACUnkA4Obj *self, s32 arg1);
} D_80087AACUnkA4Methods;
struct D_80087AACUnkA4Obj {
    D_80087AACUnkA4Methods *methods;
};

struct D_80087AACObj {
    D_80087AACMethods *methods;
    u8 pad04[0x1C - 0x4];
    /* +0x01C, read by this unit's own func_80058694 (unsigned
     * comparisons -- `sltiu`). */
    u32 unk_0x1C;
    u8 pad20[0x2C - 0x20];
    /* +0x02C, written by this unit's own func_80058078. */
    s32 unk_0x2C;
    u8 pad30[0x38 - 0x30];
    /* +0x038, read by this unit's own func_80058390. */
    s32 unk_0x38;
    /* +0x03C, read by this unit's own func_800580E0. */
    s32 unk_0x3C;
    u8 pad40[0x48 - 0x40];
    /* +0x048, read by this unit's own func_80057FC8. */
    D_80087AACUnk48Obj *unk_0x48;
    u8 pad4C[0x84 - 0x4C];
    /* +0x084, written by this unit's own func_80058078. */
    s32 unk_0x84;
    u8 pad88[0xA4 - 0x88];
    /* +0x0A4, set by this unit's own ctor (func_80057FC8) to its own
     * `arg1`; dispatched through by func_800580E0/func_80058404. */
    D_80087AACUnkA4Obj *unk_0xA4;
    /* +0x0A8, a 100-entry array of `D_80087AACEntry *` -- built by this
     * unit's own func_80058228 (still queued), destroyed by
     * func_80058308 (still queued), indexed by func_80058694. */
    D_80087AACEntry *unk_0xA8[100];
    /* +0x238, read by this unit's own func_800581C4/func_80058390. */
    s32 unk_0x238;
    /* +0x23C, read/written by this unit's own func_80058694 (unsigned
     * comparison -- `sltiu`). */
    u32 unk_0x23C;
    /* +0x240, read by this unit's own func_80058694. */
    s8 *unk_0x240;
};

void *func_80057F68(void *arg1) {
    void *obj = func_80017B34(0x244);
    if (obj != NULL) {
        func_80058764()->ctor(obj, arg1);
        return obj;
    }
    return NULL;
}

extern char D_8001176C[];

void *func_80057FC8(D_80087AACObj *self, void *arg1) {
    Get_vtable_TaskCore()->slot8(self, 0, D_8001176C, 0);
    self->methods = func_80058764();
    self->unk_0x48->methods->slot9C(self->unk_0x48, -1);
    self->unk_0xA4 = arg1;
    self->methods->slotD8(self, 0);
    return self->methods->slot40(self, arg1);
}

extern char D_80011778[];

void func_80058078(D_80087AACObj *self) {
    self->unk_0x84 = 5;
    self->unk_0x2C = 0x190;
    self->methods->slotD4(self, D_80011778, 0);
    self->methods->slot6C(self, 0xA);
}

void func_800580E0(D_80087AACObj *self, void *arg1, void *arg2) {
    Get_vtable_TaskCore()->slot5C(self, arg1, arg2);
    if (self->unk_0x3C == 1) {
        D_80087AACUnkA4Result *result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0);
        if (result->unk_0x4 != 0 || result->unk_0x8 != 0) {
            self->unk_0xA8[0]->methods->slot60(self->unk_0xA8[0], self->unk_0x1C & 1);
        }
    }
    self->methods->slot124(self);
}

void func_800581C4(D_80087AACObj *self) {
    if (self->unk_0x238 == 0) {
        self->methods->slot70(self, 0x10);
        self->methods->slot94(self);
    }
}

/* A 3-byte colour-ish triple, read/written strictly byte-for-byte in
 * DECLARATION order (round 19, verified against retail byte-for-byte --
 * the natural sequential order is what matches, no reordering needed).
 * Field names are a plausible RGB reading of a colour-cycling table
 * builder, not confirmed evidence; see this function's own match report. */
typedef struct D_8008ABB8Color {
    s8 r;
    s8 g;
    s8 b;
} D_8008ABB8Color;
extern u8 D_8008ABAC;
extern u8 D_8008ABB4;
extern D_8008ABB8Color D_8008ABB8;
extern D_80087AACEntry *func_800404D0(void *a0, void *a1, s32 a2);

void func_80058228(D_80087AACObj *self) {
    D_8008ABB8Color rgb;
    s32 i;

    self->unk_0xA8[0] = func_800404D0(&D_8008ABAC, &D_8008ABB4, 0);
    rgb = D_8008ABB8;
    for (i = 1; i < 100; i++) {
        s32 dec;

        self->unk_0xA8[i] = func_800404D0(&D_8008ABAC, &rgb, 0);
        dec = 1;
        if (i < 7) {
            dec = 0x14;
        }
        rgb.r -= dec;
        rgb.g -= dec;
        rgb.b -= dec;
    }
    self->unk_0x240 = func_80017B34(4);
}

extern void func_80017CFC(void *arg);

void func_80058308(D_80087AACObj *self) {
    s32 i;

    func_80017CFC(self->unk_0x240);
    for (i = 0; i < 100; i++) {
        self->unk_0xA8[i]->methods->slot4(self->unk_0xA8[i]);
    }
    Get_vtable_TaskCore()->slotDC(self);
}

s32 func_80058390(D_80087AACObj *self, void *arg1, void *arg2) {
    s32 result;
    Get_vtable_TaskCore()->slot44(self, arg1, arg2);
    result = 2;
    if (self->unk_0x238 == 0) {
        result = self->unk_0x38;
    }
    return result;
}

extern s32 func_800585B4(D_80087AACObj *self, D_80087AACUnkA4Result *arg1);

/* A 2-word {x, y}-shaped point, matching what this unit's own func_80058404
 * passes to D_80087AACEntryMethods::slotC4 (round 19). */
typedef struct Point2 {
    s32 x, y;
} Point2;

void func_80058404(D_80087AACObj *self, void *arg1) {
    D_80087AACUnkA4Result *result;
    s32 count;
    s32 i;
    s32 idx;
    s32 flag;
    Point2 point;
    Point2 firstPoint;

    Get_vtable_TaskCore()->slotE0(self, arg1);
    result = self->unk_0xA4->methods->slot1B0(self->unk_0xA4, 0);
    self->unk_0x238 = func_800585B4(self, result);

    flag = 0;
    if (result->unk_0x4 != 0) {
        count = 100;
    } else {
        count = result->unk_0x8;
        if (count >= 0x65) {
            count = 100;
        }
    }

    idx = result->unk_0x8 - 1;
    for (i = 0; i < count; i++, idx--) {
        s8 *p;
        s8 dx, dy;
        s32 ndy;

        if (idx < 0) {
            idx = 0x16C;
        }
        p = (s8 *)((u8 *)result + idx * 2);
        dx = p[0x18];
        point.x = dx * 10 - 5;
        dy = p[0x19];
        ndy = -dy;
        point.y = ndy * 10 - 5;

        if (i == 0) {
            firstPoint = point;
            flag = 1;
        } else {
            self->unk_0xA8[i]->methods->slotC4(self->unk_0xA8[i], arg1, (s32 *)&point, 0);
        }
    }

    if (flag) {
        self->unk_0xA8[0]->methods->slotC4(self->unk_0xA8[0], arg1, (s32 *)&firstPoint, 0);
    }
}

/* Four halfword targets, 0x01FF/0x0101/0x0000/0xFD00 -- exactly the i < 4
 * bound below, which is why the loop count is the table's length and not a
 * coincidence. */
extern s16 D_80087BD4[4];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `p` and `days`
 * are LOCAL pointer caches of D_80087BD4 and log->days respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing D_80087BD4[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { p = D_80087BD4; }` branch below and the
 * `p = (days = D_80087BD4);` chained assignment are BOTH semantically
 * inert -- p is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
s32 func_800585B4(D_80087AACObj *self, D_80087AACUnkA4Result *log)
{
    u32 i;
    s16 *days;
    s32 j;
    s16 *p;
    s32 idx;
    s32 found;
    s32 limit;

    if (log->scored != 0) {
        goto fail;
    }

    if (log->unk_0x4 != 0) {
        limit = 100;
    } else {
        limit = log->unk_0x8;
        if (limit > 100) {
            limit = 100;
        }
    }

    for (i = 0; i < 4; i++) {
        found = 0;
        idx = log->unk_0x8 - 1;
        for (j = 0; j < limit; j++) {
            if (idx < 0) {
                idx = 0x16C;
            } else {
                p = D_80087BD4;
            }
            p = (days = D_80087BD4);
            days = log->days;
            if (p[i] == days[idx]) {
                self->unk_0x240[i] = j;
                found++;
            }
            idx--;
        }
        if (found == 0) {
            goto fail;
        }
    }

    log->scored = 1;
    self->unk_0x23C = 0;
    return 1;

fail:
    return 0;
}

extern s32 D_8008ABBC;

void func_80058694(D_80087AACObj *self) {
    if (self->unk_0x238 != 0) {
        if (self->unk_0x1C >= 0x1F) {
            if (self->unk_0x23C < 4) {
                if ((self->unk_0x1C % 24) == 0) {
                    s8 idx = self->unk_0x240[self->unk_0x23C];
                    self->unk_0xA8[idx]->methods->slotB8(self->unk_0xA8[idx], 1, &D_8008ABBC);
                    self->unk_0x23C += 1;
                }
            }
        }
    }
}

extern D_80087AACMethods D_80087AAC;

D_80087AACMethods *func_80058764(void) {
    return &D_80087AAC;
}
