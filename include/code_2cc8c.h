#ifndef CODE_2CC8C_H
#define CODE_2CC8C_H

#include "common.h"

/*
 * The class whose method table is D_80086B60 (78 slots, base) with a
 * derived override table at D_80087AAC (73 slots) -- resolved with
 * tools/classtable.py D_80086B60 / D_80086B60 --vs D_80087AAC. No FirecatFG
 * name survives, so the struct is named `Obj86B60` after the base table's
 * address and fields are named by offset until real names are known. Per
 * this project's established multiple-independent-local-views convention
 * (see e.g. class_3bb8c.h's own header comment), this is THIS unit's own
 * view -- no other unit includes this header.
 *
 * Both tables agree on every slot this unit's 18 attempted functions
 * touch, so the base/derived split does not matter for this file; only
 * slots 0x090/0x094 (unused here) and a few 0x0D8+ slots differ between
 * them. See src/code_2cc8c.c's own header comment for the carve
 * provenance and the two functions (func_8003C48C, func_8003C63C) this
 * unit could not attempt (addiu_at-blocked dense switches).
 *
 * Only the slots/fields this unit's functions actually touch are given
 * concrete types; everything else stays opaque padding.
 */
typedef struct Obj86B60 Obj86B60;
typedef struct Obj86B60Methods Obj86B60Methods;
typedef struct Unk4CObj Unk4CObj;
typedef struct Unk48Obj Unk48Obj;
typedef struct Unk48ObjMethods Unk48ObjMethods;
typedef struct Unk78Obj Unk78Obj;
typedef struct Unk78ObjMethods Unk78ObjMethods;
typedef struct Unk74Obj Unk74Obj;
typedef struct Unk74ObjMethods Unk74ObjMethods;
typedef struct Unk64Elem Unk64Elem;
typedef struct Unk64ElemMethods Unk64ElemMethods;
typedef struct SrcDesc SrcDesc;

/*
 * FOR THE NEXT RUNNER (code_2cc8c_b, same 153-function block, same class
 * framework): a note on how much to trust `Unk48Obj`/`Unk4CObj`/`Unk78Obj`
 * below, since all three are minimal placeholder types and it matters
 * which part of that is "confirmed small" vs. "not yet looked at".
 *
 * - **The `padNNN[K]` byte ranges in all three are UNOBSERVED, not
 *   confirmed-unused.** Nothing in this unit's 16 matched + 4 stalled
 *   functions ever reads or writes those bytes -- that is the entire
 *   basis for calling them padding. It is NOT evidence those bytes are
 *   inert; per this project's class-framework shape (CLAUDE.md, "Writing
 *   a class method"), every one of these three is almost certainly a full
 *   object with its own real fields beyond offset 0, this unit's
 *   functions simply never touch them. Treat every `padNNN` here as "ends
 *   here only because our evidence ends here", and extend/narrow it the
 *   moment a function in `code_2cc8c_b` (or any other unit) reads inside
 *   one of these ranges.
 * - **Offset 0 being a method-table pointer IS confirmed for two of the
 *   three** (`Unk48Obj`, `Unk78Obj`) -- each is dereferenced through the
 *   `lw self,0; lw slot,N(methods); jalr` idiom at least once, which is
 *   real evidence, not a framework assumption.
 * - **`Unk4CObj` is different in kind: nothing in this unit ever loads
 *   `*(unk4C+0)` at all.** Every access goes through named fields at
 *   +0x008/+0x00C/+0x010/+0x024 directly; the struct is never
 *   dereferenced through a "methods" pointer anywhere in this unit's
 *   functions. Do not assume `Unk4CObj` starts with a method-table
 *   pointer the way the other two do just because this codebase is
 *   class-framework-heavy -- it may be a plain data record instead (a
 *   "target descriptor", by its usage). `pad000[0x008]` here is
 *   genuinely unknown, unlike the padding at the START of `Unk48Obj`/
 *   `Unk78Obj`, which at least is known to be `(methods, then K unread
 *   bytes)`.
 * - **None of the three has a known SIZE.** Each struct below is only as
 *   large as its highest observed field plus that field's own size --
 *   `Unk48Obj` could plausibly be anywhere from 0x084 bytes (just past
 *   `slot80`) to much larger; `Unk4CObj` is at least 0x028 bytes (past
 *   `unk24`) with no upper bound; `Unk78Obj` is at least 0x0BC bytes. If a
 *   future unit's allocator call reveals a literal byte count for any of
 *   these three classes, record it here.
 */

/*
 * self->unk4C's pointee ("target"). Established from six independent
 * functions all agreeing:
 *  - func_8003C7F4/func_8003C858/func_8003C8D0/func_8003C944/func_8003C9B0
 *    each use it ONLY as a null/non-null gate (never dereferenced), so it
 *    is at minimum a pointer.
 *  - func_8003CA1C dereferences +0x00C (s32, compared directly against
 *    self->unk58) and +0x024 (a pointer to a word-pointer array, indexed
 *    by self->unk58 and null-checked -- `target->unk24[self->unk58]`).
 *  - func_8003C63C (case a1==5, STALL -- see that function's report for
 *    why it was never attempted as a whole; this field use was read off
 *    the disassembly independently of attempting the function) reads
 *    +0x008 (s32, forwarded as an argument) and takes the ADDRESS of
 *    +0x010 (passed as a 3-byte colour-ish buffer to self->methods->slotE4,
 *    the same slot func_8003CC2C feeds a locally-built 3-byte buffer to).
 */
struct Unk4CObj {
    u8 pad000[0x008];
    s32 unk8;           /* +0x008, OBSERVED: func_8003C63C (not attempted) */
    s32 unkC;            /* +0x00C, OBSERVED: func_8003CA1C */
    u8 unk10[3];          /* +0x010, INFERRED 3-byte colour buffer read by
                              address only (func_8003C63C, not attempted) */
    u8 pad13[0x018 - 0x013];
    void **unk18;          /* +0x018, OBSERVED: func_8003D3B0, an array of
                               pointers indexed by an Obj86B60 index and
                               null-checked (never dereferenced) -- a
                               registration slot table, one entry per index
                               tracked by Obj86B60->unk58/unk50 */
    u8 pad01C[0x024 - 0x01C];
    void **unk24;         /* +0x024, OBSERVED: func_8003CA1C, word-pointer
                              array indexed by self->unk58 */
};

/* self->unk48's pointee ("child"). A DIFFERENT class from Obj86B60 -- its
 * own vtable slot +0x080 takes FOUR args (self, a1, a2, a3), whereas
 * Obj86B60's OWN slot +0x080 (func_8003C944) takes none beyond self. Only
 * func_8003C7B4 touches it, so only that one slot is modelled. */
struct Unk48ObjMethods {
    u8 pad000[0x080];
    void (*slot80)(Unk48Obj *self, s32 a1, s32 a2, s32 a3); /* +0x080 */
};
struct Unk48Obj {
    Unk48ObjMethods *methods; /* +0x000 */
};

/* self->unk78's pointee. Only func_8003CC2C touches it, calling one slot
 * with a literal 1 and the same 3-byte colour buffer func_8003CC2C builds
 * for its own self->methods->slotE4 call just above. */
struct Unk78ObjMethods {
    u8 pad000[0x0B8];
    void (*slotB8)(Unk78Obj *self, s32 a1, u8 *buf); /* +0x0B8 */
};
struct Unk78Obj {
    Unk78ObjMethods *methods; /* +0x000 */
};

/* self->unk74's pointee ("sub-resource handle"). Only func_8003CDE0 touches
 * it, loaded via `func_8003B39C(path)` (already matched, `class_39e08.c`,
 * where it returns the unit's own local view `SubObjG *` -- this unit keeps
 * its own local view of the same table per the project's established
 * multiple-independent-local-views convention). slot4's return value is
 * discarded at its one call site here, so it is typed `void *` rather than
 * copying `class_39e08.h`'s `SubObjG *` return type -- a discarded return is
 * never evidence of the callee's real return type (see
 * DECOMPILATION_LEARNINGS), and this unit has no use for the more specific
 * type. */
struct Unk74ObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk74Obj *self);  /* +0x004 */
    u8 pad008[0x05C - 0x008];
    void (*slot5C)(Unk74Obj *self);  /* +0x05C */
    u8 pad060[0x078 - 0x060];
    void (*slot78)(Unk74Obj *self);  /* +0x078 */
};
struct Unk74Obj {
    Unk74ObjMethods *methods;        /* +0x000 */
};

extern Unk74Obj *func_8003B39C(const char *path); /* already matched in
                                                       class_39e08.c; local
                                                       view retyped to this
                                                       unit's own Unk74Obj */

extern void *func_80017B34(s32 size);   /* allocator, confirmed across many
                                            units */
extern void func_80017CFC(void *ptr);   /* matching free/release, confirmed
                                            void-returning in code_171e0.h
                                            and Entity.h */
extern void func_800183DC(void *a0, void *a1); /* not yet seen elsewhere in
                                                    this project; typed from
                                                    func_8003D6D4's own call
                                                    site only */

/*
 * func_8003D5CC's 2nd parameter -- an unrelated "source list" descriptor,
 * NOT an Obj86B60 or any class in this unit's own hierarchy (no method
 * table dereference anywhere in that function). Only the two fields it
 * touches are modelled.
 */
struct SrcDesc {
    u8 pad000[0x004];
    s32 unk4;    /* +0x004, becomes self->unk60[idx] */
    u8 pad008[0x018 - 0x008];
    char **unk18; /* +0x018, NULL-terminated array of C strings -- each
                      element is passed to func_80013348 (strlen, already
                      typed `s32 func_80013348(char *s)` in
                      code_171e0.h) and to func_800408CC */
};

extern s32 func_80013348(char *s); /* already matched elsewhere
                                        (code_171e0.c) as a strlen-shaped
                                        helper; local view here */
extern Unk64Elem *func_800408CC(void *ctx, s32 len, char *name); /* not
                                        yet seen elsewhere; typed from
                                        func_8003D5CC's own call site --
                                        its return value is stored directly
                                        into the same self->unk64[idx]
                                        array func_8003D980/func_8003D2CC/
                                        func_8003DA10/func_8003DE9C walk as
                                        Unk64Elem * */

/*
 * self->unk64[idx]'s pointee, as walked by func_8003D980 -- a DIFFERENT
 * reading of the same field func_8003D6D4/func_8003DDC8/func_8003DE30 use
 * as an opaque resource handle. func_8003D980 reinterprets that handle as
 * `Unk64Elem **` (an array of `self->unk5C[idx]` object pointers) and
 * dispatches through each element's own +0x0B8 slot. Both readings are
 * kept -- the field itself stays `void **` in `Obj86B60` (the generic,
 * more common usage) and this function alone casts locally, per this
 * project's "empty-bodied vtable occupant is not evidence the SLOT takes
 * no arguments" family of narrow-evidence cautions applied to a field
 * instead of a slot.
 */
struct Unk64ElemMethods {
    u8 pad000[0x060];
    void (*slot60)(Unk64Elem *self, s32 a1);   /* +0x060, OBSERVED:
                                                    func_8003DCAC */
    u8 pad064[0x0B8 - 0x064];
    void (*slotB8)(Unk64Elem *self, void *a1); /* +0x0B8 */
};
struct Unk64Elem {
    Unk64ElemMethods *methods; /* +0x000 */
};

/*
 * self->methods. Only the slots this unit's functions actually CALL
 * THROUGH (as opposed to slots that simply ARE these functions, entered
 * from elsewhere) are given here -- there is no need to model a slot this
 * unit never itself invokes.
 *
 * slotB0/slotC4 are read as DATA (a raw function-pointer VALUE stashed into
 * self->unk88/self->unk8C by func_8003CAF8/func_8003CB30), never called
 * directly through the vtable in this unit -- their type is inferred from
 * how self->unk88/unk8C are later CALLED (func_8003CBC0/func_8003CCDC).
 *
 * slotAC/slotC0 ARE func_8003CBC0/func_8003CCDC respectively (OBSERVED from
 * classtable.py). func_8003C51C calls them with `self` only -- register
 * `$a1` is genuinely live-but-unconsumed at those two call sites (leftover
 * from an earlier `self->methods->slot60(self, 6)` call a few instructions
 * before, on the branch that reaches them), not a real argument; neither
 * callee's own body reads it. See func_8003C51C's own report.
 */
struct Obj86B60Methods {
    u8 pad000[0x060];
    void (*slot60)(Obj86B60 *self, s32 reason);  /* +0x060, external
                                                      (func_8004D90C) */
    u8 pad064[0x070 - 0x064];
    void (*slot70)(Obj86B60 *self, s32 a1);       /* +0x070, IS
                                                      func_8003C7B4 */
    u8 pad074[0x090 - 0x074];
    void (*slot90)(Obj86B60 *self);               /* +0x090, external
                                                      (func_8004D9D4);
                                                      OBSERVED: func_8003C63C
                                                      (STALL, not attempted --
                                                      read off the
                                                      disassembly only) */
    void (*slot94)(Obj86B60 *self);                /* +0x094, external
                                                       (func_8004DABC);
                                                       OBSERVED:
                                                       func_8003CA1C and
                                                       func_8003C63C (STALL) */
    u8 pad098[0x0AC - 0x098];
    s32 (*slotAC)(Obj86B60 *self);                 /* +0x0AC, OBSERVED:
                                                       IS func_8003CBC0 */
    s32 (*slotB0)(Obj86B60 *self);                 /* +0x0B0, IS
                                                       func_8003CC2C; read as
                                                       DATA by func_8003CAF8 */
    u8 pad0B4[0x0C0 - 0xB4];
    s32 (*slotC0)(Obj86B60 *self);                 /* +0x0C0, IS
                                                       func_8003CCDC */
    s32 (*slotC4)(Obj86B60 *self);                  /* +0x0C4, external
                                                       (func_8003CD48); read
                                                       as DATA by
                                                       func_8003CB30 */
    u8 pad0C8[0x0E4 - 0xC8];
    void (*slotE4)(Obj86B60 *self, u8 *buf);        /* +0x0E4, external
                                                       (func_8003D9D4);
                                                       OBSERVED:
                                                       func_8003CC2C and
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    void (*slotE8)(Obj86B60 *self);                 /* +0x0E8, external
                                                       (func_8003D3B0);
                                                       OBSERVED: func_8003C9B0 */
    void (*slotEC)(Obj86B60 *self);                 /* +0x0EC, external
                                                       (func_8003D444);
                                                       OBSERVED: func_8003C944 */
    void (*slotF0)(Obj86B60 *self, s32 a1, s32 a2); /* +0x0F0, external
                                                       (func_8004DABC);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    u8 pad0F4[0x100 - 0xF4];
    void (*slot100)(Obj86B60 *self, s32 a1, s32 a2); /* +0x100, external;
                                                       OBSERVED:
                                                       func_8003DA10 */
    void (*slot104)(Obj86B60 *self, void *a1);      /* +0x104, external;
                                                       OBSERVED:
                                                       func_8003D2CC */
    void (*slot108)(Obj86B60 *self);                /* +0x108, external
                                                       (func_8003DA10);
                                                       OBSERVED: func_8003CA1C */
    s32 (*slot10C)(Obj86B60 *self);                 /* +0x10C, external
                                                       (func_8003DAD4);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    s32 (*slot110)(Obj86B60 *self);                 /* +0x110, external
                                                       (func_8003DCAC);
                                                       OBSERVED:
                                                       func_8003C63C (STALL,
                                                       not attempted) */
    void (*slot114)(Obj86B60 *self);                /* +0x114, external
                                                       (func_8003DDC8);
                                                       OBSERVED: func_8003C9B0 */
    s32 (*slot118)(Obj86B60 *self);                 /* +0x118. CORRECTED
                                                       (round 12, runner
                                                       alpha): the occupant
                                                       is func_8003DE30, NOT
                                                       func_8003DFA0 as this
                                                       comment previously
                                                       said -- verified by
                                                       reading the raw table
                                                       bytes at
                                                       D_80086B60+0x118 in
                                                       disk/SLPS_015.56
                                                       directly (also
                                                       cross-checked with
                                                       tools/classtable.py
                                                       D_80086B60). The old
                                                       attribution came from
                                                       func_8003C944.md's
                                                       "Struct knowledge
                                                       established" section,
                                                       which was itself
                                                       wrong about WHICH
                                                       function occupies this
                                                       slot even though the
                                                       byte OFFSET it matched
                                                       against (0x118) was
                                                       correct -- a call
                                                       site discarding/not
                                                       discarding a return
                                                       value says nothing
                                                       about slot identity.
                                                       OBSERVED: func_8003C944 */
    void (*slot11C)(Obj86B60 *self, s32 a1, s32 a2); /* +0x11C, external;
                                                       OBSERVED:
                                                       func_8003DDC8,
                                                       func_8003DE30 (both
                                                       call it with a
                                                       computed index value
                                                       and a literal 1) */
    s32 (*slot120)(Obj86B60 *self);                 /* +0x120, external:
                                                       IS func_8003DFA0
                                                       (verified the same
                                                       way as slot118 above;
                                                       `func_8003DFA0` itself
                                                       returns
                                                       `self->unk60[self->
                                                       unk58]`, s32) */
};

/*
 * self->unkC's pointee, observed only by func_8003E538 -- a shared
 * base-class method also reachable through UNRELATED classes' own vtables
 * at this same slot offset (class_39e08.h documents func_8003E538/78
 * occupying Obj865C8Methods/Class86668Methods +0x064/+0x068). Dispatch
 * shape: `self->unkC->target->methods->slot48(target)` -- one extra level
 * of indirection past the usual `self->fieldN->methods->slotM(self->fieldN)`
 * idiom. Only the one field/slot func_8003E538 touches is modelled.
 */
typedef struct Obj86B60UnkC Obj86B60UnkC;
typedef struct Obj86B60UnkCTarget Obj86B60UnkCTarget;
typedef struct Obj86B60UnkCTargetMethods Obj86B60UnkCTargetMethods;
struct Obj86B60UnkCTargetMethods {
    u8 pad000[0x048];
    void (*slot48)(Obj86B60UnkCTarget *self); /* +0x048, OBSERVED: func_8003E538 */
};
struct Obj86B60UnkCTarget {
    Obj86B60UnkCTargetMethods *methods; /* +0x000 */
};
struct Obj86B60UnkC {
    Obj86B60UnkCTarget *target; /* +0x000, OBSERVED: func_8003E538 */
};

/*
 * The object itself. Every field below is OBSERVED (not inferred) from at
 * least one of this unit's 18 attempted functions -- see each function's
 * own match report for the specific derivation. Gaps are left as opaque
 * padding; nothing here claims knowledge of bytes no function touched.
 */
struct Obj86B60 {
    Obj86B60Methods *methods;   /* +0x000 */
    u8 pad004[0x00C - 0x004];
    Obj86B60UnkC *unkC;          /* +0x00C, func_8003E538 (see Obj86B60UnkC's
                                    own comment): `self->unkC->target->methods
                                    ->slot48(target)`. Also zeroed by
                                    func_8003E874 (a ctor-shaped function that
                                    also zeroes unk10/unk30 below). NOTE: this
                                    slot is reached through a SHARED base-class
                                    method -- class_39e08.h's own view of an
                                    unrelated class documents the same
                                    func_8003E538 occupying its own vtable at
                                    the identical offset (+0x064), so this
                                    field is very likely part of a common
                                    base-object layout every subclass shares
                                    at this offset, not something Obj86B60
                                    itself introduces -- kept here anyway,
                                    per this header's flat single-struct
                                    style (no explicit base/derived split). */
    s32 unk10;                  /* +0x010, func_8003E874 only: zeroed by the
                                    same ctor-shaped function as unkC/unk30;
                                    real meaning unknown, generic word */
    s32 unk14;                  /* +0x014, func_8003DA10: forwarded as
                                    slot100's 2nd arg, otherwise untouched
                                    by this unit -- generic word, not
                                    dereferenced here */
    u8 pad018[0x01C - 0x018];
    s32 unk1C;                  /* +0x01C, func_8003CC2C (a running count/
                                    frame value multiplied against unk84);
                                    func_8003C63C (STALL) zeroes it on
                                    several message codes */
    u8 pad020[0x020 - 0x020];
    s32 unk20;                  /* +0x020, func_8003C63C (STALL) sets it
                                    to a literal 5 */
    u8 pad024[0x030 - 0x024];
    s32 unk30;                  /* +0x030, func_8003E874 only: zeroed by the
                                    same ctor-shaped function as unkC/unk10;
                                    real meaning unknown, generic word */
    u8 pad034[0x038 - 0x034];
    s32 unk38;                  /* +0x038, func_8003C63C (STALL) sets it
                                    to 1 */
    s32 unk3C;                  /* +0x03C, a mode/state value: func_8003C858
                                    compares ==1, func_8003C8D0 !=1,
                                    func_8003C944/func_8003C9B0 ==1/==2,
                                    func_8003C48C (STALL) gates on !=0,
                                    func_8003C63C (STALL) sets 0/1 */
    s32 unk40;                  /* +0x040, func_8003C794 (setter: raw value
                                    if negative, value*20 if >= 0);
                                    func_8003C51C compared against unk1C */
    u8 pad044[0x048 - 0x044];
    Unk48Obj *unk48;            /* +0x048, func_8003C7B4 only */
    Unk4CObj *unk4C;            /* +0x04C, see Unk4CObj's own comment */
    s32 unk50;                  /* +0x050, func_8003D3B0: capacity/wrap
                                    bound for the unk58 index into
                                    unk4C->unk18[] (also func_8003D2CC's
                                    loop count) */
    Unk64Elem **unk54;          /* +0x054, func_8003D2CC: walked with an
                                    incrementing pointer, dereferenced
                                    directly for each element */
    s32 unk58;                  /* +0x058, func_8003CA1C: index into
                                    unk4C->unk24[] and compared against
                                    unk4C->unkC */
    s32 *unk5C;                  /* +0x05C, array indexed by unk58: a
                                     per-slot capacity/bound.
                                     func_8003D6D4 passes unk5C[unk58] as
                                     func_800183DC's 2nd arg (raw register,
                                     type doesn't affect those bytes);
                                     func_8003DDC8/func_8003DE30 use it as
                                     an explicit upper bound compared
                                     against unk60[unk58], which is what
                                     settles it as a count, not a pointer */
    s32 *unk60;                   /* +0x060, array indexed by unk58: a
                                     per-slot running count, incremented
                                     (wrapping to 0 past unk5C[unk58]) by
                                     func_8003DDC8 and decremented
                                     (wrapping to unk5C[unk58]-1 below 0) by
                                     func_8003DE30 -- a ring-buffer index */
    void **unk64;                /* +0x064, func_8003D6D4: array indexed by
                                     unk58, giving func_800183DC's 1st arg
                                     and func_80017CFC's arg */
    u8 pad068[0x070 - 0x068];
    const char *unk70;          /* +0x070, func_8003CDE0: truthy gate and a
                                    cache of the path last passed to
                                    func_8003B39C */
    Unk74Obj *unk74;            /* +0x074, func_8003CDE0 only */
    Unk78Obj *unk78;             /* +0x078, func_8003CC2C only */
    u8 pad07C[0x084 - 0x07C];
    s32 unk84;                  /* +0x084, func_8003CC2C: multiplied
                                    against unk1C */
    s32 (*unk88)(Obj86B60 *self); /* +0x088, a callback: set (to NULL or
                                    self->methods->slotB0) by func_8003CAF8,
                                    invoked by func_8003CBC0 */
    s32 (*unk8C)(Obj86B60 *self); /* +0x08C, same idiom via slotC4/
                                    func_8003CB30/func_8003CCDC */
    u8 unk90[3];                 /* +0x090, func_8003CB68 (setter, from
                                    a1[0..2]); func_8003CC2C reads it as a
                                    colour base */
    u8 unk93[3];                 /* +0x093, func_8003CB68 (setter, from
                                    a2[0..2]) */
    u8 unk96[3];                 /* +0x096, func_8003CB68 (setter, from
                                    a3[0..2]) */
    u8 unk99[0x09C - 0x099];
    void (*unk9C)(void *ctx);   /* +0x09C, a callback: set by func_8003CAEC,
                                    invoked (with unkA0 as its argument) by
                                    func_8003CA94 */
    void *unkA0;                 /* +0x0A0, set by func_8003CAEC, passed to
                                    unk9C by func_8003CA94 */
};

/*
 * Shared "IntermediateBase" utility class, reached through func_8003E5C8().
 * UPDATED (round 12, runner alpha): func_8003E5C8 has now been carved into
 * THIS unit's own code_2cc8c_c.c and is defined there -- this comment
 * previously said "not this unit's function to write" because it was
 * written before that carve. Same idiom already established in
 * src/code_2c054.c (TaskUtilMethods) and src/class_39e08.c
 * (IntermediateBaseMethods): each unit that reaches it keeps its own local
 * view, self typed `void *` since it is shared across unrelated classes.
 * Only the two slots this unit's func_8003C51C (and func_8003C63C, STALL)
 * actually reach are modelled.
 */
typedef struct IntermediateBaseMethods IntermediateBaseMethods;
struct IntermediateBaseMethods {
    u8 pad000[0x05C];
    void (*slot5C)(void *self, s32 a1, s32 a2);  /* +0x05C */
    void (*slot60)(void *self, s32 a1);           /* +0x060 */
};

extern IntermediateBaseMethods *func_8003E5C8(void); /* returns &D_8006E878,
                                                          same static table
                                                          as code_2c054.h's
                                                          and class_39e08.h's
                                                          own views */
extern IntermediateBaseMethods D_8006E878; /* the table itself, so
                                                func_8003E5C8's own
                                                definition (code_2cc8c_c.c)
                                                can return &D_8006E878 */

/*
 * This unit's own local view of the shared BasicClass ancestor table
 * (returned by func_80018390, a no-argument getter -- same "ctor at
 * +0x008, dtor at +0x00C, self typed void* universally" idiom already
 * established independently in include/class_16334.h, include/code_171e0.h
 * and include/code_d294.h. Declared again here, under a unit-local name,
 * per this project's policy of NOT unifying independent local views of the
 * same table into one shared header. Only the one slot func_8003E874
 * dispatches through is modelled.
 */
typedef struct BasicClassMethodsCC8C BasicClassMethodsCC8C;
struct BasicClassMethodsCC8C {
    u8 pad000[0x018];
    void (*slot18)(void *self); /* +0x018, func_8003E874's forward target */
};

extern BasicClassMethodsCC8C *func_80018390(void);

#endif
