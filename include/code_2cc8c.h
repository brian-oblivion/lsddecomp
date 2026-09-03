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
typedef struct Unk68Obj Unk68Obj;
typedef struct Unk68ObjMethods Unk68ObjMethods;
typedef struct SrcDesc SrcDesc;
typedef struct HeaderObj HeaderObj;
typedef struct EventArg EventArg;
typedef struct Unk14Obj Unk14Obj;
typedef struct Unk14ObjMethods Unk14ObjMethods;
typedef struct Unk18Obj Unk18Obj;
typedef struct Unk18ObjMethods Unk18ObjMethods;
typedef struct SubHandleObj SubHandleObj;
typedef struct SubHandleObjMethods SubHandleObjMethods;
typedef struct Unk18AcObj Unk18AcObj;
typedef struct Unk18AcObjMethods Unk18AcObjMethods;
typedef struct GenericObj GenericObj;
typedef struct GenericObjMethods GenericObjMethods;
typedef struct Obj86B60InitArgs Obj86B60InitArgs;
typedef struct Unk10Obj Unk10Obj;
typedef struct Unk10ObjMethods Unk10ObjMethods;
typedef struct Unk4ArgObj Unk4ArgObj;
typedef struct Unk4ArgObjMethods Unk4ArgObjMethods;

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
 * A generic "event" argument, round 13: `arg1->target->header` is read by
 * func_8003E030 to pick which of self->methods->slot54/58/5C to forward to.
 * Same two-type shape as class_39e08.h's own independent `EventArg`/
 * `HeaderObj` local view (a `target` pointer to an object whose first word
 * is a low-nibble-coded header/kind value) -- this unit keeps its own
 * separate local view per the project's established convention. Only the
 * one field/offset func_8003E030 touches is modelled.
 */
struct HeaderObj {
    s32 header; /* +0x000 */
};
struct EventArg {
    HeaderObj *target; /* +0x000 */
};

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
    const char *unk0;   /* +0x000, OBSERVED: func_8003CE98 (round 12) -- a
                            path: passed to func_8003B39C(unk0) when
                            non-NULL to build unk4, mirroring
                            Obj86B60->unk70's own path-cache idiom. ALSO
                            OBSERVED (truthy-only) by func_8003D050, which
                            gates a call through unk4 on this being
                            non-NULL -- corrects this struct's earlier
                            header note that nothing ever loads *(unk4C+0);
                            that was true only of the 16 functions attempted
                            through round 11. */
    Unk74Obj *unk4;      /* +0x004, OBSERVED: func_8003CE98 (constructed via
                            func_8003B39C(unk0) + slot78/slot5C when unk0 is
                            set, else read as an existing handle and written
                            back unchanged; same Unk74Obj slot4 interface
                            Obj86B60->unk74 uses) and func_8003D050 (slot4
                            called on it, gated by unk0's truthiness) */
    s32 unk8;           /* +0x008, OBSERVED: func_8003C63C (not attempted) */
    s32 unkC;            /* +0x00C, OBSERVED: func_8003CA1C */
    u8 unk10[3];          /* +0x010, INFERRED 3-byte colour buffer read by
                              address only (func_8003C63C, not attempted);
                              CONFIRMED as a 3-byte buffer read (not just
                              address-taken) by func_8003DCAC/func_8003DE9C,
                              both already matched, passing it directly to
                              an Unk64Elem slotB8 call */
    u8 pad13[0x018 - 0x013];
    void **unk18;          /* +0x018, OBSERVED: func_8003D3B0, an array of
                               pointers indexed by an Obj86B60 index and
                               null-checked (never dereferenced) -- a
                               registration slot table, one entry per index
                               tracked by Obj86B60->unk58/unk50 */
    char **unk1C;           /* +0x01C, OBSERVED: func_8003CE98 (round 12) --
                                a NULL-terminated array of C strings, DISTINCT
                                from unk18 at +0x018 (adjacent field, same
                                shape, different slot). Walked with
                                `func_80013348` (strlen) and passed to
                                `func_800408CC` to build each entry of
                                Obj86B60->unk54[i]/unk64[i]. */
    u8 *unk20;               /* +0x020, OBSERVED: func_8003D194 (round 12) --
                                a pointer walked forward 8 bytes per loop
                                iteration (an external array of 8-byte
                                records this unit never reads through
                                directly, only forwards as func_8003D194's
                                3rd arg to an Unk64Elem slot4C call) */
    void **unk24;         /* +0x024, OBSERVED: func_8003CA1C, word-pointer
                              array indexed by self->unk58 */
};

/*
 * The pointee of Unk4CObj->unk24[idx] (round 12, from func_8003DAD4 and
 * func_8003D73C, cross-checked against already-matched func_8003DCAC's own
 * `((s32 *)self->unk4C->unk24[idx])[1]` read at the same +0x004 offset).
 * Only the three offsets these functions actually touch are modelled;
 * func_8003DCAC/func_8003DE9C's own `(u8 *)...unk24[idx] + 8` buffer usage
 * is left as a raw cast in those (already-matched) functions rather than
 * retrofitted onto this type, per this project's convention of not
 * editing matched functions to adopt a later, more specific type.
 */
typedef struct Unk24Elem Unk24Elem;
struct Unk24Elem {
    u8 pad000[0x004];
    s32 unk4;    /* +0x004, a per-slot counter/index: SET here by
                    func_8003DAD4, READ back as `newVal` by the
                    already-matched func_8003DCAC */
    u8 pad008[0x010 - 0x008];
    s32 unk10;   /* +0x010 */
    s32 unk14;   /* +0x014, combined with unk10 and a per-slot counter into
                    a 2-word stack buffer (`{unk10, unk14 - counter*10}`)
                    passed by address to an Unk64Elem slotBC call, then
                    incremented by 10 per loop iteration -- see
                    func_8003DAD4/func_8003D73C */
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
    u8 pad000[0x004];
    void (*slot4)(Unk64Elem *self);            /* +0x004, OBSERVED:
                                                    func_8003D050 (round 12) */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(Unk64Elem *self, void *a1, void *buf); /* +0x04C,
                                                    OBSERVED: func_8003D194
                                                    and func_8003D73C
                                                    (round 12) -- both pass a
                                                    raw buffer pointer as the
                                                    3rd arg (an 8-byte-stride
                                                    external record in
                                                    func_8003D194, the
                                                    address of a 2-word stack
                                                    pair in func_8003D73C),
                                                    so `buf` stays untyped */
    void (*slot50)(Unk64Elem *self);            /* +0x050, OBSERVED:
                                                    func_8003D73C (round 12) */
    u8 pad054[0x060 - 0x054];
    void (*slot60)(Unk64Elem *self, s32 a1);   /* +0x060, OBSERVED:
                                                    func_8003DCAC */
    u8 pad064[0x0B8 - 0x064];
    void (*slotB8)(Unk64Elem *self, void *a1); /* +0x0B8 */
    void (*slotBC)(Unk64Elem *self, void *a1); /* +0x0BC, OBSERVED:
                                                    func_8003DAD4 (round 12),
                                                    address of a 2-word
                                                    stack pair */
};
struct Unk64Elem {
    Unk64ElemMethods *methods; /* +0x000 */
};

/*
 * self->unk68's pointee (round 12, from func_8003D050/func_8003DAD4/
 * func_8003D73C). Built by `func_800404D0(&D_8008A8E8, &D_8008A8F0, 0)` in
 * func_8003CE98 -- func_800404D0 itself lives in the still-uncarved
 * code_2cc8c_d segment (not this unit's function to attempt), so it is
 * declared here only as an external returning this unit's own local view
 * of the type it constructs. D_8008A8E8/D_8008A8F0 are likewise only ever
 * address-taken here (never dereferenced by this unit), so they stay
 * minimally typed.
 */
struct Unk68ObjMethods {
    u8 pad000[0x004];
    void (*slot4)(Unk68Obj *self);              /* +0x004, OBSERVED:
                                                     func_8003D050 */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(Unk68Obj *self, s32 a1);      /* +0x04C, OBSERVED:
                                                     func_8003D73C */
    void (*slot50)(Unk68Obj *self);               /* +0x050, OBSERVED:
                                                     func_8003DAD4,
                                                     func_8003D73C */
    u8 pad054[0x0C0 - 0x054];
    void (*slotC0)(Unk68Obj *self, void *buf);     /* +0x0C0, OBSERVED:
                                                     func_8003D73C, address
                                                     of a 2-word stack pair
                                                     `{0x28, count*12}` */
};
struct Unk68Obj {
    Unk68ObjMethods *methods; /* +0x000 */
};

extern Unk68Obj *func_800404D0(void *a0, void *a1, s32 a2); /* external
                                    (code_2cc8c_d, not this unit); local
                                    view -- return type inferred from
                                    self->unk68's own dispatch pattern */
extern s32 D_8008A8E8[2];   /* address-taken only by this unit */
extern char D_8008A8F0[4];  /* address-taken only by this unit */

/*
 * The pointee of `Unk18Obj->unkB0` (round 13, func_8003E628), returned by
 * `func_8003FDB0` -- a function already known elsewhere in this project
 * (`include/Entity.h`'s own `Unk100Obj`/`func_8003FDB0`), kept here under a
 * unit-local name per this project's established "independent local views"
 * convention. Only the one slot this unit's `func_8003E628` dispatches
 * through is modelled.
 */
struct SubHandleObjMethods {
    u8 pad000[0x04C];
    void (*slot4C)(SubHandleObj *self, void *arg1, void *arg2); /* +0x04C,
                                    OBSERVED: func_8003E628 */
};
struct SubHandleObj {
    SubHandleObjMethods *methods; /* +0x000 */
};

extern SubHandleObj *func_8003FDB0(void *name, s32 arg1, s32 arg2); /* local
                                    view of include/Entity.h's own
                                    `func_8003FDB0` */
extern Unk18AcObj *func_8001CA94(void); /* local view of include/code_d294.h's
                                    own `New_Class6B5CC` allocator, returning
                                    `Class6B5CCObj *` there -- this unit's
                                    own view retyped (round 13) once
                                    func_8003E6CC dereferenced it, see
                                    Unk18AcObj's own comment */
extern u8 D_8008A90C[]; /* address-taken only by this unit, passed as
                            func_8003FDB0's "name" argument */
extern u8 D_8008A904[]; /* address-taken only by this unit, passed as
                            SubHandleObjMethods::slot4C's 3rd argument */
extern u8 D_8008A8F4[]; /* round 14, code_2cc8c_d (asm/data/7B008.sdata.s,
                            not decompiled): address-taken only, passed as
                            func_8003EACC's own default value for slot80's
                            2nd argument when its own arg5 is NULL. */

/*
 * `Unk18Obj->unkAC`'s pointee (round 13, func_8003E6CC) -- the return of
 * `func_8001CA94`, first stored opaquely by `func_8003E628` and here
 * dereferenced and released through the inherited BasicClass "release"
 * slot. Only that one slot is modelled.
 */
struct Unk18AcObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk18AcObj *self); /* +0x004, inherited BasicClass
                                          "release"; OBSERVED: func_8003E6CC */
};
struct Unk18AcObj {
    Unk18AcObjMethods *methods; /* +0x000 */
};

/*
 * A generic class-instance view (round 13, func_8003E770): every class's
 * vtable in this game begins with a "header" word (`tools/classtable.py`'s
 * own label for it, "not a pointer; varies per class -- id/flags"), and
 * `func_8003E770` discriminates its 2nd parameter's DYNAMIC CLASS by
 * reading `arg1->methods->header & 0xF` -- i.e. runtime type identification
 * through the vtable header nibble, not a struct field of `arg1` itself.
 * Only that one field, plus the one instance field (`unk14`) this function
 * also reads, are modelled; `arg1`'s real class is unknown and irrelevant
 * to this function's own behaviour.
 */
struct GenericObjMethods {
    s32 header; /* +0x000 */
};
struct GenericObj {
    GenericObjMethods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    void *unkC;                 /* +0x00C, OBSERVED: func_8003EEC0 (round 14),
                                    truthy-tested only */
    u8 pad010[0x014 - 0x010];
    s32 unk14;                  /* +0x014, OBSERVED: func_8003E770 */
};

/*
 * self->unk18's pointee, round 13 (func_8003E10C). Constructed by a
 * New_X allocator this unit itself carves (func_8003E5D8, 0xBC bytes) via
 * `func_8003F24C()->ctor(self)` -- func_8003F24C lives in a still-uncarved
 * remainder of this segment (not this unit's function to write), so it is
 * declared here only as an external returning this unit's own local view
 * of the class table it constructs. Only the one slot func_8003E10C
 * dispatches through is modelled.
 */
/* Round 14 (code_2cc8c_d): a plain 3-word vector, copied wholesale from a
 * caller-supplied source into Unk18Obj::unk14 (func_8003EBC4). Local view,
 * same shape as code_d294.h's own Vec3_d294 but this project's convention
 * is not to unify independent per-unit views of an unnamed shape. Declared
 * here (ahead of Unk18ObjMethods) since that struct's own slot78 needs it.
 */
typedef struct Vec3_2cc8c {
    s32 x;
    s32 y;
    s32 z;
} Vec3_2cc8c;

struct Unk18ObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk18Obj *self);            /* +0x004, inherited
                                                    BasicClass "release"
                                                    (finalize then free);
                                                    OBSERVED: func_8003E280
                                                    (round 13) */
    void (*ctor)(Unk18Obj *self);              /* +0x008, called by
                                                    func_8003E5D8 with only
                                                    `self` set up */
    u8 pad00C[0x010 - 0x00C];
    void (*slot10)(Unk18Obj *self, void *a1);  /* +0x010, inherited
                                                    BasicClass addChild;
                                                    OBSERVED: func_8003E10C */
    void (*slot14)(Unk18Obj *self, void *a1);  /* +0x014, inherited
                                                    BasicClass removeChild;
                                                    OBSERVED: func_8003E280
                                                    (round 13) */
    u8 pad018[0x040 - 0x018];
    void (*slot40)(Unk18Obj *self);            /* +0x040, OBSERVED:
                                                    func_8003E628 (round 13)
                                                    -- a DIFFERENT table from
                                                    Obj86B60Methods's own
                                                    slot40 (`D_8006E8E4`'s
                                                    own occupant here is
                                                    `func_8003E968`, not
                                                    `func_8003E100`) */
    u8 pad044[0x074 - 0x044];
    void (*slot74)(Unk18Obj *self);            /* +0x074, OBSERVED:
                                                    func_8003E6CC (round 13) */
    /* +0x078/+0x07C/+0x080, round 14 (func_8003EACC's own call site,
       guarded by `self->unk10 == NULL`): dispatched as `(self, a2)`,
       `(self, a3)`, `(self, a1_or_default)` respectively. Occupants (this
       unit, per tools/classtable.py D_8006E8E4): func_8003EBC4 (+0x078,
       still queued), func_8003EBF8 (+0x07C, still queued), func_8003EC2C
       (+0x080, the documented gp_rel blocker -- NOT decompiled here). */
    void (*slot78)(Unk18Obj *self, Vec3_2cc8c *a1); /* +0x078, retyped round 14 once func_8003EBC4 (its own occupant) confirmed the shape */
    void (*slot7C)(Unk18Obj *self, Vec3_2cc8c *a1); /* +0x07C, retyped round 14 once func_8003EBF8 (its own occupant) confirmed the shape */
    void (*slot80)(Unk18Obj *self, void *a1); /* +0x080 */
    u8 pad084[0x090 - 0x084];
    void (*slot90)(Unk18Obj *self);            /* +0x090, OBSERVED:
                                                    func_8003E6CC (round 13) */
    /* +0x094/+0x098, round 14 (code_2cc8c_d): func_8003E8B8's own call
       site -- dispatched as `(self, arg1, arg2)` when a GenericObj arg1's
       header tag is 5 (slot94) or 1 (slot98). Occupants (this unit, still
       queued as of this comment): func_8003EE40 (+0x094), func_8003EE88
       (+0x098). */
    void (*slot94)(Unk18Obj *self, GenericObj *arg1, s32 arg2); /* +0x094 */
    void (*slot98)(Unk18Obj *self, GenericObj *arg1, s32 arg2); /* +0x098 */
    void (*slot9C)(Unk18Obj *self); /* +0x09C, occupant func_8003EEC0 (round 14); dispatched by func_8003EE40 */
    /* +0x0A0, occupant func_80012064 (asm/psyq_2258.s, PsyQ library, not
       decompiled) -- dispatched by func_8003EEC0 (round 14) at three call
       sites with different arities (self alone; self+unkAC; self+another
       Unk18Obj*), so kept as an untyped function pointer and cast per
       call site rather than picking one fixed signature. */
    void *slotA0;
    void (*slotA4)(Unk18Obj *self); /* +0x0A4, occupant func_8003F04C (round 14, still queued as of this comment); dispatched by func_8003EE88 */
    void (*slotA8)(Unk18Obj *self, s32 a1);    /* +0x0A8, OBSERVED:
                                                    func_8003E6CC (round 13) */
};
/* Round 14 (code_2cc8c_d): a plain 2-word record, copied as one whole-
   struct assignment (see Unk18Obj::unk34/unk38, func_8003EA0C) --
   MEASURED, retail loads both source words before storing either, ruling
   out sequential per-field copies same as the SByte3_d294 tell below. */
typedef struct Pair32_d294 {
    s32 a;
    s32 b;
} Pair32_d294;

/* Round 13 (code_2cc8c_d): a 3-signed-byte record, copied as one whole-
   struct assignment (see Unk18Obj::unk58/unk5B). */
typedef struct SByte3_d294 {
    s8 b0;
    s8 b1;
    s8 b2;
} SByte3_d294;

struct Unk18Obj {
    Unk18ObjMethods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    GenericObj *unkC;          /* +0x00C, zeroed by the ctor
                                  (func_8003E628); OBSERVED (round 13,
                                  set to `arg1`) by func_8003E770 when
                                  `arg1->methods->header & 0xF == 1` */
    GenericObj *unk10;         /* +0x010, zeroed by the ctor
                                  (func_8003E628); OBSERVED (round 13,
                                  set to `arg1`) by func_8003E770 when
                                  `arg1->methods->header & 0xF == 4` */
    /* +0x014, round 14: a Vec3, written wholesale by func_8003EBC4 from
       its own arg1 -- RETYPED from an opaque byte span once
       func_8003EBC4's own store pattern (3 plain word stores at +0x14/
       +0x18/+0x1C) confirmed the shape; func_8003EACC's own use (only the
       address, forwarded to func_8003F2AC) is unaffected by the retype. */
    Vec3_2cc8c unk14;
    /* +0x020, round 14: another Vec3, written wholesale by func_8003EBF8
       from its own arg1 -- same shape/evidence as unk14 just above. */
    Vec3_2cc8c unk20;
    u8 pad2C[0x030 - 0x02C];
    s32 unk30;                 /* +0x030, OBSERVED: func_8003E770 (round
                                  13), set from `arg1->unk14` on the same
                                  `header == 4` path that sets `unk10` */
    /* +0x034, round 14 (func_8003EA0C): copied wholesale from a caller-
       supplied Pair32_d294 -- MEASURED, retail loads both source words
       before storing either, ruling out sequential per-field stores. */
    Pair32_d294 unk34;
    /* +0x03C..+0x048, round 13 (code_2cc8c_d): four plain field setters
       (func_8003EA24/EA2C/EA48/EA64), all `sw $a1, N($a0)` or the same
       guarded by `if (self->unk70 == 0)`. No further evidence of real
       type/meaning beyond "a stored word", so kept `s32`. */
    s32 unk3C;                  /* +0x03C, OBSERVED: func_8003EA24 (round 13) */
    s32 unk40;                  /* +0x040, OBSERVED: func_8003EA64 (round 13) */
    s32 unk44;                  /* +0x044, OBSERVED: func_8003EA2C (round 13),
                                    only written when `self->unk70 == 0` */
    s32 unk48;                  /* +0x048, OBSERVED: func_8003EA48 (round 13),
                                    only written when `self->unk70 == 0` */
    s32 unk4C;                  /* +0x04C, OBSERVED: func_8003EEC0 (round 14) */
    s32 unk50;                  /* +0x050, OBSERVED: func_8003EEC0 (round 14) */
    s32 unk54;                  /* +0x054, OBSERVED: func_8003EA7C (round 13) */
    /* +0x058/+0x05B, round 13 (code_2cc8c_d): two 3-byte fields, each
       copied wholesale from a caller-supplied 3-byte source via a WHOLE
       struct assignment (func_8003EA84/EAA4 -- MEASURED: retail loads all
       three source bytes before storing any of them, ruling out a
       sequential per-field copy). Bytes are signed (`lb`, not `lbu`).
       Real element type unknown, so named generically rather than guessed
       as e.g. an RGB triple. */
    SByte3_d294 unk58;          /* +0x058, OBSERVED: func_8003EA84 (round 13) */
    SByte3_d294 unk5B;          /* +0x05B, OBSERVED: func_8003EAA4 (round 13) */
    u8 pad05E[0x060 - 0x05E];
    s32 unk60;                  /* +0x060, OBSERVED: func_8003EAC4 (round 13) */
    u8 pad064[0x070 - 0x064];
    /* +0x070, round 13 (code_2cc8c_d): a guard flag -- func_8003EA2C/EA48
       (above) only write unk44/unk48 when this is zero/NULL, i.e. a
       "already initialized" latch. RESOLVED round 14: func_8003ECD0 is its
       own set site -- a one-time allocator/init routine, guarded by this
       same flag, that sets it to 1 (and zeroes unk74) once it succeeds. */
    s32 unk70;
    s32 unk74;                  /* +0x074, OBSERVED: func_8003ECD0 (round 14),
                                    zeroed alongside unk70 */
    /* +0x078..+0x08C, round 14 (func_8003ECD0): seven `s32`-typed
       addresses/sizes carved out of one `func_80017B34` allocation --
       MEASURED, not modeled as real pointer types since retail computes
       every one of them via plain word arithmetic (not pointer-typed
       addition), and unk78/unk7C are ALSO dereferenced directly as raw
       2-word records (`*(s32*)unk78 = ...; *(s32*)(unk78+4) = ...;`).
       Real structure/meaning beyond "byte offsets within one buffer"
       unknown. */
    s32 unk78;
    s32 unk7C;
    s32 unk80;
    s32 unk84;
    s32 unk88;
    s32 unk8C;
    s32 unk90;                  /* +0x090, OBSERVED: func_8003EE40 (round 14),
                                    incremented unconditionally every call */
    u8 pad094[0x098 - 0x094];
    s32 unk98;                  /* +0x098, OBSERVED: func_8003EEC0 (round 14),
                                    a running count incremented by 1 each call */
    u8 pad09C[0x0AC - 0x09C];
    Unk18AcObj *unkAC;          /* +0x0AC, OBSERVED: func_8003E628 (round 13,
                                  set from `func_8001CA94()`, a
                                  `New_Class6B5CC` allocator, `code_d294.c`)
                                  and func_8003E6CC (round 13, dereferenced
                                  and released -- see `Unk18AcObj`'s own
                                  comment) */
    SubHandleObj *unkB0;        /* +0x0B0, OBSERVED: func_8003E628 (round
                                  13) -- set from `func_8003FDB0`; also read
                                  back by func_8003F230 (round 13, this
                                  unit) as a plain getter */
    s32 unkB4;                  /* +0x0B4, OBSERVED: func_8003F23C (round 13) */
    s32 unkB8;                  /* +0x0B8, OBSERVED: func_8003F244 (round 13) */
};

extern Unk18ObjMethods D_8006E8E4; /* the table itself (Unk18ObjMethods, resolved via tools/classtable.py D_8006E8E4), so func_8003F24C's own definition (code_2cc8c_d.c) can return &D_8006E8E4 */
extern Unk18ObjMethods *func_8003F24C(void); /* getter for Unk18Obj's own
                                    class table (returns &D_8006E8E4);
                                    used by func_8003E5D8's own New_X
                                    allocator. RETARGETED round 13: this
                                    used to live in a still-uncarved
                                    remainder, alpha's own comment said
                                    "not this unit's function to write" --
                                    the round-13 carve of code_2cc8c_d
                                    brought it in, so it is matched there
                                    now. */
extern Unk18Obj *func_8003E5D8(void); /* this unit's own New_X allocator for
                                    Unk18Obj, 0xBC bytes; forward-declared
                                    here since func_8003E10C (earlier in ROM
                                    order) calls it */

/* Round 13 (code_2cc8c_d): the rest of Unk18ObjMethods's own slot
   occupants this unit carves. Trivial setters/getters typed straight to
   Unk18Obj's own newly-discovered fields above; see the field comments
   for what each was OBSERVED from. */
void func_8003E8B8(Unk18Obj *self, GenericObj *arg1, s32 arg2);

/* func_8003F2AC (asm/code_2cc8c_e.s, the NEXT slice, still uncarved):
   func_8003EACC (round 14, this unit) calls it with only `&self->unk14`
   set up; declared here only with that call site's own shape. */
extern void func_8003F2AC(void *arg0);

/* func_8003FC18 (asm/code_2cc8c_e.s, the NEXT slice, still uncarved):
   func_8003ECD0 (round 14, this unit) calls it twice, always with its own
   1st/2nd arguments literal 0 and its own 3rd argument one of the two
   buffer addresses it just built; declared here only with that shape. */
extern void func_8003FC18(s32 a0, s32 a1, s32 a2);

/* func_80021114 (asm/psyq_GsLinkObject4.s, PsyQ library, not game code):
   func_8003EDF4 (round 14, this unit) calls it with a literal 0 and
   ignores the return; declared here only with that shape. */
extern void func_80021114(s32 a0);

/* The following (asm/code_2cc8c_e.s, the NEXT slice, still uncarved)
   are all called only from func_8003EEC0 (round 14, this unit); declared
   here only with that call site's own shapes. */
extern void func_8003FB0C(s32 a0);
extern void func_8003FC70(s32 a0);
extern void func_8003FD4C(s32 a0, s32 a1);
extern void func_8003FBE4(s32 a0);

/* func_80024AE4 (asm/psyq_GsLinkObject4.s, PsyQ library, not game code):
   func_8003EEC0 calls it with self->unk5B's own three bytes reinterpreted
   as UNSIGNED (`lbu`, not `lb` -- despite `unk5B` itself being written as
   signed bytes by func_8003EAA4, this call site reads them unsigned; kept
   as a local cast rather than retyping the field, since the two readings
   disagree). */
extern void func_80024AE4(u8 a0, u8 a1, u8 a2);

void func_8003EACC(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5);
void func_8003EB84(Unk18Obj *self);
void func_8003EBC4(Unk18Obj *self, Vec3_2cc8c *a1);
void func_8003EBF8(Unk18Obj *self, Vec3_2cc8c *a1);
void func_8003ECD0(Unk18Obj *self);
void func_8003EDF4(Unk18Obj *self);
void func_8003EE40(Unk18Obj *self, GenericObj *arg1, s32 arg2);
void func_8003EE88(Unk18Obj *self, GenericObj *arg1, s32 arg2);
void func_8003EEC0(Unk18Obj *self);
void func_8003EA0C(Unk18Obj *self, Pair32_d294 *pair);
void func_8003EA24(Unk18Obj *self, s32 a1);
void func_8003EA2C(Unk18Obj *self, s32 a1);
void func_8003EA48(Unk18Obj *self, s32 a1);
void func_8003EA64(Unk18Obj *self, s32 a1);
void func_8003EA7C(Unk18Obj *self, s32 a1);
void func_8003EA84(Unk18Obj *self, SByte3_d294 *src);
void func_8003EAA4(Unk18Obj *self, SByte3_d294 *src);
void func_8003EAC4(Unk18Obj *self, s32 a1);
SubHandleObj *func_8003F230(Unk18Obj *self);
void func_8003F23C(Unk18Obj *self, s32 a1);
void func_8003F244(Unk18Obj *self, s32 a1);
Unk18Obj *func_8003F25C(Unk18Obj *self);

/* func_80024B90 (asm/psyq_GsLinkObject4.s, PsyQ library, not game code):
   func_8003F28C (round 13, code_2cc8c_d) calls it with `self` forwarded
   unexamined and ignores the return; declared here only with that
   call site's own shape. */
extern void func_80024B90(Unk18Obj *self);
void func_8003F28C(Unk18Obj *self);

extern void *func_80042400(void); /* external, no args; local view returns
                                    void* (used as a generic word/child
                                    pointer here); same callee as
                                    class_39e08.h's own `SubObjG *` view */
extern void *func_80042694(void); /* external, no args; not yet seen
                                    elsewhere in this project */

/*
 * An ALTERNATE reading of self->unk14 (round 13, func_8003E10C only): the
 * field itself stays `s32` in `Obj86B60` below (already established,
 * generic-word usage confirmed by a sibling unit's func_8003DA10 forwarding
 * it untyped to slot100) -- same "keep the general field, cast locally"
 * shape already used for Unk4CObj->unk24[idx]/Unk64Elem's own
 * func_8003D980 alternate reading. Here func_8003E10C dispatches through it
 * as a pointer to an object with its own vtable; only the one slot it
 * reaches is modelled.
 */
struct Unk14ObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk14Obj *self);            /* +0x004, inherited
                                                    BasicClass "release";
                                                    OBSERVED: func_8003E280
                                                    (round 13) */
    u8 pad008[0x010 - 0x008];
    void (*slot10)(Unk14Obj *self, void *a1);  /* +0x010, inherited
                                                    BasicClass addChild;
                                                    OBSERVED: func_8003E10C */
    void (*slot14)(Unk14Obj *self, void *a1);  /* +0x014, inherited
                                                    BasicClass removeChild;
                                                    OBSERVED: func_8003E280
                                                    (round 13) */
};
struct Unk14Obj {
    Unk14ObjMethods *methods; /* +0x000 */
};

/*
 * A THIRD alternate reading of the same shape, this time for self->unk10
 * (round 13, func_8003E280 only): released through the identical inherited
 * BasicClass "release" slot self->unk14/unk18's own pointee types use.
 * self->unk10 itself stays `s32` in `Obj86B60` (already established,
 * generic-word/child-pointer usage confirmed by func_8003E10C) -- cast
 * locally here, same convention as `Unk14Obj`.
 */
struct Unk10ObjMethods {
    u8 pad000[0x004];
    void *(*slot4)(Unk10Obj *self);            /* +0x004, inherited
                                                    BasicClass "release";
                                                    OBSERVED: func_8003E280 */
    u8 pad008[0x044 - 0x008];
    void (*slot44)(Unk10Obj *self);            /* +0x044, OBSERVED:
                                                    func_8003E418 (round 13) */
};
struct Unk10Obj {
    Unk10ObjMethods *methods; /* +0x000 */
};

/*
 * self->unkC->unk4's pointee (round 13, func_8003E418) -- the SAME field
 * `func_8003E10C` forwards as an opaque `addChild` child and `func_8003E280`
 * forwards as a `removeChild` target; this function is the first to
 * dereference it as a real class instance. Only the two slots it dispatches
 * through are modelled.
 */
struct Unk4ArgObjMethods {
    u8 pad000[0x044];
    void (*slot44)(Unk4ArgObj *self); /* +0x044, OBSERVED: func_8003E418 */
    void (*slot48)(Unk4ArgObj *self); /* +0x048, OBSERVED: func_8003E418 */
};
struct Unk4ArgObj {
    Unk4ArgObjMethods *methods; /* +0x000 */
};

/*
 * self->unkC->unk0's pointee (round 13, func_8003E578) -- the SAME field
 * `func_8003E10C`/`func_8003E280` forward as an opaque `addChild`/
 * `removeChild` child; this function is the first to dereference it as a
 * real class instance (same "one field, multiple independent-evidence
 * readings" shape as `Unk4ArgObj` for the adjacent `unk4` field). Only the
 * one slot this function dispatches through is modelled.
 */
typedef struct Unk0ArgObj Unk0ArgObj;
typedef struct Unk0ArgObjMethods Unk0ArgObjMethods;
struct Unk0ArgObjMethods {
    u8 pad000[0x04C];
    void (*slot4C)(Unk0ArgObj *self); /* +0x04C, OBSERVED: func_8003E578 */
};
struct Unk0ArgObj {
    Unk0ArgObjMethods *methods; /* +0x000 */
};

/*
 * func_8003E10C's 2nd parameter (round 13) -- a small "init args" struct:
 * two children forwarded to the inherited BasicClass addChild (self->
 * methods->slot10), and three optional fields each read with a "use if
 * set, else derive from a helper call" idiom mirroring self->unk10/unk14/
 * unk18's own construction. Only the five fields func_8003E10C touches are
 * modelled.
 */
struct Obj86B60InitArgs {
    Unk0ArgObj *unk0; /* +0x000, forwarded to self->methods->slot10 (child)
                          as `void *`; ALSO OBSERVED (round 13) dereferenced
                          directly by func_8003E578 as a real class instance
                          -- see Unk0ArgObj's own comment */
    Unk4ArgObj *unk4; /* +0x004, forwarded to self->methods->slot10 (child)
                          as `void *`; ALSO OBSERVED (round 13) dereferenced
                          directly by func_8003E418 as a real class instance
                          -- see Unk4ArgObj's own comment */
    void *unk8;      /* +0x008, fallback source for self->unk10 */
    void *unkC;      /* +0x00C, fallback source for self->unk14 */
    Unk18Obj *unk10; /* +0x010, fallback source for self->unk18 */
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
    u8 pad000[0x010];
    void (*slot10)(Obj86B60 *self, void *a1);     /* +0x010, inherited
                                                      BasicClass addChild
                                                      (BasicClass__func_17f98);
                                                      OBSERVED: func_8003E10C */
    void (*slot14)(Obj86B60 *self, void *a1);     /* +0x014, inherited
                                                      BasicClass removeChild
                                                      (BasicClass__func_17ff0);
                                                      OBSERVED: func_8003E280
                                                      (round 13) */
    u8 pad018[0x030 - 0x018];
    void (*slot30)(Obj86B60 *self);               /* +0x030, inherited
                                                      BasicClass slot
                                                      (BasicClass__func_182cc);
                                                      OBSERVED: func_8003E4B8
                                                      (round 13) */
    u8 pad034[0x040 - 0x034];
    void (*slot40)(Obj86B60 *self);               /* +0x040, IS
                                                      func_8003E100 (already
                                                      matched); OBSERVED:
                                                      func_8003DFDC */
    u8 pad044[0x048 - 0x044];
    void (*slot48)(Obj86B60 *self);               /* +0x048, IS
                                                      func_8003E280;
                                                      OBSERVED: func_8003E10C */
    void (*slot4C)(Obj86B60 *self, s32 a1, s32 a2, s32 a3); /* +0x04C,
                                                      external (func_8003C238);
                                                      OBSERVED: func_8003E10C */
    void (*slot50)(Obj86B60 *self);               /* +0x050, external
                                                      (func_8004D898);
                                                      OBSERVED: func_8003E280
                                                      (round 13) */
    void (*slot54)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x054, IS
                                                      func_8003E418;
                                                      OBSERVED: func_8003E030 */
    void (*slot58)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x058,
                                                      external (func_8003C48C,
                                                      STALL in code_2cc8c);
                                                      OBSERVED: func_8003E030 */
    void (*slot5C)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x05C, IS
                                                      func_8003C51C (already
                                                      matched there with a1
                                                      typed s32 -- an
                                                      independent local view,
                                                      same shared slot);
                                                      OBSERVED: func_8003E030 */
    void (*slot60)(Obj86B60 *self, s32 reason);  /* +0x060, external
                                                      (func_8004D90C) */
    void (*slot64)(Obj86B60 *self);               /* +0x064, IS
                                                      func_8003E538 (already
                                                      matched); OBSERVED:
                                                      func_8003E4B8
                                                      (round 13) */
    void (*slot68)(Obj86B60 *self);               /* +0x068, IS
                                                      func_8003E578;
                                                      OBSERVED: func_8003E4B8
                                                      (round 13) */
    u8 pad06C[0x070 - 0x06C];
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
    u8 pad0F4[0x0F8 - 0xF4];
    void (*slotF8)(Obj86B60 *self, void *a1, Unk74Obj *a2); /* +0x0F8,
                                                       OBSERVED:
                                                       func_8003CE98
                                                       (round 12) */
    void (*slotFC)(Obj86B60 *self);                 /* +0x0FC, OBSERVED:
                                                       func_8003D050
                                                       (round 12) */
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
    s32 unk10;                  /* +0x010, func_8003E874: zeroed by the same
                                    ctor-shaped function as unkC/unk30.
                                    ALSO OBSERVED (round 13) by func_8003E10C,
                                    which sets it from an init-args field or
                                    a helper call and forwards it BOTH as an
                                    addChild-style child argument and (cast
                                    locally) as Unk14Obj::slot10's 2nd arg --
                                    kept `s32` (the more general reading) per
                                    this header's "keep the general field,
                                    cast locally" convention; see Unk14Obj's
                                    own comment above. */
    s32 unk14;                  /* +0x014, func_8003DA10: forwarded as
                                    slot100's 2nd arg -- generic word.
                                    ALSO OBSERVED (round 13) by func_8003E10C,
                                    which both sets it (from an init-args
                                    field or a helper call) and, on one path,
                                    dispatches through it as a pointer to an
                                    object with its own vtable (cast locally
                                    to `Unk14Obj *`, see that type's own
                                    comment) -- kept `s32` here since that is
                                    still the more general of the two
                                    observed readings. */
    Unk18Obj *unk18;             /* +0x018, func_8003E10C (round 13): set
                                    from an init-args field or from this
                                    unit's own New_X allocator
                                    (func_8003E5D8), then dispatched through
                                    (`self->unk18->methods->slot10(...)`) */
    s32 unk1C;                  /* +0x01C, func_8003CC2C (a running count/
                                    frame value multiplied against unk84);
                                    func_8003C63C (STALL) zeroes it on
                                    several message codes */
    u8 pad020[0x020 - 0x020];
    s32 unk20;                  /* +0x020, func_8003C63C (STALL) sets it
                                    to a literal 5 */
    s32 unk24;                  /* +0x024, func_8003E10C (round 13): set to
                                    arg2 (also gates the rest of that
                                    function's body on == 0) */
    u8 pad028[0x030 - 0x028];
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
    Unk68Obj *unk68;             /* +0x068, OBSERVED: func_8003D050,
                                     func_8003DAD4, func_8003D73C (round 12)
                                     -- built once by func_8003CE98 via
                                     func_800404D0(&D_8008A8E8, &D_8008A8F0,
                                     0), then dispatched through repeatedly */
    u8 pad06C[0x070 - 0x06C];
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
 * same table into one shared header. Only the slots this unit's queued
 * functions actually dispatch through are modelled.
 */
typedef struct BasicClassMethodsCC8C BasicClassMethodsCC8C;
struct BasicClassMethodsCC8C {
    u8 pad000[0x008];
    void (*ctor)(void *self); /* +0x008, IS BasicClass__BasicClass
                                  (code_8220.c); OBSERVED: func_8003DFDC */
    void (*slot0C)(void *self); /* +0x00C, IS BasicClass__func_17f2c
                                  (code_8220.c, "finalize"); OBSERVED:
                                  func_8003E6CC (round 13) */
    void (*slot10)(void *self, void *child); /* +0x010, IS
                                  BasicClass__func_17f98 (code_8220.c,
                                  "addChild"); OBSERVED: func_8003E770
                                  (round 13) */
    void (*slot14)(void *self, void *child); /* +0x014, IS
                                  BasicClass__func_17ff0 (code_8220.c,
                                  "removeChild"); OBSERVED: func_8003E7F4
                                  (round 13) */
    void (*slot18)(void *self); /* +0x018, func_8003E874's forward target */
    u8 pad01C[0x038 - 0x01C];
    void (*slot38)(void *self, void *arg1, s32 arg2); /* +0x038, IS
                                  BasicClass__func_18358 (code_8220_b);
                                  OBSERVED: func_8003E030 */
};

extern BasicClassMethodsCC8C *func_80018390(void);

#endif
