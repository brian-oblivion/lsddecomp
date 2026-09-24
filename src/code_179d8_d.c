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
 * code_179d8_d -- window [100..119] of the original 274-function code_179d8
 * monolith, originally 0x1C440..0x1CC08 (vram 0x8002BC40..0x8002C408).
 *
 * ROUND 34 (head): the unit's FIRST SIX functions left it. CD_searchdir
 * (func_8002BC40), CD_cachefile (func_8002BCEC, the 175w stall), cd_read
 * (func_8002BFA8) and iso9660's own WEAK memcpy (func_8002C014) are the tail
 * of `libcd/iso9660.o` (Psy-Q 3.3), which starts in code_179d8_g; strcmp
 * (func_8002C048) and strncmp (func_8002C0AC) are `libc2/strcmp.o` and
 * `libc2/strncmp.o`. Five had been matched as C -- they were Sony's the whole
 * time, and reclassifying them out of the game count is the correction
 * CLAUDE.md asks for, not a regression. The unit is now 0x1C92C..0x1CC08
 * (vram 0x8002C12C..), New_Class6D940 onward, 14 functions. The ISO9660
 * directory-record views and diagnostic-string externs that lived here went
 * with the functions; the stall's preserved body is in its report.
 *
 * Carved MID-round 16, to re-staff a runner whose own unit was exhausted.
 * Screened 17/20 clean on the three-grep blocker census -- the highest of
 * any window in this monolith -- but that number overstates its value: 9 of
 * the 20 are 4-6 word leaves, and splat matched five of those itself as
 * empty `jr $ra; nop` bodies (func_8002C3C0/C3C8/C3F0/C3F8/C400 below).
 * Those five count as `matched` in tools/progress.py without having been
 * work, which is exactly the caveat CLAUDE.md attaches to that column.
 *
 * NEITHER OF THIS UNIT'S TWO "BLOCKED" FUNCTIONS IS BLOCKED.  Both were
 * filed as `addiu_at`, and `addiu_at` was RESOLVED in round 21 (maspsx
 * `--addiu-at`; docs/research/addiu-at-blocker.md).  Re-screened with
 * `python3 tools/nearmiss.py` on 2026-09-08 (round 24):
 *   func_8002BC40 (43w)   MATCHED round 24, 43/43, first attempt.
 *   func_8002BCEC (175w)  blocker-clean, and NOT GAME CODE. It lies fully
 *                         inside `libcd/iso9660.o` (Psy-Q 3.3), an object
 *                         already placed and verified against retail, so
 *                         no C matches it -- convert per
 *                         docs/SDK-OBJECTS-GUIDE.md, do not decompile.
 *                         553 lines of derivation were spent before
 *                         anyone asked. `tools/sdkstalls.py` asks.
 * Their stub reports are gone.  The previous version of this comment listed
 * both as "blocked, have stub reports", which by round 24 was a stale
 * DIRECTIVE over free ground -- the fourth unit in two rounds to carry one.
 * func_8002C278 was originally screened as a third (nop_mflo_mfhi) but
 * that screen was inverted (checked mult/div BEFORE mflo/mfhi instead of
 * after) -- the head corrected it mid-round and deleted the stub report.
 * It is fresh ground; the mult/mfhi pair in its body is retail's signed-
 * divide-by-constant idiom, not the blocked mflo/mfhi-then-mult direction.
 *
 * Unlike its siblings code_179d8_b and code_179d8_c, this slice owns NO
 * jump table -- all seven jtbl blocks in the 0xFD8 rodata slot fall outside
 * 0x8002BC40..0x8002C408 -- so no rodata sub-slot is attached to it.
 *
 * Sibling-slice finding worth having up front (established in code_179d8_b,
 * round 16): this region is NOT class-framework code. tools/classtable.py
 * --scan has no hit anywhere near these globals, and the neighbouring
 * functions read as a low-level serial/link driver poking raw control words
 * into a block of globals that look like hardware/SIO register staging. Do
 * not expect vtables here; do not go looking for a `this` pointer.
 *
 * Declarations: keep anything that encodes THIS unit's reading of the region
 * next to the code, in this file. Do NOT create a shared code_179d8*.h --
 * the sibling slices are staffed independently and a shared header is what
 * makes their merges collide.
 */
#include "common.h"

/*
 * D_8006D940: a function-pointer table this unit's own `New_Class6D940`/
 * `Class6D940__Class6D940` dispatch through. Named/typed as a plain local struct,
 * NOT claimed to be a class-framework vtable -- per this unit's header
 * comment (sibling-slice finding: no classtable.py hit anywhere near this
 * region). Only the two slots this unit's own functions reach are typed;
 * the rest stays opaque padding. Kept LOCAL to this file, not a shared
 * header, per this round's rule for code_179d8 slices.
 */
typedef struct Table6D940 Table6D940;
struct Table6D940 {
    u8 pad000[0x008];
    /* +0x008, New_Class6D940's own dispatch -- this IS Class6D940__Class6D940
     * itself (same 2-arg (self, arg1) shape). */
    void (*slot08)(void *self, s32 arg1);
    u8 pad00C[0x06C - 0x00C];
    /* +0x06C, Class6D940__Class6D940's own conditional dispatch. */
    void (*slot6C)(void *self, s32 arg1);
};
extern Table6D940 D_8006D940;

/* Forward-declared: defined below at its own ROM address (GetClass6D940Methods),
 * but called here (New_Class6D940, Class6D940__Class6D940) before that point in the
 * file. Without this, cc1 implicitly declares it `int`, which happens to be
 * byte-identical on this ABI but is a lie about the real signature. */
Table6D940 *GetClass6D940Methods(void);

/* The 0x34-byte object New_Class6D940 allocates. Only the fields
 * Class6D940__Class6D940 itself touches are named. */
typedef struct Obj6D940 Obj6D940;
struct Obj6D940 {
    Table6D940 *methods; /* +0x000, Class6D940__Class6D940 */
    u8 pad004[0x02C - 0x004];
    s32 unk2C;            /* +0x02C, Class6D940__Class6D940: zeroed */
    s32 unk30;             /* +0x030, Class6D940__Class6D940: zeroed */
};

/*
 * A second, DIFFERENT function-pointer table, reached only via the
 * uncarved accessor `GetActiveDataSourceMethods()` (not this unit's function to
 * define). Only the three slots this unit's functions dispatch through
 * are typed.
 */
typedef struct BaseTable6D940 BaseTable6D940;
struct BaseTable6D940 {
    u8 pad000[0x008];
    void (*slot08)(void *self); /* +0x008, Class6D940__Class6D940's own base-chain call */
    /* +0x00C, Class6D940__Destroy's own dispatch -- that function's whole body
     * is this one call with nothing after it, so its own return type is
     * genuinely ambiguous (a void wrapper around an s32 tail call is
     * byte-identical); typed s32 here per CLAUDE.md's rule to default to
     * `return callee(...)` absent positive void evidence. */
    s32 (*slot0C)(void *self);
    u8 pad010[0x064 - 0x010];
    /* +0x064, Class6D940__SetFlag's own dispatch -- same tail-call ambiguity as
     * slot0C above. */
    s32 (*slot64)(void *self);
};
extern BaseTable6D940 *GetActiveDataSourceMethods(void);

/* Pool allocator, already established elsewhere (e.g.
 * include/class_16334.h, include/code_8220.h) -- declared LOCAL here since
 * this unit does not include either header. */
extern void *BMemPMgrAlloc(s32 size);

void *New_Class6D940(s32 arg1)
{
    void *self;
    Table6D940 *table;

    self = BMemPMgrAlloc(0x34);
    if (self != NULL) {
        table = GetClass6D940Methods();
        table->slot08(self, arg1);
        return self;
    }
    return NULL;
}

void Class6D940__Class6D940(Obj6D940 *self, s32 arg1)
{
    GetActiveDataSourceMethods()->slot08(self);
    self->methods = GetClass6D940Methods();
    self->unk2C = 0;
    self->unk30 = 0;
    if (arg1 != 0) {
        self->methods->slot6C(self, arg1);
    }
}

s32 Class6D940__Destroy(void *self)
{
    return GetActiveDataSourceMethods()->slot0C(self);
}

s32 Class6D940__SetFlag(s32 *self)
{
    self[0xC] = 1;
    return GetActiveDataSourceMethods()->slot64(self);
}

/*
 * func_8002C278's own "descriptor" pointer, resolved either from a cached
 * byte offset (Obj278::unk34) or freshly from `index*12+8` into
 * Ctx278::unk10's byte array. Field meaning unestablished beyond
 * offset/width -- this region reads as raw hardware/SIO register staging
 * (no classtable.py hit anywhere nearby), not class-framework data.
 */
typedef struct Entry278 {
    u8 unk0;   /* +0x0, tag/kind: zero means "not present", tested first */
    u8 unk1;   /* +0x1 */
    u16 unk2;  /* +0x2 */
    u8 unk4;   /* +0x4 */
    u8 unk5;   /* +0x5 */
    s16 unk6;  /* +0x6 */
    s32 unk8;  /* +0x8 */
} Entry278;

/* func_8002C278's own object (its own `arg1`). Only the fields this
 * function itself touches are named. */
typedef struct Obj278 {
    u8 pad0[0xC];
    s32 unkC;   /* +0xC */
    s32 unk10;  /* +0x10 */
    s32 unk14;  /* +0x14 */
    u8 pad18[0x1A - 0x18];
    s16 unk1A;  /* +0x1A */
    u8 pad1C[0x2C - 0x1C];
    s16 unk2C;  /* +0x2C */
    s16 unk2E;  /* +0x2E */
    s32 unk30;  /* +0x30, cache-hit flag: 1 if unk34 was reused, 0 if freshly computed */
    s32 unk34;  /* +0x34, BEFORE resolution: a cached byte offset into Ctx278::unk10; AFTER: Entry278::unk8 */
    s32 unk38;  /* +0x38 */
} Obj278;

/* Opaque target of Ctx278::unk2C -- only the one dispatched slot named. */
typedef struct Ctx278SubMethods Ctx278SubMethods;
typedef struct Ctx278Sub Ctx278Sub;
struct Ctx278SubMethods {
    u8 pad000[0x080];
    /* (self, desc->unk2, index, obj): the last two are forwarded from
     * func_8002C278's own parameters, so they emit no set-up code. */
    s32 (*slot80)(Ctx278Sub *self, s32 unk2, s32 index, Obj278 *obj);
};
struct Ctx278Sub {
    Ctx278SubMethods *methods;
};

/* func_8002C278's own `arg0`. Only the fields this function itself
 * touches are named. */
typedef struct Ctx278 {
    u8 pad0[0x10];
    u8 *unk10;       /* +0x10, byte-addressed base for the Entry278 table */
    u8 pad14[0x2C - 0x14];
    Ctx278Sub *unk2C; /* +0x2C */
} Ctx278;

s32 func_8002C278(Ctx278 *ctx, Obj278 *self, s32 index)
{
    Entry278 *desc;
    s32 whole;
    s32 frac;
    s32 unk2;

    if (index < 0x190) {
        if (self->unk34 != 0) {
            desc = (Entry278 *)(ctx->unk10 + self->unk34);
            self->unk30 = 1;
        } else {
            desc = (Entry278 *)(index * 12 + 8 + ctx->unk10);
            self->unk30 = 0;
        }
        self->unk34 = desc->unk8;
        if (desc->unk0 != 0) {
            whole = index / 20;
            frac = index - whole * 20;
            self->unkC = (frac << 11) + 0x400;
            self->unk10 = (s32)desc->unk6 << 11;
            self->unk14 = (whole << 11) + 0x400;
            self->unk1A = desc->unk5 << 10;
            self->unk2C = desc->unk1;
            self->unk2E = desc->unk4;
            unk2 = desc->unk2;
            self->unk38 = unk2;
            return ctx->unk2C->methods->slot80(ctx->unk2C, unk2, index, self);
        }
        return -1;
    }
    return 0;
}


Table6D940 *GetClass6D940Methods(void)
{
    return &D_8006D940;
}

s32 func_8002C3B8(void)
{
    return 0;
}

void func_8002C3C0(void) {
}

void func_8002C3C8(void) {
}

void func_8002C3D0(void)
{
    char buf[0x40];
}

void func_8002C3E0(void)
{
    char buf[0x40];
}

void func_8002C3F0(void) {
}

void func_8002C3F8(void) {
}

void func_8002C400(void) {
}
