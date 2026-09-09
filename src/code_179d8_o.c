/*
 * code_179d8_o -- functions 0..3 of the original 274-function code_179d8
 * monolith, 0x179d8..0x17AD0 (vram 0x800271D8..0x800272D0).  Carved round 26
 * (2026-09-09): the FRONT window of the head remainder, taken because it is
 * the cheapest blocker-clean ground left in the executable.
 *
 * Blocker census at carve time, four screens per function (gp_rel, forward
 * nop_mflo_mfhi, `jr $t2` trampoline, jtbl):
 *   new_class_6d4e8 (20w)  CLEAN
 *   func_80027228   (19w)  CLEAN
 *   func_80027274   (21w)  CLEAN
 *   func_800272C8    (2w)  CLEAN  -- a bare `jr $ra; nop` leaf
 * 4 of 4 clean, and the window references NO rodata or data symbol at all
 * (zero `%hi`/`%lo` in the whole slice), so no rodata sub-slot is attached.
 *
 * The head remainder that keeps the name `code_179d8` starts immediately
 * after this unit and is gp_rel-saturated (33 of its 39 functions), which is
 * why the cut is here: this window carries neither a jump table nor any
 * blocked-function stub-report debt.
 *
 * This unit calls out to func_80027E68 and func_80027E78, which are still
 * INCLUDE_ASM in that remainder -- their prototypes belong in THIS file, not
 * in a shared header (this unit has none and should not acquire one).
 *
 * `new_class_6d4e8` carries a name inherited from FirecatFG. Treat it as a
 * HYPOTHESIS, not evidence: see CLAUDE.md on the hand-rolled class framework
 * and resolve any method-table slot with tools/classtable.py rather than by
 * counting.
 *
 * Round 26 (echo): resolved via `tools/classtable.py 0x8006D4E8 --vs
 * 0x8006B58C`.  D_8006D4E8 (this class's own 29-slot method table, the
 * address the FirecatFG name is drawn from) overrides BasicClass's table
 * (D_8006B58C, 14 slots, header 0) at exactly three slots: +0x004
 * (func_800269F0), +0x008 (func_80027228) and +0x00C (func_80027274);
 * slots +0x010..+0x038 are inherited verbatim (same BasicClass__func_*
 * addresses in both tables) and the rest (+0x040 upward, including
 * func_800272C8) are new slots BasicClass's own table does not have at
 * all. +0x008 is BasicClass's own constructor slot in D_8006B58C
 * (BasicClass__BasicClass sits there) -- CONFIRMED by that lookup, not
 * assumed from the FirecatFG name.  So: `new_class_6d4e8` is a genuine
 * "allocate + construct" pair, and its constructor IS this same unit's
 * `func_80027228`, dispatched back through the class's own table (the
 * generic `new` doesn't call func_80027228 by name -- it fetches
 * func_80027E68()'s table and calls whatever sits at the ctor slot, which
 * happens to resolve to func_80027228 for this class).
 *
 * This unit's own local view of the class-table framework it participates
 * in: NOT the same shape as include/class_3ac78.h's GenericObject /
 * GenericMethodsHeader (that hierarchy's ctor sits at +0x04C) -- a
 * DIFFERENT, independent class hierarchy elsewhere in the project.  Kept
 * local per the project's multiple-independent-local-views convention.
 */
#include "common.h"

typedef struct Obj6D4E8Methods Obj6D4E8Methods;
struct Obj6D4E8Methods {
    s32 header;                    /* +0x000, not a pointer -- 0x13 for D_8006D4E8, 0 for BasicClass's own table */
    void *unk04;                   /* +0x004, func_800269F0 for this class -- unused by this unit's own functions */
    void (*ctor)(void *self);      /* +0x008, confirmed via classtable.py: BasicClass's OWN table has
                                     * BasicClass__BasicClass at this exact slot */
    u8 pad00C[0x05C - 0x00C];
    void (*slot5C)(void *self);    /* +0x05C, func_80027274's second vtable call */
    u8 pad060[0x074 - 0x060];
    void (*slot74)(void);          /* +0x074, func_80027274's first vtable call -- confirmed zero-argument:
                                     * retail's jalr for this slot carries a plain `nop` delay slot with no
                                     * register load anywhere above it, unlike the +0x05C call three
                                     * instructions later which explicitly sets $a0 = self. */
};

typedef struct {
    Obj6D4E8Methods *methods;      /* +0x000, set by the constructor to this class's own table
                                     * (func_80027E68()'s return value) -- the table-pointer-at-offset-0
                                     * convention CLAUDE.md documents for this project's class framework */
    u8 pad004[0x028 - 0x004];
    s16 unk28;                     /* +0x028, cleared by the constructor (a halfword store, `sh`) */
} Obj6D4E8;

/* A further-base class's ctor-dispatch table, shape confirmed only for the
 * one slot this unit's constructor uses -- same "ctor at +0x008" convention
 * as Obj6D4E8Methods above, mirroring class_3ac78.c's own local
 * BaseCtorTable_3ac78 for the identical "further-base ctor first" idiom. */
typedef struct {
    u8 pad0[0x008];
    void (*ctor)(void *self);      /* +0x008 */
} BaseCtorTable6D4E8;

extern void *func_80017B34(s32 size);              /* Psy-Q allocator, matched signature used project-wide */
extern BaseCtorTable6D4E8 *func_80026C9C(void *self); /* still INCLUDE_ASM in the code_179d8 remainder */
extern Obj6D4E8Methods *func_80027E68(void);        /* still INCLUDE_ASM in the code_179d8 remainder;
                                                       returns this class's own table, &D_8006D4E8 */
extern void func_80027E78(void);                    /* still INCLUDE_ASM in the code_179d8 remainder;
                                                       confirmed zero-argument the same way slot74 is above */

Obj6D4E8 *new_class_6d4e8(void)
{
    Obj6D4E8 *self;

    self = func_80017B34(0x2C);
    if (self != NULL) {
        func_80027E68()->ctor(self);
        return self;
    }
    return NULL;
}

void func_80027228(Obj6D4E8 *self)
{
    func_80026C9C(self)->ctor(self);
    self->methods = func_80027E68();
    self->unk28 = 0;
    func_80027E78();
}

void func_80027274(Obj6D4E8 *self)
{
    self->methods->slot74();
    self->methods->slot5C(self);
}

void func_800272C8(void)
{
}
