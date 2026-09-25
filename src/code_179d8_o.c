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
 * code_179d8_o -- functions 0..3 of the original 274-function code_179d8
 * monolith, 0x179d8..0x17AD0 (vram 0x800271D8..0x800272D0).  Carved round 26
 * (2026-09-09): the FRONT window of the head remainder, taken because it is
 * the cheapest blocker-clean ground left in the executable.
 *
 * Blocker census at carve time, four screens per function (gp_rel, forward
 * nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   New_Class6D4E8 (20w)  CLEAN
 *   Class6D4E8__Class6D4E8   (19w)  CLEAN
 *   Class6D4E8__Destroy   (21w)  CLEAN
 *   Class6D4E8__NoOpSlot40    (2w)  CLEAN  -- a bare `jr $ra; nop` leaf
 * 4 of 4 clean, and the window references NO rodata or data symbol at all
 * (zero `%hi`/`%lo` in the whole slice), so no rodata sub-slot is attached.
 *
 * The head remainder that keeps the name `code_179d8` starts immediately
 * after this unit and is gp_rel-saturated (33 of its 39 functions), which is
 * why the cut is here: this window carries neither a jump table nor any
 * blocked-function stub-report debt.
 *
 * This unit calls out to GetClass6D4E8Methods and InitCdDrive, which are still
 * INCLUDE_ASM in that remainder -- their prototypes belong in THIS file, not
 * in a shared header (this unit has none and should not acquire one).
 *
 * `New_Class6D4E8` carries a name inherited from FirecatFG. Treat it as a
 * HYPOTHESIS, not evidence: see CLAUDE.md on the hand-rolled class framework
 * and resolve any method-table slot with tools/classtable.py rather than by
 * counting.
 *
 * Round 26 (echo): resolved via `tools/classtable.py 0x8006D4E8 --vs
 * 0x8006B58C`.  D_8006D4E8 (this class's own 29-slot method table, the
 * address the FirecatFG name is drawn from) overrides BasicClass's table
 * (D_8006B58C, 14 slots, header 0) at exactly three slots: +0x004
 * (DestroyChained), +0x008 (Class6D4E8__Class6D4E8) and +0x00C (Class6D4E8__Destroy);
 * slots +0x010..+0x038 are inherited verbatim (same BasicClass__func_*
 * addresses in both tables) and the rest (+0x040 upward, including
 * Class6D4E8__NoOpSlot40) are new slots BasicClass's own table does not have at
 * all. +0x008 is BasicClass's own constructor slot in D_8006B58C
 * (BasicClass__BasicClass sits there) -- CONFIRMED by that lookup, not
 * assumed from the FirecatFG name.  So: `New_Class6D4E8` is a genuine
 * "allocate + construct" pair, and its constructor IS this same unit's
 * `Class6D4E8__Class6D4E8`, dispatched back through the class's own table (the
 * generic `new` doesn't call Class6D4E8__Class6D4E8 by name -- it fetches
 * GetClass6D4E8Methods()'s table and calls whatever sits at the ctor slot, which
 * happens to resolve to Class6D4E8__Class6D4E8 for this class).
 *
 * This unit's own local view of the class-table framework it participates
 * in: NOT the same shape as include/class_3ac78.h's GenericObject /
 * GenericMethodsHeader (that hierarchy's ctor sits at +0x04C) -- a
 * DIFFERENT, independent class hierarchy elsewhere in the project.  Kept
 * local per the project's multiple-independent-local-views convention.
 */
#include "common.h"

typedef struct Class6D4E8Methods Class6D4E8Methods;
struct Class6D4E8Methods {
    s32 header;                    /* +0x000, not a pointer -- 0x13 (DATASOURCE_CD in code_171e0.c) for D_8006D4E8 */
    void *ownDtorChain;            /* +0x004, DestroyChained -- unused by this unit's own functions */
    void (*ctor)(void *self);      /* +0x008, Class6D4E8__Class6D4E8 (classtable.py; BasicClass__BasicClass
                                     * and Class6D430__Class6D430 sit at this slot in the base tables) */
    u8 pad00C[0x05C - 0x00C];
    void (*freeBuffer)(void *self);      /* +0x05C, inherited Class6D430__FreeBuffer */
    u8 pad060[0x074 - 0x060];
    void (*cancelRequests)(void *self);  /* +0x074, Class6D4E8__CancelRequests (code_179d8_q), which reads
                                           * its `self` in $a0.  Round 79 correction: this slot was typed
                                           * `void (*)(void)` on the strength of the nop delay slot, but $a0
                                           * still holds Class6D4E8__Destroy's own `self` there (the entry
                                           * `move s0,a0` leaves it intact), so passing `self` is
                                           * byte-identical -- the same shape as Class6D430__Destroy's
                                           * `onBufferChanged(this)` in code_171e0.c. */
};

typedef struct {
    Class6D4E8Methods *methods;      /* +0x000, set by the constructor to this class's own table
                                     * (GetClass6D4E8Methods()'s return value) -- the table-pointer-at-offset-0
                                     * convention CLAUDE.md documents for this project's class framework */
    u8 pad004[0x028 - 0x004];
    s16 unk28;                     /* +0x028, cleared by the constructor (a halfword store, `sh`); Class6D430's
                                     * own ctor already clears it too (code_171e0.h: u16 unk28). No reader
                                     * in this unit, so no name. */
} Class6D4E8;

/* The parent class's table (D_8006D430, Class6D430Methods in
 * include/code_171e0.h), viewed only down to the one slot this unit's
 * constructor uses: +0x008 is Class6D430__Class6D430. */
typedef struct {
    u8 pad0[0x008];
    void (*ctor)(void *self);      /* +0x008 */
} Class6D430CtorView;

extern void *BMemPMgrAlloc(s32 size);              /* Psy-Q allocator, matched signature used project-wide */
/* ROUND 59 (extern review): parameter list only, return type untouched. This
 * is NOT still INCLUDE_ASM -- it is defined in src/code_171e0.c as
 * `void *GetClass6D430Methods(void)`, and the callee at 0x80026C9C is
 * lui/addiu/jr reading no argument register. Measured before changing it:
 * Class6D4E8__Class6D4E8's `jal 80026c9c` (0x80027234) carries `move s0,a0` in the
 * delay slot -- a callee-save spill, not argument setup -- so the `self`
 * passed below costs zero bytes and the one-parameter prototype was simply
 * false. Unspecified parameters keep Class6D4E8__Class6D4E8's call site untouched. */
extern Class6D430CtorView *GetClass6D430Methods();
extern Class6D4E8Methods *GetClass6D4E8Methods(void); /* code_179d8_q: returns &D_8006D4E8, this class's table */
extern void InitCdDrive(void);                        /* code_179d8_q: one-shot CdSetDebug(0) + CdlSetmode double speed */

Class6D4E8 *New_Class6D4E8(void)
{
    Class6D4E8 *self;

    self = BMemPMgrAlloc(0x2C);
    if (self != NULL) {
        GetClass6D4E8Methods()->ctor(self);
        return self;
    }
    return NULL;
}

void Class6D4E8__Class6D4E8(Class6D4E8 *self)
{
    GetClass6D430Methods(self)->ctor(self);
    self->methods = GetClass6D4E8Methods();
    self->unk28 = 0;
    InitCdDrive();
}

void Class6D4E8__Destroy(Class6D4E8 *self)
{
    self->methods->cancelRequests(self);
    self->methods->freeBuffer(self);
}

void Class6D4E8__NoOpSlot40(void)
{
}
