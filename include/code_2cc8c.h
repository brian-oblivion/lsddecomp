#ifndef CODE_2CC8C_H
#define CODE_2CC8C_H

#include "common.h"

/* Forward typedefs, used by `extern` declarations further up this file
 * than their own struct bodies (round 14, code_2cc8c_e's own local views,
 * referenced by earlier code_2cc8c_d call-site declarations). */
typedef struct TexPageDesc TexPageDesc;
typedef struct Class6E99CObj Class6E99CObj;
typedef struct ClassEAC0Obj ClassEAC0Obj;
typedef struct Pair32E99C Pair32E99C;

/*
 * The class whose method table is gClass86B60Methods (78 slots, base) with a
 * derived override table at gGraphRoomMethods (73 slots) -- resolved with
 * tools/classtable.py gClass86B60Methods / gClass86B60Methods --vs gGraphRoomMethods. No FirecatFG
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
 * Obj86B60__OnNotify to pick which of self->methods->slot54/58/5C to forward to.
 * Same two-type shape as class_39e08.h's own independent `EventArg`/
 * `HeaderObj` local view (a `target` pointer to an object whose first word
 * is a low-nibble-coded header/kind value) -- this unit keeps its own
 * separate local view per the project's established convention. Only the
 * one field/offset Obj86B60__OnNotify touches is modelled.
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
                                `strlen` and passed to
                                `New_Obj6EAC0` to build each entry of
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

extern void *BMemPMgrAlloc(s32 size);   /* allocator, confirmed across many
                                            units */
extern void *BMemPMgrFree(void *ptr);  /* matching free/release. Its own
                                            disassembly (still INCLUDE_ASM,
                                            asm/nonmatchings/code_8220/
                                            BMemPMgrFree.s) ends with an
                                            explicit `addu $v0,$zero,$zero`
                                            -- it genuinely returns NULL,
                                            not void. code_171e0.h/Entity.h
                                            type it `void` because every
                                            caller there discards the
                                            result (the established
                                            "a discarded return value is
                                            never evidence of void" trap);
                                            round 14 (code_2cc8c_f) needs
                                            the real return value, so this
                                            unit's shared view is retyped.
                                            Every existing call site in
                                            this unit (code_2cc8c_b.c,
                                            code_2cc8c_d.c) discards the
                                            result too, so this is a
                                            zero-byte-cost retype -- full
                                            build reconfirmed green. */
extern void ReleaseBasicClassArray(void *a0, void *a1); /* not yet seen elsewhere in
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
                      element is passed to strlen (already typed
                      `s32 strlen(char *s)` in code_171e0.h) and to
                      New_Obj6EAC0 */
};

extern s32 strlen(char *s); /* Psy-Q libc2/strlen, linked from Sony's
                                        own object; local view here */
extern Unk64Elem *New_Obj6EAC0(void *ctx, s32 len, char *name); /* not
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
 * func_8003D73C). Built by `New_ClassEAC0(&D_8008A8E8, &D_8008A8F0, 0)` in
 * func_8003CE98 -- New_ClassEAC0 itself lives in the still-uncarved
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
    void (*slot4C)(Unk68Obj *self, s32 a1, void *pos); /* +0x04C, OBSERVED:
                                                     func_8003D73C, its only
                                                     caller, passes THREE
                                                     (round 75) */
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

/* Retyped round 14 once code_2cc8c_e's own body was matched: this is the
   New_X allocator for `ClassEAC0Obj` (see the Class6E99CObj/ClassEAC0Obj
   section far below) -- `BMemPMgrAlloc(0x6C)` then
   `Obj6EAC0__GetBaseMethods()->ctor(self, a0, a1, a2)`. Parameter types are UNCHANGED
   from the existing declaration (both already pointer/pointer/s32, matching
   this call site's own real arguments exactly); only the RETURN type
   differs from `code_2cc8c_b.c`'s own `Unk68Obj *` view -- ABI-identical
   (a plain pointer either way), verified with a full rebuild. */
extern ClassEAC0Obj *New_ClassEAC0(void *a0, void *a1, s32 a2);
extern s32 D_8008A8E8[2];   /* address-taken only by this unit */
extern char D_8008A8F0[4];  /* address-taken only by this unit */

/*
 * The pointee of `Unk18Obj->unkB0` (round 13, Unk18Obj__Unk18Obj), returned by
 * `New_Class6E99C` -- a function already known elsewhere in this project
 * (`include/Entity.h`'s own `Unk100Obj`/`New_Class6E99C`), kept here under a
 * unit-local name per this project's established "independent local views"
 * convention. Only the one slot this unit's `Unk18Obj__Unk18Obj` dispatches
 * through is modelled.
 */
struct SubHandleObjMethods {
    u8 pad000[0x004];
    void (*slot4)(SubHandleObj *self); /* +0x004, OBSERVED: Unk18Obj__SetSubHandle
                                    (round 14) -- release-shaped, no extra
                                    args */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(SubHandleObj *self, void *arg1, void *arg2); /* +0x04C,
                                    OBSERVED: Unk18Obj__Unk18Obj */
};
struct SubHandleObj {
    SubHandleObjMethods *methods; /* +0x000 */
};

/* Retyped round 14 once code_2cc8c_e's own body was matched: this is the
   New_X allocator for `Class6E99CObj` (this unit's own view, see the
   Class6E99CObj/ClassEAC0Obj section far below) -- `BMemPMgrAlloc(0xA0)`
   then `GetClass6E99CMethods()->ctor(self, a1, a2, a3)`. `a1`/`a2`/`a3` forward
   straight through to that ctor unmodified; `a1` is a pointer (confirmed
   by THIS unit's own two real callers, `code_2cc8c_c.c` passing
   `D_8008A90C` and `Entity.c` passing its own `name` parameter, both
   already-matched). The return type differs from the narrower
   `SubHandleObj *`/`Unk100Obj *` views those two callers (and
   `include/Entity.h`) keep for their OWN local field types -- ABI-
   identical (a plain pointer either way), so this is a compatible
   retype: both existing call sites already assign the result into their
   OWN separately-typed local, so this only changes an implicit-conversion
   warning at the assignment, not the compiled bytes. Verified with a full
   rebuild. */
extern Class6E99CObj *New_Class6E99C(void *a1, s32 a2, s32 a3);
extern Unk18AcObj *New_Class6B5CC(void); /* local view of include/code_d294.h's
                                    own `New_Class6B5CC` allocator, returning
                                    `Class6B5CCObj *` there -- this unit's
                                    own view retyped (round 13) once
                                    Unk18Obj__Finalize dereferenced it, see
                                    Unk18AcObj's own comment */
extern u8 D_8008A90C[]; /* address-taken only by this unit, passed as
                            New_Class6E99C's "name" argument */
extern u8 D_8008A904[]; /* address-taken only by this unit, passed as
                            SubHandleObjMethods::slot4C's 3rd argument */
extern u8 D_8008A8F4[]; /* round 14, code_2cc8c_d (asm/data/7B008.sdata.s,
                            not decompiled): address-taken only, passed as
                            Unk18Obj__AttachViewChild's own default value for slot80's
                            2nd argument when its own arg5 is NULL. */

/*
 * `Unk18Obj->unkAC`'s pointee (round 13, Unk18Obj__Finalize) -- the return of
 * `New_Class6B5CC`, first stored opaquely by `Unk18Obj__Unk18Obj` and here
 * dereferenced and released through the inherited BasicClass "release"
 * slot. Only that one slot is modelled.
 */
struct Unk18AcObjMethods {
    u8 pad000[0x004];
    void *(*release)(Unk18AcObj *self); /* +0x004, inherited BasicClass
                                          "release"; OBSERVED: Unk18Obj__Finalize.
                                          Renamed from slot4, round 55 --
                                          exclusive to this unit (only
                                          Unk18Obj__Finalize dereferences
                                          `self->unkAC->methods`;
                                          code_2cc8c_d.c passes `unkAC` along
                                          opaquely without going through its
                                          own vtable). Matches the canonical
                                          BasicClassMethods name at this
                                          offset (include/code_8220.h). */
};
struct Unk18AcObj {
    Unk18AcObjMethods *methods; /* +0x000 */
};

/*
 * A generic class-instance view (round 13, Unk18Obj__AddChild): every class's
 * vtable in this game begins with a "header" word (`tools/classtable.py`'s
 * own label for it, "not a pointer; varies per class -- id/flags"), and
 * `Unk18Obj__AddChild` discriminates its 2nd parameter's DYNAMIC CLASS by
 * reading `arg1->methods->header & 0xF` -- i.e. runtime type identification
 * through the vtable header nibble, not a struct field of `arg1` itself.
 * Only that one field, plus the one instance field (`unk14`) this function
 * also reads, are modelled; `arg1`'s real class is unknown and irrelevant
 * to this function's own behaviour.
 */
struct GenericObjMethods {
    s32 header; /* +0x000 */
    /* +0x050/+0x054, round 14 (Unk18Obj__Flip's own call site): dispatched
       as `(self)` only. Occupants unknown (self->unkC's real class is not
       otherwise identified in this unit). */
    u8 pad004[0x050 - 0x004];
    void (*slot50)(GenericObj *self); /* +0x050, return value unused */
    s32 (*slot54)(GenericObj *self);  /* +0x054, return stored into self->unk74 */
};
struct GenericObj {
    GenericObjMethods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    void *unkC;                 /* +0x00C, OBSERVED: Unk18Obj__Update (round 14),
                                    truthy-tested only */
    u8 pad010[0x014 - 0x010];
    s32 unk14;                  /* +0x014, OBSERVED: Unk18Obj__AddChild */
};

/*
 * self->viewport's pointee (field renamed from unk18 round 55), round 13
 * (Obj86B60__Init). Constructed by a
 * New_X allocator this unit itself carves (New_Unk18Obj, 0xBC bytes) via
 * `GetUnk18ObjMethods()->ctor(self)` -- GetUnk18ObjMethods lives in a still-uncarved
 * remainder of this segment (not this unit's function to write), so it is
 * declared here only as an external returning this unit's own local view
 * of the class table it constructs. Only the one slot Obj86B60__Init
 * dispatches through is modelled.
 */
/* Round 14 (code_2cc8c_d): a plain 3-word vector, copied wholesale from a
 * caller-supplied source into Unk18Obj::unk14 (Unk18Obj__SetViewPos). Local view,
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
    void *(*release)(Unk18Obj *self);            /* +0x004, inherited
                                                    BasicClass "release"
                                                    (finalize then free);
                                                    OBSERVED: Obj86B60__Deinit
                                                    (round 13). Renamed from
                                                    slot4, round 55 --
                                                    exclusive to this unit:
                                                    code_2cc8c_d.c never
                                                    dispatches through this
                                                    exact slot on a Unk18Obj*
                                                    (checked: its own
                                                    `methods->slot4` calls
                                                    are on unrelated types).
                                                    Matches the canonical
                                                    BasicClassMethods name at
                                                    this offset
                                                    (include/code_8220.h). */
    void (*ctor)(Unk18Obj *self);              /* +0x008, called by
                                                    New_Unk18Obj with only
                                                    `self` set up */
    u8 pad00C[0x010 - 0x00C];
    void (*addChild)(Unk18Obj *self, void *a1);  /* +0x010, inherited
                                                    BasicClass addChild;
                                                    OBSERVED: Obj86B60__Init */
    void (*removeChild)(Unk18Obj *self, void *a1);  /* +0x014, inherited
                                                    BasicClass removeChild;
                                                    OBSERVED: Obj86B60__Deinit
                                                    (round 13) */
    u8 pad018[0x040 - 0x018];
    void (*slot40)(Unk18Obj *self);            /* +0x040, OBSERVED:
                                                    Unk18Obj__Unk18Obj (round 13)
                                                    -- a DIFFERENT table from
                                                    Obj86B60Methods's own
                                                    slot40 (`D_8006E8E4`'s
                                                    own occupant here is
                                                    `Unk18Obj__InitDefaults`, not
                                                    `Obj86B60__ResetCounters`) */
    u8 pad044[0x074 - 0x044];
    void (*slot74)(Unk18Obj *self);            /* +0x074, OBSERVED:
                                                    Unk18Obj__Finalize (round 13) */
    /* +0x078/+0x07C/+0x080, round 14 (Unk18Obj__AttachViewChild's own call site,
       guarded by `self->unk10 == NULL`): dispatched as `(self, a2)`,
       `(self, a3)`, `(self, a1_or_default)` respectively. Occupants (this
       unit, per tools/classtable.py D_8006E8E4): Unk18Obj__SetViewPos (+0x078,
       still queued), Unk18Obj__SetUnk20 (+0x07C, still queued), Unk18Obj__SetRatio12
       (+0x080, the documented gp_rel blocker -- NOT decompiled here). */
    void (*slot78)(Unk18Obj *self, Vec3_2cc8c *a1); /* +0x078, retyped round 14 once Unk18Obj__SetViewPos (its own occupant) confirmed the shape */
    void (*slot7C)(Unk18Obj *self, Vec3_2cc8c *a1); /* +0x07C, retyped round 14 once Unk18Obj__SetUnk20 (its own occupant) confirmed the shape */
    void (*slot80)(Unk18Obj *self, void *a1); /* +0x080 */
    u8 pad084[0x090 - 0x084];
    void (*slot90)(Unk18Obj *self);            /* +0x090, OBSERVED:
                                                    Unk18Obj__Finalize (round 13) */
    /* +0x094/+0x098, round 14 (code_2cc8c_d): Unk18Obj__OnNotify's own call
       site -- dispatched as `(self, arg1, arg2)` when a GenericObj arg1's
       header tag is 5 (slot94) or 1 (slot98). Occupants (this unit, still
       queued as of this comment): Unk18Obj__OnNotifyTag5 (+0x094), Unk18Obj__OnNotifyTag1
       (+0x098). */
    void (*slot94)(Unk18Obj *self, GenericObj *arg1, s32 arg2); /* +0x094 */
    void (*slot98)(Unk18Obj *self, GenericObj *arg1, s32 arg2); /* +0x098 */
    void (*slot9C)(Unk18Obj *self); /* +0x09C, occupant Unk18Obj__Update (round 14); dispatched by Unk18Obj__OnNotifyTag5 */
    /* +0x0A0, occupant func_80012064 (asm/psyq_2864.s, PsyQ library that no
       SDK disc places, so it stays disassembly -- not
       decompiled) -- dispatched by Unk18Obj__Update (round 14) at three call
       sites with different arities (self alone; self+unkAC; self+another
       Unk18Obj*), so kept as an untyped function pointer and cast per
       call site rather than picking one fixed signature. */
    void *slotA0;
    void (*slotA4)(Unk18Obj *self); /* +0x0A4, occupant Unk18Obj__Flip (round 14, still queued as of this comment); dispatched by Unk18Obj__OnNotifyTag1 */
    void (*slotA8)(Unk18Obj *self, s32 a1);    /* +0x0A8, OBSERVED:
                                                    Unk18Obj__Finalize (round 13) */
};
/* Round 14 (code_2cc8c_d): a plain 2-word record, copied as one whole-
   struct assignment (see Unk18Obj::unk34/unk38, Unk18Obj__SetUnk34) --
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
                                  (Unk18Obj__Unk18Obj); OBSERVED (round 13,
                                  set to `arg1`) by Unk18Obj__AddChild when
                                  `arg1->methods->header & 0xF == 1` */
    GenericObj *unk10;         /* +0x010, zeroed by the ctor
                                  (Unk18Obj__Unk18Obj); OBSERVED (round 13,
                                  set to `arg1`) by Unk18Obj__AddChild when
                                  `arg1->methods->header & 0xF == 4` */
    /* +0x014, round 14: a Vec3, written wholesale by Unk18Obj__SetViewPos from
       its own arg1 -- RETYPED from an opaque byte span once
       Unk18Obj__SetViewPos's own store pattern (3 plain word stores at +0x14/
       +0x18/+0x1C) confirmed the shape; Unk18Obj__AttachViewChild's own use (only the
       address, forwarded to GsSetRefView2) is unaffected by the retype. */
    Vec3_2cc8c unk14;
    /* +0x020, round 14: another Vec3, written wholesale by Unk18Obj__SetUnk20
       from its own arg1 -- same shape/evidence as unk14 just above. */
    Vec3_2cc8c unk20;
    /* +0x02C, round 44 (Unk18Obj__SetRatio12): a 20.12 fixed-point value, set from
       a caller-supplied `{s16 whole; s16 frac;}` pair via the same
       split-division idiom as code_d294_c's RatioToFixed12 (divide once for
       quotient+remainder, then divide the shifted remainder again for the
       fractional part), guarded by `self->unk10 != 0`. Read as raw `s16*`
       rather than reusing code_d294.h's `WholeFrac_d294` -- a different
       unit's own local view of the same shape, not a shared type. */
    s32 unk2C;
    s32 unk30;                 /* +0x030, OBSERVED: Unk18Obj__AddChild (round
                                  13), set from `arg1->unk14` on the same
                                  `header == 4` path that sets `unk10` */
    /* +0x034, round 14 (Unk18Obj__SetUnk34): copied wholesale from a caller-
       supplied Pair32_d294 -- MEASURED, retail loads both source words
       before storing either, ruling out sequential per-field stores. */
    Pair32_d294 unk34;
    /* +0x03C..+0x048, round 13 (code_2cc8c_d): four plain field setters
       (Unk18Obj__SetUnk3C/SetUnk44/SetUnk48/SetUnk40), all `sw $a1, N($a0)`
       or the same guarded by `if (self->otReady == 0)`. No further evidence
       of real type/meaning beyond "a stored word", so kept `s32`. */
    s32 unk3C;                  /* +0x03C, OBSERVED: Unk18Obj__SetUnk3C (round 13) */
    s32 unk40;                  /* +0x040, OBSERVED: Unk18Obj__SetUnk40 (round 13) */
    s32 unk44;                  /* +0x044, OBSERVED: Unk18Obj__SetUnk44 (round 13),
                                    only written when `self->otReady == 0` */
    s32 unk48;                  /* +0x048, OBSERVED: Unk18Obj__SetUnk48 (round 13),
                                    only written when `self->otReady == 0` */
    s32 unk4C;                  /* +0x04C, OBSERVED: Unk18Obj__Update (round 14) */
    s32 unk50;                  /* +0x050, OBSERVED: Unk18Obj__Update (round 14) */
    s32 lightMode;              /* +0x054, OBSERVED: Unk18Obj__SetLightMode (round 13).
                                    RENAMED round 73 (charlie): the sole real
                                    consumer is Unk18Obj__Update's own
                                    `GsSetLightMode(self->lightMode)` call,
                                    which also gates the far-color/fog-near
                                    dispatch there (`if (lightMode == 1 ||
                                    lightMode == 3)`). Exclusive to
                                    code_2cc8c_d.c. */
    /* +0x058/+0x05B, round 13 (code_2cc8c_d): two 3-byte fields, each
       copied wholesale from a caller-supplied 3-byte source via a WHOLE
       struct assignment (Unk18Obj__SetClearColor/Unk18Obj__SetFarColor -- MEASURED:
       retail loads all three source bytes before storing any of them, ruling
       out a sequential per-field copy). Bytes are signed (`lb`, not `lbu`),
       though both are also READ unsigned (`lbu`) at their one real consumer
       each. RENAMED round 73 (charlie): each field's own name comes from its
       one identified consumer in Unk18Obj__Flip/Unk18Obj__Update
       respectively (see each field's own comment) -- tier B, not tier A,
       since the RGB-triple reading is inferred from the consuming Sony API's
       own shape, not proven for the field's bit-level meaning. */
    SByte3_d294 clearColor;     /* +0x058, OBSERVED: Unk18Obj__SetClearColor (round
                                   13). Sole real consumer: Unk18Obj__Flip's
                                   `GsSortClear(rawBytes[0..2], ...)` --
                                   Sony's own screen-clear-color argument. */
    SByte3_d294 farColor;       /* +0x05B, OBSERVED: Unk18Obj__SetFarColor (round
                                   13). Sole real consumer: Unk18Obj__Update's
                                   `SetFarColor(rawBytes[0..2])` -- Sony's own
                                   GTE far-color register writer. */
    u8 pad05E[0x060 - 0x05E];
    s32 fogNear;                /* +0x060, OBSERVED: Unk18Obj__SetFogNear (round
                                   13). RENAMED round 73 (charlie): sole real
                                   consumer is Unk18Obj__Update's own
                                   `SetFogNear(self->fogNear, self->unk40)`
                                   call -- Sony's own near-fog-distance
                                   setter's first argument. Exclusive to
                                   code_2cc8c_d.c. */
    u8 pad064[0x070 - 0x064];
    /* +0x070, round 13 (code_2cc8c_d): a guard flag -- Unk18Obj__SetUnk44/
       Unk18Obj__SetUnk48 (above) only write unk44/unk48 when this is
       zero/NULL, i.e. an "already initialized" latch. RESOLVED round 14:
       Unk18Obj__InitOt is its own set site -- a one-time allocator/init
       routine, guarded by this same flag, that sets it to 1 (and zeroes
       otIndex) once it succeeds. RENAMED round 73 (charlie): the whole
       field IS this latch (Unk18Obj__InitOt/Unk18Obj__DeinitOt are its only
       set sites), so `otReady` names the mechanics directly rather than
       guessing what it gates conceptually. Exclusive to code_2cc8c_d.c. */
    s32 otReady;
    s32 otIndex;                /* +0x074, OBSERVED: Unk18Obj__InitOt (round
                                   14), zeroed alongside otReady. RENAMED
                                   round 73 (charlie): Unk18Obj__Flip both
                                   READS it (to pick which OT half to drain)
                                   and TOGGLES it (0<->1) every call -- the
                                   double-buffer index, named after that
                                   mechanic. Exclusive to code_2cc8c_d.c. */
    /* +0x078..+0x08C, round 14 (Unk18Obj__InitOt): seven `s32`-typed
       addresses/sizes carved out of one `BMemPMgrAlloc` allocation --
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
    s32 unk90;                  /* +0x090, OBSERVED: Unk18Obj__OnNotifyTag5 (round 14),
                                    incremented unconditionally every call */
    u8 pad094[0x098 - 0x094];
    s32 unk98;                  /* +0x098, OBSERVED: Unk18Obj__Update (round 14),
                                    a running count incremented by 1 each call */
    u8 pad09C[0x0AC - 0x09C];
    Unk18AcObj *unkAC;          /* +0x0AC, OBSERVED: Unk18Obj__Unk18Obj (round 13,
                                  set from `New_Class6B5CC()`, a
                                  `New_Class6B5CC` allocator, `code_d294.c`)
                                  and Unk18Obj__Finalize (round 13, dereferenced
                                  and released -- see `Unk18AcObj`'s own
                                  comment) */
    SubHandleObj *unkB0;        /* +0x0B0, OBSERVED: Unk18Obj__Unk18Obj (round
                                  13) -- set from `New_Class6E99C`; also read
                                  back by Unk18Obj__GetSubHandle (round 13, this
                                  unit) as a plain getter */
    s32 unkB4;                  /* +0x0B4, OBSERVED: Unk18Obj__SetUnkB4 (round 13) */
    s32 unkB8;                  /* +0x0B8, OBSERVED: Unk18Obj__SetUnkB8 (round 13) */
};

extern Unk18ObjMethods D_8006E8E4; /* the table itself (Unk18ObjMethods, resolved via tools/classtable.py D_8006E8E4), so GetUnk18ObjMethods's own definition (code_2cc8c_d.c) can return &D_8006E8E4 */
extern Unk18ObjMethods *GetUnk18ObjMethods(void); /* getter for Unk18Obj's own
                                    class table (returns &D_8006E8E4);
                                    used by New_Unk18Obj's own New_X
                                    allocator. RETARGETED round 13: this
                                    used to live in a still-uncarved
                                    remainder, alpha's own comment said
                                    "not this unit's function to write" --
                                    the round-13 carve of code_2cc8c_d
                                    brought it in, so it is matched there
                                    now. */
extern Unk18Obj *New_Unk18Obj(void); /* this unit's own New_X allocator for
                                    Unk18Obj, 0xBC bytes; forward-declared
                                    here since Obj86B60__Init (earlier in ROM
                                    order) calls it */

/* Round 13 (code_2cc8c_d): the rest of Unk18ObjMethods's own slot
   occupants this unit carves. Trivial setters/getters typed straight to
   Unk18Obj's own newly-discovered fields above; see the field comments
   for what each was OBSERVED from. */
void Unk18Obj__OnNotify(Unk18Obj *self, GenericObj *arg1, s32 arg2);

/* GsSetRefView2 is NO LONGER DECLARED HERE, round 33. It is Sony's
   (`libgs/gs_131.o`, linked from the SDK object) and will one day sit next to
   `include/psyq/LIBGS.H`'s own prototype for it -- two declarations of one
   Sony name in a header six units include is the `conflicting types` failure
   that CLAUDE.md and the SDK guide both warn about, and it would surface in a
   unit that never touched this line. Its one caller, Unk18Obj__AttachViewChild, now
   declares it locally in src/code_2cc8c_d.c with that call site's own shape. */

/* func_8003FC18 is NO LONGER DECLARED HERE, round 34 -- exactly the
   GsSetRefView2 case above. It is Sony's `GsClearOt` (`libgs/gs_113.o`,
   linked from the SDK object), and a second declaration of that name in a
   header six units include is the `conflicting types` failure CLAUDE.md and
   the SDK guide both warn about. Its callers declare it locally under Sony's
   name, with their own call sites' shapes, in src/code_2cc8c_d.c.
   The `TexPageDesc *` view that used to hang off this prototype was
   code_2cc8c_e.c's reading of a Psy-Q `GsOT`; the struct stays in this header
   because other code uses it, and no byte depends on the naming. */

/* DrawSync is NO LONGER DECLARED HERE, round 53. It is Sony's
   `DrawSync` (`libgpu/sys.o`, fingerprint exact vs the disc corpus, not yet
   linked from an SDK object) -- same collision reason as GsSetRefView2 and
   GsClearOt above: LIBGPU.H carries its own prototype (`extern int
   DrawSync(int mode);`), and a second declaration of that name in a header
   six units include is the `conflicting types` failure CLAUDE.md and the SDK
   guide both warn about. Its one caller, Unk18Obj__DeinitOt, now declares it
   locally in src/code_2cc8c_d.c with that call site's own shape. */

/* The following are called only from Unk18Obj__Update (this unit). They are
   plain `void *` global setters (this call site happens to pass an
   already-`s32`-shaped value, which is an ordinary int-to-pointer conversion
   with identical codegen, same precedent as Class6B5CC__DispatchLinkCommand/Class6B5CC__TryAttachNearby in
   code_d294.h); the retypes come from the functions' own definitions and are
   ABI-identical (word-sized values either way), so they do not change this
   call site's own compiled bytes.
   ROUND 34: both are still GAME CODE, but neither lives in code_2cc8c_e any
   more -- they are the two one-function units src/code_2cc8c_e0.c and
   src/code_2cc8c_e1.c, wedged between Sony objects. Those files do NOT
   include this header, so these two declarations are not checked against
   their definitions by the compiler; they agree today and must be kept in
   step by hand.
   Their two former neighbours, func_8003FC70 and func_8003FD4C, are gone from
   here: they are Sony's `GsSetLightMode` (libgs/gs_108) and `SetFogNear`
   (libgte/fog_01), declared locally in src/code_2cc8c_d.c under those names
   for the same collision reason as GsSetRefView2 and GsClearOt above. */
extern void func_8003FB0C(void *a0);
extern void func_8003FBE4(void *a0);

/* ResetGraph (asm/psyq_10ee0.s, PsyQ library, LIBGPU.H's own
   declared signature is `extern int ResetGraph(int mode);` -- declared
   locally here rather than including the whole SDK header, matching this
   unit's existing PsyQ-declaration style). Unk18Obj__Flip calls it with a
   literal 1 and ignores the return. */
extern s32 ResetGraph(s32 mode);

/* GsSortClear is NO LONGER DECLARED HERE, round 53. It is Sony's
   `GsSortClear` (`libgs/gs_001.o`, fingerprint exact vs the disc corpus, not
   yet linked from an SDK object) -- same collision reason as GsClearOt above:
   LIBGS.H carries its own prototype (`void GsSortClear(u_char r, u_char g,
   u_char b, GsOT *ot);`), and a second declaration of that name in a header
   six units include is the `conflicting types` failure CLAUDE.md and the SDK
   guide both warn about. Its one caller, Unk18Obj__Flip, now declares it
   locally in src/code_2cc8c_d.c with that call site's own shape (self->unk58's
   three bytes read unsigned, same "writer reads signed, this reader reads
   unsigned" situation as unk5B/Unk18Obj__Update, plus one more word). */

/* func_8003FBF4 is NO LONGER DECLARED HERE, round 34. It is Sony's
   `GsDrawOt` (`libgs/gs_111.o`, linked from the SDK object) -- same
   collision reason as GsSetRefView2/GsClearOt above. Its one caller,
   Unk18Obj__Flip, declares it locally in src/code_2cc8c_d.c with that call
   site's own shape. */

void Unk18Obj__AttachViewChild(Unk18Obj *self, void *a1, void *a2, void *a3, void *arg5);
void Unk18Obj__DetachViewChild(Unk18Obj *self);
void Unk18Obj__SetViewPos(Unk18Obj *self, Vec3_2cc8c *a1);
void Unk18Obj__SetUnk20(Unk18Obj *self, Vec3_2cc8c *a1);
void Unk18Obj__InitOt(Unk18Obj *self);
void Unk18Obj__DeinitOt(Unk18Obj *self);
void Unk18Obj__OnNotifyTag5(Unk18Obj *self, GenericObj *arg1, s32 arg2);
void Unk18Obj__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2);
void Unk18Obj__Update(Unk18Obj *self);
void Unk18Obj__Flip(Unk18Obj *self);
void Unk18Obj__SetSubHandle(Unk18Obj *self, SubHandleObj *arg1);
void Unk18Obj__SetUnk34(Unk18Obj *self, Pair32_d294 *pair);
void Unk18Obj__SetUnk3C(Unk18Obj *self, s32 a1);
void Unk18Obj__SetUnk44(Unk18Obj *self, s32 a1);
void Unk18Obj__SetUnk48(Unk18Obj *self, s32 a1);
void Unk18Obj__SetUnk40(Unk18Obj *self, s32 a1);
void Unk18Obj__SetLightMode(Unk18Obj *self, s32 a1);
void Unk18Obj__SetClearColor(Unk18Obj *self, SByte3_d294 *src);
void Unk18Obj__SetFarColor(Unk18Obj *self, SByte3_d294 *src);
void Unk18Obj__SetFogNear(Unk18Obj *self, s32 a1);
SubHandleObj *Unk18Obj__GetSubHandle(Unk18Obj *self);
void Unk18Obj__SetUnkB4(Unk18Obj *self, s32 a1);
void Unk18Obj__SetUnkB8(Unk18Obj *self, s32 a1);
Unk18Obj *Unk18Obj__GetTail(Unk18Obj *self);

void Unk18Obj__SetGeomScreen(Unk18Obj *self);

extern void *func_80042400(void); /* external, no args; local view returns
                                    void* (used as a generic word/child
                                    pointer here); same callee as
                                    class_39e08.h's own `SubObjG *` view */
extern void *func_80042694(void); /* external, no args; not yet seen
                                    elsewhere in this project */

/*
 * An ALTERNATE reading of self->unk14 (round 13, Obj86B60__Init only): the
 * field itself stays `s32` in `Obj86B60` below (already established,
 * generic-word usage confirmed by a sibling unit's func_8003DA10 forwarding
 * it untyped to slot100) -- same "keep the general field, cast locally"
 * shape already used for Unk4CObj->unk24[idx]/Unk64Elem's own
 * func_8003D980 alternate reading. Here Obj86B60__Init dispatches through it
 * as a pointer to an object with its own vtable; only the one slot it
 * reaches is modelled.
 */
struct Unk14ObjMethods {
    u8 pad000[0x004];
    void *(*release)(Unk14Obj *self);            /* +0x004, inherited
                                                    BasicClass "release";
                                                    OBSERVED: Obj86B60__Deinit
                                                    (round 13). Renamed from
                                                    slot4, round 55 --
                                                    exclusive to this unit
                                                    (Unk14Obj is this unit's
                                                    own local type, unused by
                                                    any other code_2cc8c
                                                    sibling), matches the
                                                    canonical BasicClassMethods
                                                    name at this offset
                                                    (include/code_8220.h). */
    u8 pad008[0x010 - 0x008];
    void (*addChild)(Unk14Obj *self, void *a1);  /* +0x010, inherited
                                                    BasicClass addChild;
                                                    OBSERVED: Obj86B60__Init.
                                                    Renamed from slot10, round
                                                    55, same exclusivity/
                                                    evidence as release
                                                    above. */
    void (*removeChild)(Unk14Obj *self, void *a1);  /* +0x014, inherited
                                                    BasicClass removeChild;
                                                    OBSERVED: Obj86B60__Deinit
                                                    (round 13). Renamed from
                                                    slot14, round 55, same
                                                    exclusivity/evidence as
                                                    release above. */
};
struct Unk14Obj {
    Unk14ObjMethods *methods; /* +0x000 */
};

/*
 * A THIRD alternate reading of the same shape, this time for self->unk10
 * (round 13, Obj86B60__Deinit only): released through the identical inherited
 * BasicClass "release" slot self->unk14/unk18's own pointee types use.
 * self->unk10 itself stays `s32` in `Obj86B60` (already established,
 * generic-word/child-pointer usage confirmed by Obj86B60__Init) -- cast
 * locally here, same convention as `Unk14Obj`.
 */
struct Unk10ObjMethods {
    u8 pad000[0x004];
    void *(*release)(Unk10Obj *self);            /* +0x004, inherited
                                                    BasicClass "release";
                                                    OBSERVED: Obj86B60__Deinit.
                                                    Renamed from slot4, round
                                                    55 -- exclusive to this
                                                    unit (Unk10Obj is this
                                                    unit's own local type),
                                                    matches the canonical
                                                    BasicClassMethods name at
                                                    this offset
                                                    (include/code_8220.h). */
    u8 pad008[0x044 - 0x008];
    void (*slot44)(Unk10Obj *self);            /* +0x044, OBSERVED:
                                                    Obj86B60__OnTag1Notify (round 13) */
};
struct Unk10Obj {
    Unk10ObjMethods *methods; /* +0x000 */
};

/*
 * self->initArgs->unk4's pointee (round 13, Obj86B60__OnTag1Notify) -- the SAME field
 * `Obj86B60__Init` forwards as an opaque `addChild` child and `Obj86B60__Deinit`
 * forwards as a `removeChild` target; this function is the first to
 * dereference it as a real class instance. Only the two slots it dispatches
 * through are modelled.
 */
struct Unk4ArgObjMethods {
    u8 pad000[0x044];
    void (*slot44)(Unk4ArgObj *self); /* +0x044, OBSERVED: Obj86B60__OnTag1Notify */
    void (*slot48)(Unk4ArgObj *self); /* +0x048, OBSERVED: Obj86B60__OnTag1Notify */
};
struct Unk4ArgObj {
    Unk4ArgObjMethods *methods; /* +0x000 */
};

/*
 * self->initArgs->unk0's pointee (round 13, Obj86B60__NotifyChildReset) -- the SAME field
 * `Obj86B60__Init`/`Obj86B60__Deinit` forward as an opaque `addChild`/
 * `removeChild` child; this function is the first to dereference it as a
 * real class instance (same "one field, multiple independent-evidence
 * readings" shape as `Unk4ArgObj` for the adjacent `unk4` field). Only the
 * one slot this function dispatches through is modelled.
 */
typedef struct Unk0ArgObj Unk0ArgObj;
typedef struct Unk0ArgObjMethods Unk0ArgObjMethods;
struct Unk0ArgObjMethods {
    u8 pad000[0x04C];
    void (*slot4C)(Unk0ArgObj *self); /* +0x04C, OBSERVED: Obj86B60__NotifyChildReset */
};
struct Unk0ArgObj {
    Unk0ArgObjMethods *methods; /* +0x000 */
};

/*
 * Obj86B60__Init's 2nd parameter (round 13) -- a small "init args" struct:
 * two children forwarded to the inherited BasicClass addChild (self->
 * methods->slot10), and three optional fields each read with a "use if
 * set, else derive from a helper call" idiom mirroring self->unk10/unk14/
 * unk18's own construction. Only the five fields Obj86B60__Init touches are
 * modelled.
 */
struct Obj86B60InitArgs {
    Unk0ArgObj *unk0; /* +0x000, forwarded to self->methods->slot10 (child)
                          as `void *`; ALSO OBSERVED (round 13) dereferenced
                          directly by Obj86B60__NotifyChildReset as a real class instance
                          -- see Unk0ArgObj's own comment */
    Unk4ArgObj *unk4; /* +0x004, forwarded to self->methods->slot10 (child)
                          as `void *`; ALSO OBSERVED (round 13) dereferenced
                          directly by Obj86B60__OnTag1Notify as a real class instance
                          -- see Unk4ArgObj's own comment */
    void *unk8;      /* +0x008, fallback source for self->unk10 */
    void *unkC;      /* +0x00C, fallback source for self->unk14 */
    Unk18Obj *unk10; /* +0x010, fallback source for self->viewport */
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
    void (*addChild)(Obj86B60 *self, void *a1);     /* +0x010, inherited
                                                      BasicClass addChild
                                                      (BasicClass__AddChild);
                                                      OBSERVED: Obj86B60__Init.
                                                      Renamed from slot10,
                                                      round 55 -- exclusive to
                                                      this unit (no other
                                                      code_2cc8c sibling
                                                      dispatches through
                                                      Obj86B60Methods at this
                                                      offset). */
    void (*removeChild)(Obj86B60 *self, void *a1);     /* +0x014, inherited
                                                      BasicClass removeChild
                                                      (BasicClass__RemoveChild);
                                                      OBSERVED: Obj86B60__Deinit
                                                      (round 13). Renamed from
                                                      slot14, round 55, same
                                                      exclusivity as addChild
                                                      above. */
    u8 pad018[0x030 - 0x018];
    void (*slot30)(Obj86B60 *self);               /* +0x030, inherited
                                                      BasicClass slot
                                                      (BasicClass__NotifyParents);
                                                      OBSERVED: Obj86B60__NotifyParents
                                                      (round 13) */
    u8 pad034[0x040 - 0x034];
    void (*resetCounters)(Obj86B60 *self);               /* +0x040, IS
                                                      Obj86B60__ResetCounters (already
                                                      matched); OBSERVED:
                                                      IntermediateBase__IntermediateBase.
                                                      Renamed from slot40,
                                                      round 55 -- exclusive to
                                                      this unit. */
    u8 pad044[0x048 - 0x044];
    void (*deinit)(Obj86B60 *self);               /* +0x048, IS
                                                      Obj86B60__Deinit;
                                                      OBSERVED: Obj86B60__Init.
                                                      Renamed from slot48,
                                                      round 55, same
                                                      exclusivity as
                                                      resetCounters above. */
    void (*slot4C)(Obj86B60 *self, s32 a1, s32 a2, s32 a3); /* +0x04C,
                                                      external (TaskCoreObj__func_8003C238);
                                                      OBSERVED: Obj86B60__Init */
    void (*slot50)(Obj86B60 *self);               /* +0x050, external
                                                      (func_8004D898);
                                                      OBSERVED: Obj86B60__Deinit
                                                      (round 13) */
    void (*onTag1Notify)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x054, IS
                                                      Obj86B60__OnTag1Notify;
                                                      OBSERVED: Obj86B60__OnNotify.
                                                      Renamed from slot54,
                                                      round 55 -- exclusive to
                                                      this unit. */
    void (*slot58)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x058,
                                                      external (func_8003C48C,
                                                      STALL in code_2cc8c);
                                                      OBSERVED: Obj86B60__OnNotify */
    void (*slot5C)(Obj86B60 *self, EventArg *arg1, s32 arg2); /* +0x05C, IS
                                                      func_8003C51C (already
                                                      matched there with a1
                                                      typed s32 -- an
                                                      independent local view,
                                                      same shared slot);
                                                      OBSERVED: Obj86B60__OnNotify */
    void (*slot60)(Obj86B60 *self, s32 reason);  /* +0x060, external
                                                      (func_8004D90C) */
    void (*slot64)(Obj86B60 *self);               /* +0x064, IS
                                                      Obj86B60__NotifyTargetReset (already
                                                      matched); OBSERVED:
                                                      Obj86B60__NotifyParents
                                                      (round 13) */
    void (*slot68)(Obj86B60 *self);               /* +0x068, IS
                                                      Obj86B60__NotifyChildReset;
                                                      OBSERVED: Obj86B60__NotifyParents
                                                      (round 13) */
    u8 pad06C[0x070 - 0x06C];
    void (*slot70)(Obj86B60 *self, s32 a1);       /* +0x070, IS
                                                      func_8003C7B4 */
    /* +0x074..+0x084: the five message handlers func_8003C48C dispatches to
     * (round 23). Read off jtbl_80011090: message code 0x12 -> slot80,
     * 0x13 -> slot84, 0x17 -> slot7C, 0x19 -> slot78, 0x21 -> slot74; every
     * other code in [0x12,0x21] is a no-op. In the base table gClass86B60Methods
     * these are func_8003C7F4 / func_8003C858 / func_8003C8D0 /
     * func_8003C944 / func_8003C9B0 respectively. Pad split is ADDITIVE and
     * preserves the original 0x1C total (5 * 4 + 8). */
    void (*slot74)(Obj86B60 *self, s32 a1);       /* +0x074, IS func_8003C9B0 */
    void (*slot78)(Obj86B60 *self, s32 a1);       /* +0x078, IS func_8003C944 */
    void (*slot7C)(Obj86B60 *self, s32 a1);       /* +0x07C, IS func_8003C8D0 */
    void (*slot80)(Obj86B60 *self, s32 a1);       /* +0x080, IS func_8003C7F4 */
    void (*slot84)(Obj86B60 *self, s32 a1);       /* +0x084, IS func_8003C858 */
    u8 pad088[0x090 - 0x088];
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
    void (*slot10C)(Obj86B60 *self);                /* +0x10C, external
                                                       (func_8003DAD4).
                                                       RETYPED s32 -> void,
                                                       round 23: the s32 was
                                                       read off func_8003C63C's
                                                       disassembly while that
                                                       function was UNATTEMPTED,
                                                       and a discarded return
                                                       value is invisible in
                                                       the bytes. Matching
                                                       func_8003C63C requires
                                                       void -- see that
                                                       report's tail-merge
                                                       finding. OBSERVED:
                                                       func_8003C63C */
    void (*slot110)(Obj86B60 *self);                /* +0x110, external
                                                       (func_8003DCAC).
                                                       RETYPED s32 -> void,
                                                       round 23, on positive
                                                       evidence independent of
                                                       that match: the occupant
                                                       func_8003DCAC is ALREADY
                                                       MATCHED in
                                                       src/code_2cc8c_b.c as
                                                       `void func_8003DCAC(
                                                       Obj86B60 *self)`.
                                                       OBSERVED: func_8003C63C */
    void (*slot114)(Obj86B60 *self);                /* +0x114, external
                                                       (func_8003DDC8);
                                                       OBSERVED: func_8003C9B0 */
    s32 (*slot118)(Obj86B60 *self);                 /* +0x118. CORRECTED
                                                       (round 12, runner
                                                       alpha): the occupant
                                                       is func_8003DE30, NOT
                                                       Obj86B60__GetActiveSlotCount as this
                                                       comment previously
                                                       said -- verified by
                                                       reading the raw table
                                                       bytes at
                                                       gClass86B60Methods+0x118 in
                                                       disk/SLPS_015.56
                                                       directly (also
                                                       cross-checked with
                                                       tools/classtable.py
                                                       gClass86B60Methods). The old
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
                                                       IS Obj86B60__GetActiveSlotCount
                                                       (verified the same
                                                       way as slot118 above;
                                                       `Obj86B60__GetActiveSlotCount` itself
                                                       returns
                                                       `self->unk60[self->
                                                       unk58]`, s32) */
};

/*
 * self->initArgs's pointee (field renamed from unkC round 55), observed only
 * by Obj86B60__NotifyTargetReset -- a shared
 * base-class method also reachable through UNRELATED classes' own vtables
 * at this same slot offset (class_39e08.h documents Obj86B60__NotifyTargetReset/78
 * occupying Obj865C8Methods/Class86668Methods +0x064/+0x068). Dispatch
 * shape: `self->initArgs->target->methods->slot48(target)` -- one extra level
 * of indirection past the usual `self->fieldN->methods->slotM(self->fieldN)`
 * idiom. Only the one field/slot Obj86B60__NotifyTargetReset touches is modelled.
 */
typedef struct Obj86B60UnkC Obj86B60UnkC;
typedef struct Obj86B60UnkCTarget Obj86B60UnkCTarget;
typedef struct Obj86B60UnkCTargetMethods Obj86B60UnkCTargetMethods;
struct Obj86B60UnkCTargetMethods {
    u8 pad000[0x048];
    void (*slot48)(Obj86B60UnkCTarget *self); /* +0x048, OBSERVED: Obj86B60__NotifyTargetReset */
};
struct Obj86B60UnkCTarget {
    Obj86B60UnkCTargetMethods *methods; /* +0x000 */
};
struct Obj86B60UnkC {
    Obj86B60UnkCTarget *target; /* +0x000, OBSERVED: Obj86B60__NotifyTargetReset */
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
    Obj86B60UnkC *initArgs;      /* +0x00C, renamed from unkC round 55 --
                                    Obj86B60__Init sets it to `(Obj86B60UnkC *)arg1`,
                                    i.e. it is literally the retained
                                    Obj86B60InitArgs pointer the object was
                                    constructed with, re-viewed through the
                                    Obj86B60UnkC local type wherever only its
                                    `target` field is needed (tier A: exclusive
                                    to this unit, no other code_2cc8c sibling
                                    reaches this offset on an Obj86B60*).
                                    OBSERVED: Obj86B60__NotifyTargetReset (see
                                    Obj86B60UnkC's own comment): `self->initArgs->target
                                    ->methods->slot48(target)`. Also zeroed by
                                    Obj86B60__ResetAndRemoveAllChildren (a ctor-shaped function that
                                    also zeroes unk10/unk30 below). NOTE: this
                                    slot is reached through a SHARED base-class
                                    method -- class_39e08.h's own view of an
                                    unrelated class documents the same
                                    Obj86B60__NotifyTargetReset occupying its own vtable at
                                    the identical offset (+0x064), so this
                                    field is very likely part of a common
                                    base-object layout every subclass shares
                                    at this offset, not something Obj86B60
                                    itself introduces -- kept here anyway,
                                    per this header's flat single-struct
                                    style (no explicit base/derived split). */
    s32 unk10;                  /* +0x010, Obj86B60__ResetAndRemoveAllChildren: zeroed by the same
                                    ctor-shaped function as initArgs/unk30.
                                    ALSO OBSERVED (round 13) by Obj86B60__Init,
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
                                    ALSO OBSERVED (round 13) by Obj86B60__Init,
                                    which both sets it (from an init-args
                                    field or a helper call) and, on one path,
                                    dispatches through it as a pointer to an
                                    object with its own vtable (cast locally
                                    to `Unk14Obj *`, see that type's own
                                    comment) -- kept `s32` here since that is
                                    still the more general of the two
                                    observed readings. */
    Unk18Obj *viewport;          /* +0x018, renamed from unk18 round 55 --
                                    tier B, exclusive to this unit: holds a
                                    Unk18Obj instance, and Unk18Obj itself
                                    manages a 3-word position/rotation pair
                                    (its own unk14/unk20, both Vec3) and
                                    calls GsSetRefView2 (code_2cc8c_d.c,
                                    Unk18Obj__AttachViewChild) -- a PSX GPU "set
                                    reference viewport" call -- so the
                                    pointee is a camera/viewport object, even
                                    though what specifically the *game*
                                    uses it for is not established. Set by
                                    Obj86B60__Init (round 13): set
                                    from an init-args field or from this
                                    unit's own New_X allocator
                                    (New_Unk18Obj), then dispatched through
                                    (`self->viewport->methods->slot10(...)`) */
    s32 frameCounter;                  /* +0x01C, func_8003CC2C (a running count/
                                    frame value multiplied against unk84);
                                    func_8003C63C (STALL) zeroes it on
                                    several message codes. PROPOSED (round 55,
                                    tier B): unk1C -> frameCounter -- shared
                                    with code_2cc8c.c (func_8003C63C,
                                    func_8003CC2C, func_8003CBB8), so not
                                    renamed here; see this unit's
                                    "## Proposed field names". */
    u8 pad020[0x020 - 0x020];
    s32 unk20;                  /* +0x020, func_8003C63C (STALL) sets it
                                    to a literal 5 */
    s32 initMode;               /* +0x024, renamed from unk24 round 55 --
                                    tier B, exclusive to this unit.
                                    Obj86B60__Init (round 13): set to
                                    arg2 (also gates the rest of that
                                    function's body, and of Obj86B60__Deinit,
                                    on == 0) */
    u8 pad028[0x030 - 0x028];
    s32 unk30;                  /* +0x030, Obj86B60__ResetAndRemoveAllChildren only: zeroed by the
                                    same ctor-shaped function as initArgs/unk10;
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
    s32 activeSlot;                  /* +0x058, func_8003CA1C: index into
                                    unk4C->unk24[] and compared against
                                    unk4C->unkC */
    s32 *unk5C;                  /* +0x05C, array indexed by unk58: a
                                     per-slot capacity/bound.
                                     func_8003D6D4 passes unk5C[unk58] as
                                     ReleaseBasicClassArray's 2nd arg (raw register,
                                     type doesn't affect those bytes);
                                     func_8003DDC8/func_8003DE30 use it as
                                     an explicit upper bound compared
                                     against unk60[unk58], which is what
                                     settles it as a count, not a pointer */
    s32 *slotCounts;                   /* +0x060, array indexed by unk58: a
                                     per-slot running count, incremented
                                     (wrapping to 0 past unk5C[unk58]) by
                                     func_8003DDC8 and decremented
                                     (wrapping to unk5C[unk58]-1 below 0) by
                                     func_8003DE30 -- a ring-buffer index */
    void **unk64;                /* +0x064, func_8003D6D4: array indexed by
                                     unk58, giving ReleaseBasicClassArray's 1st arg
                                     and BMemPMgrFree's arg */
    Unk68Obj *unk68;             /* +0x068, OBSERVED: func_8003D050,
                                     func_8003DAD4, func_8003D73C (round 12)
                                     -- built once by func_8003CE98 via
                                     New_ClassEAC0(&D_8008A8E8, &D_8008A8F0,
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
 * Shared "IntermediateBase" utility class, reached through Get_vtable_IntermediateBase().
 * UPDATED (round 12, runner alpha): Get_vtable_IntermediateBase has now been carved into
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

extern IntermediateBaseMethods *Get_vtable_IntermediateBase(void); /* returns &gIntermediateBaseMethods,
                                                          same static table
                                                          as code_2c054.h's
                                                          and class_39e08.h's
                                                          own views */
extern IntermediateBaseMethods gIntermediateBaseMethods; /* the table itself, so
                                                Get_vtable_IntermediateBase's own
                                                definition (code_2cc8c_c.c)
                                                can return &gIntermediateBaseMethods */

/*
 * This unit's own local view of the shared BasicClass ancestor table
 * (returned by Get_vtable_BasicClass, a no-argument getter -- same "ctor at
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
                                  (code_8220.c); OBSERVED: IntermediateBase__IntermediateBase */
    void (*finalize)(void *self); /* +0x00C, IS BasicClass__Finalize
                                  (code_8220.c, "finalize"); OBSERVED:
                                  Unk18Obj__Finalize (round 13). Renamed from
                                  slot0C, round 55 -- exclusive to this unit
                                  (only code_2cc8c_c.c calls
                                  Get_vtable_BasicClass() in the code_2cc8c
                                  family). */
    void (*addChild)(void *self, void *child); /* +0x010, IS
                                  BasicClass__AddChild (code_8220.c,
                                  "addChild"); OBSERVED: Unk18Obj__AddChild
                                  (round 13). Renamed from slot10, round 55,
                                  same exclusivity as finalize above. */
    void (*removeChild)(void *self, void *child); /* +0x014, IS
                                  BasicClass__RemoveChild (code_8220.c,
                                  "removeChild"); OBSERVED: Unk18Obj__RemoveChild
                                  (round 13). Renamed from slot14, round 55,
                                  same exclusivity as finalize above. */
    void (*removeAllChildren)(void *self); /* +0x018, Obj86B60__ResetAndRemoveAllChildren's
                                  forward target. Renamed from slot18, round 55,
                                  matches the canonical BasicClassMethods'
                                  own name at this exact offset
                                  (include/code_8220.h). Same exclusivity as
                                  finalize above. */
    u8 pad01C[0x038 - 0x01C];
    void (*onNotify)(void *self, void *arg1, s32 arg2); /* +0x038, IS
                                  BasicClass__OnNotify (code_8220_b);
                                  OBSERVED: Obj86B60__OnNotify. NOT renamed:
                                  code_2cc8c_d.c's Unk18Obj__OnNotify also
                                  dispatches through this exact slot (its
                                  own Get_vtable_BasicClass() call), so this
                                  field is shared -- PROPOSED (round 55,
                                  tier A): slot38 -> onNotify, matching
                                  BasicClassMethods' own canonical name at
                                  this offset (include/code_8220.h). Head
                                  applies by type scope. */
};

extern BasicClassMethodsCC8C *Get_vtable_BasicClass(void);

/*
/*
 * HEAD NOTE, round 14: the two class views below model the SAME table
 * family, from two units, and both are kept deliberately.
 *
 * `Obj6EAC0` (runner bravo, code_2cc8c_f) and `ClassEAC0Obj` (runner alpha,
 * code_2cc8c_e) are independent local views of D_8006EAC0 and its siblings.
 * They were derived in parallel, from different call sites, and each is
 * depended on by its own unit's ALREADY-MATCHED code -- so neither can be
 * dropped without re-verifying the other unit's byte-exact functions.
 *
 * This is the project's multiple-independent-local-views convention landing
 * inside ONE header, which is unusual and is normally the thing to unify.
 * It is not unified here because unification would edit matched code for a
 * naming benefit, and matched code is the thing this project exists to
 * protect. Whoever next works either unit should unify them -- alpha's view
 * carries the deeper inheritance chain (Class6B5CCObj -> ClassEAC0 ->
 * Class6E99C, established via classtable.py against D_8006B58C) and bravo's
 * carries the per-call-site slot arity, so the union of the two is strictly
 * better than either.
 */

/*
 * A previously-unnamed BasicClass-derived class family, round 14
 * (code_2cc8c_f): base table D_8006EAC0 ("d") and its override table
 * D_8006EB90, resolved with `tools/classtable.py D_8006EAC0 --vs
 * D_8006EB90`. A third sibling table, D_8006EC74 (returned by the
 * external getter func_80041C3C), shares the identical slot layout and
 * is reached only through that getter, never dereferenced by address
 * here. No FirecatFG name survives; named `Obj6EAC0` after the base
 * table's own address, per this project's naming-by-table-address
 * convention (see `Obj86B60` above). Per the multiple-independent-
 * local-views convention, this is THIS unit's own view. Only the
 * slots/fields this unit's 26 non-trivial functions actually touch are
 * modelled; every gap stays opaque padding.
 *
 * Slot arity is per-CALL-SITE, not per-slot: `slot4C` and `slotC4` are
 * both called elsewhere in this unit with FEWER arguments than the
 * struct's own declared type, which is the project's established
 * "per-call-site convention" (see docs/DECOMPILATION_LEARNINGS.md) --
 * narrower call sites cast the slot to a narrower function-pointer type
 * rather than widening every call to match one struct-wide signature.
 *
 * NAMED round 54 (runner alpha, track 3). Struct name kept as `Obj6EAC0`
 * (no independent class identity established; renaming it would also
 * collide with the still-unresolved `ClassEAC0Obj` dual-view note above,
 * which is a bigger, separate merge). Every FIELD/METHOD name below is
 * new this round; see each function's own `docs/match-reports/*.md` for
 * the evidence trail. The composite picture that came out of naming this
 * unit's own functions (tier B, not independently confirmed against any
 * other unit): this looks like a small on-screen TEXT/DIGIT DISPLAY --
 * `hasChildren`==0 instances are single-character leaf glyphs (`SetChar`),
 * `hasChildren`!=0 instances are containers holding a `children` array of
 * more `Obj6EAC0`s laid out along one axis (`posX`/`posY` as a running
 * cursor, advanced by `childPitch` per child, with one extra +0x10 gap
 * inserted at `gapIndex` -- plausibly a decimal-point/separator slot).
 * The evidence: `New_Obj6EAC0(ctx, count, text)` builds an N-child
 * instance and its `text` argument flows straight through construction
 * into `Obj6EAC0__SetText`, which walks a NUL-terminated byte string
 * dispatching one child per character; and this SAME unit's
 * `FormatFullWidthNumber`/`EncodeFullWidthSjis` (unrelated free functions
 * that happen to live in this file, confirmed by their OWN callers
 * elsewhere to take a plain buffer, not an `Obj6EAC0 *`, despite sharing
 * this file's dominant `self`-typed signature style) build exactly the
 * kind of zero-padded, Shift-JIS-encoded digit string this class's own
 * `SetText` would consume. No caller outside this unit constructs or
 * touches an `Obj6EAC0` (confirmed: `grep -rl Obj6EAC0 src/*.c` finds only
 * this file and one dead comment in `class_3bb8c_c.c`), so this reading
 * is internally consistent but not cross-checked against any other unit's
 * independent evidence -- treat "text/digit display" as the working
 * hypothesis this unit's own functions all agree with, not a confirmed
 * fact. `unk44`/`unk48`/`unk4C`/`unk60`/`unk62` are left unrenamed:
 * nothing in this unit's functions gives them a purpose beyond "a stored
 * word"/"a stored halfword" (see their own field comments below).
 */
typedef struct Obj6EAC0 Obj6EAC0;
typedef struct Obj6EAC0Methods Obj6EAC0Methods;
struct Obj6EAC0Methods {
    u8 pad000[0x008];
    void (*slot08)(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3); /* +0x008,
                                  ctor-shaped: OBSERVED forwarded 3 raw
                                  args by New_Obj6EAC0's New_X wrapper.
                                  IS Obj6EAC0__Construct (this unit, STALL) in
                                  the derived table. */
    void (*slot0C)(Obj6EAC0 *self); /* +0x00C, IS Obj6EAC0__Destruct (derived,
                                  this unit) -- takes no extra args */
    u8 pad010[0x040 - 0x010];
    void (*slot40)(Obj6EAC0 *self, s32 a1); /* +0x040, IS Obj6EAC0__FinishConstruct
                                  (derived, this unit) */
    u8 pad044[0x04C - 0x044];
    void (*slot4C)(); /* +0x04C, DELIBERATELY UNPROTOTYPED (K&R style):
                                  call sites in this unit need it at BOTH
                                  3 and 4 explicit arguments
                                  (Obj6EAC0__SetChar forwards 4;
                                  Obj6EAC0__LayoutChildrenWithGap calls it at 3, twice, with
                                  different argument MEANINGS each time)
                                  and C requires an exact arg-count match
                                  through a prototyped function-pointer
                                  type, which no single prototype here
                                  could satisfy. IS Obj6EAC0__Layout (base,
                                  this unit, reads 3) and Obj6EAC0__LayoutChildrenWithGap
                                  (derived, this unit, reads 3) */
    void (*slot50)(Obj6EAC0 *self); /* +0x050, IS func_80040C00 (derived,
                                  this unit) */
    u8 pad054[0x060 - 0x054];
    s32 (*slot60)(Obj6EAC0 *self, s32 a1); /* +0x060, IS Obj6EAC0__QueryChildren
                                  (derived, this unit), which recurses
                                  into a child's own slot60 with the same
                                  a1 and threads the return value through
                                  as its own return (last-iteration wins) */
    s32 (*slot64)(Obj6EAC0 *self, s32 a1); /* +0x064, IS func_80040714
                                  (base, this unit) -- tail-returns a
                                  packed-bitfield accessor */
    s32 (*slot68)(Obj6EAC0 *self, s32 a1); /* +0x068, IS func_80040740
                                  (base, this unit), same shape as
                                  slot64 */
    u8 pad06C[0x0B8 - 0x06C];
    void (*slotB8)(Obj6EAC0 *self, s32 a1); /* +0x0B8, the only OBSERVED
                                  CALL through this slot is
                                  Obj6EAC0__PropagateColor's own child dispatch, at
                                  2 args. Obj6EAC0__SetColor (base occupant)
                                  takes a 3rd (`u8 *src`) in its own
                                  definition, which is fine -- an
                                  occupant's own arity need not match a
                                  narrower call site (nothing in this
                                  unit calls slotB8 at 3 args) */
    void (*slotBC)(Obj6EAC0 *self, Pair32E99C *a1); /* +0x0BC, IS Obj6EAC0__SetPosition
                                  (base, this unit) and Obj6EAC0__LayoutChildren
                                  (derived, this unit); a1 a 2-word
                                  struct pointer in both -- same shape as
                                  Class6E99CObj's own Pair32E99C (see
                                  Class6E99C__PushPosition) */
    void (*slotC0)(Obj6EAC0 *self, void *a1); /* +0x0C0, IS func_80040824
                                  (base, this unit); a1 a 2-halfword
                                  struct pointer */
    void (*slotC4)(); /* +0x0C4, DELIBERATELY UNPROTOTYPED, same reason as
                                  slot4C above: Obj6EAC0__SetChar forwards 4
                                  args, Obj6EAC0__SetText dispatches a CHILD's
                                  slotC4 at only 2. IS Obj6EAC0__SetChar
                                  (base, this unit, reads 3) and
                                  Obj6EAC0__SetChildChar (derived, this unit,
                                  reads 2) */
    void (*slotC8)(Obj6EAC0 *self, s32 a1); /* +0x0C8, IS func_800408A0
                                  (base, this unit, setter) and
                                  Obj6EAC0__NoOpSetter (derived, this unit,
                                  splat-generated trivial jr $ra; nop) */
    s32 (*slotCC)(Obj6EAC0 *self, s32 a1); /* +0x0CC, IS Obj6EAC0__SetMask
                                  (base, this unit) and Obj6EAC0__SetText
                                  (derived, this unit) */
    void (*slotD0)(Obj6EAC0 *self); /* +0x0D0, derived-only, IS
                                  Obj6EAC0__NoOpSlotD0 (this unit, splat-
                                  generated trivial) */
    void (*slotD4)(Obj6EAC0 *self, s32 a1); /* +0x0D4, derived-only, IS
                                  Obj6EAC0__SetChildPitch (this unit, setter) */
};

struct Obj6EAC0 {
    Obj6EAC0Methods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    s32 hasChildren;           /* +0x00C, OBSERVED: an enable/child-count
                                  gate tested by Obj6EAC0__Layout,
                                  Obj6EAC0__LayoutChildrenWithGap, func_80040C00,
                                  Obj6EAC0__LayoutChildren */
    u8 pad010[0x044 - 0x010];
    s32 unk44;                 /* +0x044, OBSERVED: func_800408A0 (setter) */
    s32 unk48;                 /* +0x048, OBSERVED: zeroed by Obj6EAC0__SetChar */
    s32 unk4C;                 /* +0x04C, OBSERVED: Obj6EAC0__SetChar (setter,
                                  from its own a3) */
    s32 posX;                  /* +0x050, OBSERVED: Obj6EAC0__SetPosition/
                                  Obj6EAC0__LayoutChildren (slotBC occupants) --
                                  first word of a 2-word struct copied
                                  from their own `a1` argument */
    s32 posY;                  /* +0x054, OBSERVED: ditto, second word */
    u32 flags;                 /* +0x058, OBSERVED: a packed-bitfield word,
                                  passed as `GetSetBitField(&self->flags, ...)`
                                  -- same generic accessor as
                                  include/code_d294.h's `unk10` */
    u8 pad05C[0x060 - 0x05C];
    s16 unk60;                 /* +0x060, OBSERVED: func_80040824 (slotC0
                                  occupant) -- first halfword of a
                                  2-halfword struct copied from its own
                                  `a1` argument */
    s16 unk62;                 /* +0x062, OBSERVED: ditto, second halfword */
    u8 color[3];                /* +0x064, OBSERVED: Obj6EAC0__SetColor -- a
                                  3-byte colour buffer, overwritten or
                                  added-into via Obj6EAC0__ApplyColor */
    s32 mask;                  /* +0x068, OBSERVED: Obj6EAC0__SetMask (setter,
                                  a `(1 << a1) - 1` bitmask) */
    u8 pad06C[0x0A9 - 0x06C];
    u8 totalChildCount;         /* +0x0A9, OBSERVED: Obj6EAC0__Destruct (passed
                                  as ReleaseBasicClassArray's count arg),
                                  Obj6EAC0__LayoutChildren (loop bound) */
    u8 gapIndex;                /* +0x0AA, OBSERVED: Obj6EAC0__LayoutChildrenWithGap -- a
                                  one-shot "extra offset" gate compared
                                  against the loop index */
    u8 childCount;               /* +0x0AB, OBSERVED: a per-slice element
                                  COUNT, paired with childStart as the base
                                  index -- Obj6EAC0__LayoutChildrenWithGap, func_80040C00,
                                  Obj6EAC0__QueryChildren, Obj6EAC0__PropagateColor */
    u8 childStart;               /* +0x0AC, OBSERVED: a per-slice element
                                  START INDEX into children, paired with
                                  childCount above */
    u8 padAD[0x0B0 - 0x0AD];
    s32 childPitch;             /* +0x0B0, OBSERVED: Obj6EAC0__SetChildPitch (setter);
                                  read and added into a local running total
                                  by Obj6EAC0__LayoutChildrenWithGap/Obj6EAC0__LayoutChildren */
    Obj6EAC0 **children;        /* +0x0B4, OBSERVED: an array of child
                                  objects of this SAME class, indexed by
                                  childStart..childStart+childCount and dispatched
                                  through their own `->methods` */
};

/* Same generic packed-bitfield-word accessor documented in
 * include/code_d294.h (`u32 GetSetBitField(u32 *word, s32 shift, s32
 * width, u32 value)`), reached here over `&self->flags` instead of
 * `&self->unk10`. Declared again here under this unit's own local view
 * per the established multiple-independent-local-views convention. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

extern Obj6EAC0Methods D_8006EAC0; /* the base table itself, so
                                       Obj6EAC0__GetBaseMethods's own definition
                                       (this unit) can return &D_8006EAC0 */
extern Obj6EAC0Methods D_8006EB90; /* the override table itself, so
                                       Obj6EAC0__GetDerivedMethods's own definition
                                       (this unit) can return &D_8006EB90 */
extern Obj6EAC0Methods *func_80041C3C(void); /* returns &D_8006EC74, a
                                       third sibling table sharing this
                                       layout; external to this unit
                                       (asm/psyq_memset.s @ 0x80041C3C,
                                       almost certainly misclassified
                                       game code rather than real SDK --
                                       see the open question in
                                       DECOMPILATION_LEARNINGS about the
                                       psyq_memset boundary) */

extern Obj6EAC0 *func_80041AB4(s32 a1, s32 a2); /* another New_X-shaped
                                       allocator over this same class
                                       family (0xAC bytes, ctor via
                                       func_80041C3C()->slot08, "return-
                                       regardless" variant); external to
                                       this unit (asm/psyq_memset.s @
                                       0x80041AB4); OBSERVED:
                                       Obj6EAC0__Construct */

/*
 * MEASURED elsewhere (round 9, include/class_3bb8c.h / src/class_3ac78.c):
 * GetClass6B5CCMethods takes NO arguments and its whole body is a fixed
 * `lui/addiu %hi/%lo(gClass6B5CCMethods); jr $ra` -- it always returns the SAME
 * global table regardless of caller, a shared "default handler" utility
 * reached the same way IntermediateBaseMethods/TaskUtilMethods are
 * reached elsewhere in this project. This unit's own call touches only
 * slot 0x04C, at a NARROWER 2-extra-argument arity than
 * Obj6EAC0Methods::slot4C's widest use, so it gets its own tiny local
 * view rather than reusing that struct (same per-call-site-arity
 * reasoning as slot4C/slotC4 above). Do not reconcile this declaration
 * with code_d294.h's or class_3ac78.h's own differently-typed views of
 * the same symbol -- see class_3ac78.c's comment on GetClass6B5CCMethods for
 * why that is expected.
 */
/* GetClass6B5CCMethods's return type, UNIFIED by the head at merge time. Runners
 * alpha and bravo each built a local view of it in the same round, with
 * different names AND different members -- bravo's had only `slot4C` at
 * +0x04C, alpha's only `ctor` at +0x008 -- which collided as `conflicting
 * types` the moment both landed in this one header. Merged here at the
 * correct offsets rather than picked between, so both units' call sites keep
 * working. The canonical definition is src/code_d294_b.c's
 * `Class6B5CCMethods *GetClass6B5CCMethods(void)`. */
typedef struct D6B5CCGetterMethodsCC8C D6B5CCGetterMethodsCC8C;
struct D6B5CCGetterMethodsCC8C {
    u8 pad000[0x008];
    void *(*ctor)(void *self);                      /* +0x008, code_2cc8c_e */
    u8 pad00C[0x04C - 0x00C];
    void (*slot4C)(Obj6EAC0 *self, s32 a1, s32 a2); /* +0x04C, code_2cc8c_f */
};
extern D6B5CCGetterMethodsCC8C *GetClass6B5CCMethods(void);

/*
 * Round 14 (code_2cc8c_e): a THIRD independent class pair, found while
 * carving the segment's remaining 60-function tail. `tools/classtable.py
 * D_8006E99C --vs D_8006B58C` shows D_8006E99C shares its dtor (+0x00C)
 * and three more slots (+0x010/+0x014/+0x018) plus all seven BasicClass-
 * inherited slots (+0x01C..+0x038) with gClass6B5CCMethods (code_d294.h's own
 * `Class6B5CCMethods`) -- the same base-class fingerprint code_d294.h
 * already established, so D_8006E99C is a Class6B5CCObj descendant. It is
 * NOT a direct child, though: its own ctor (Class6E99C__Class6E99C, this unit)
 * calls `Obj6EAC0__GetBaseMethods()->ctor(self, a1, a2, a3)` before overwriting
 * `self->methods` with `&D_8006E99C` and re-dispatching through it --
 * exactly the established "base ctor first, then set own vtable pointer,
 * then dispatch through it" idiom (see e.g. Class86B60__Class86B60's entry in
 * DECOMPILATION_LEARNINGS). `Obj6EAC0__GetBaseMethods` (code_2cc8c_f, bravo's own
 * function) is a bare no-argument getter for a SECOND table, D_8006EAC0
 * -- itself sharing the identical fingerprint with gClass6B5CCMethods, so the
 * real chain is Class6B5CCObj -> "ClassEAC0" -> "Class6E99C". Two
 * `New_X`-shaped allocators confirm the two concrete sizes: New_Class6E99C
 * allocates 0xA0 bytes for a Class6E99C instance (getting its own table
 * via GetClass6E99CMethods, a bare getter this unit also implements) and
 * New_ClassEAC0 allocates a SMALLER 0x6C bytes for a bare ClassEAC0
 * instance (getting D_8006EAC0 via Obj6EAC0__GetBaseMethods) -- consistent with
 * ClassEAC0 being the smaller, less-derived class. ClassEAC0__ClassEAC0 is
 * ClassEAC0's OWN ctor, sharing the identical "call a further-base ctor,
 * reset methods, redispatch slot40" shape one level up: it calls
 * `GetClass6B5CCMethods()->ctor(self)` (GetClass6B5CCMethods, code_d294.h's own getter
 * for the ACTUAL Class6B5CCObj table, gClass6B5CCMethods) first.
 *
 * Per this project's established multiple-independent-local-views
 * convention, these are THIS unit's own flat views -- no attempt is made
 * to literally embed Class6B5CCObj (code_d294.h) as a C base member, since
 * that unit only models fields up to +0x030 and the real extent of either
 * class here is unknown past what this unit's own functions touch. Only
 * the slots/fields this unit's functions actually reach are typed; the
 * rest stays opaque padding. Field names are offset-based
 * (`unkNN`) until real names are known.
 */
typedef struct ClassEAC0Methods ClassEAC0Methods;
typedef struct Class6E99CMethods Class6E99CMethods;

/* A small two-value record read only via 16-bit loads at a 4-byte stride
 * (offsets +0x000/+0x004, not +0x000/+0x002) -- Class6E99C__PushPosition's own `a1`
 * argument. The 4-byte spacing between two 2-byte reads means the real
 * source struct has an untouched field in between (or after); not
 * modelled further since nothing here reads it. */
typedef struct SkipShort2 SkipShort2;
struct SkipShort2 {
    s16 x;             /* +0x000 */
    u8 pad2[0x004 - 0x002];
    s16 y;             /* +0x004 */
};

/* A small "shift/stride/width/height" texture-page-like descriptor --
 * func_8003FC18's own 3rd argument (round 14, code_2cc8c_e). Only the
 * fields that function touches are named. (typedef forward-declared near
 * the top of this file, see there.) */
struct TexPageDesc {
    s32 shift;   /* +0x000 */
    s32 stride;  /* +0x004 */
    s32 width;   /* +0x008 */
    s32 height;  /* +0x00C */
    s32 size;    /* +0x010, computed = (4 << shift) + stride - 4 */
};

/* A small two-`s32` record -- Class6E99C__PushPosition's own `a2` argument, read as a
 * plain consecutive pair and copied wholesale into the object's own
 * unk50/unk54. Forward-typedef'd at the top of this file since
 * Obj6EAC0Methods::slotBC (below) needs the name before this body is
 * seen -- see that forward-typedef block's own comment. */
struct Pair32E99C {
    s32 a; /* +0x000 */
    s32 b; /* +0x004 */
};

struct ClassEAC0Methods {
    s32 header;                                        /* +0x000 */
    void *unk04;                                        /* +0x004, BasicClass__Release, inherited, unused here */
    void (*ctor)(ClassEAC0Obj *self, void *a1, void *a2, s32 a3); /* +0x008,
                                ClassEAC0__ClassEAC0 (this unit). `a1`/`a2` kept as
                                plain `void *` here (not `SkipShort2 *`) --
                                this is the SLOT's own type, used by every
                                CALLER of the ctor through the vtable
                                (New_Class6E99C/Class6E99C__Class6E99C/New_ClassEAC0,
                                none of which know about `SkipShort2`); the
                                occupant's own definition is free to use a
                                more specific parameter type internally. */
    void (*dtor)(ClassEAC0Obj *self);                   /* +0x00C, Class6B5CC__Finalize, shared with Class6B5CCMethods */
    u8 pad010[0x040 - 0x010];
    void (*finishConstruct)(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3); /* +0x040, ClassEAC0__FinishConstruct (this unit).
                                RENAMED round 61 (was slot40). */
    u8 pad044[0x0B8 - 0x044];
    /* +0x0B8, OBSERVED (this unit, ClassEAC0__FinishConstruct/Class6E99C__StartFadeToIndex/
       Class6E99C__StartFadeDefault): dispatched as `(self, 1, tableEntry)` where
       `tableEntry` is a computed address into D_8006EA90 (indexed) or the
       fixed D_8006EAA8/D_8008A924. Occupant `Obj6EAC0__SetColor`
       (code_2cc8c_f, bravo's own function) -- not this unit's to type
       further. */
    void (*slotB8)(ClassEAC0Obj *self, s32 a1, void *tableEntry);
    u8 pad0BC[0x0CC - 0x0BC];
    /* +0x0CC, OBSERVED (ClassEAC0__FinishConstruct): dispatched as `(self, 0xD)`.
       Occupant `Obj6EAC0__SetMask` (code_2cc8c_f). */
    void (*slotCC)(ClassEAC0Obj *self, s32 a1);
    u8 pad0D0[0x0DC - 0x0D0];
    /* +0x0DC, OBSERVED (Class6E99C__StartFadeToIndex/Class6E99C__StartFadeDefault): dispatched with only
       `self`, and its own return feeds `slotB8`'s table index. For THIS
       class's own leaf instances the occupant is `Class6E99C__Configure` (this
       unit, a Class6E99C-table slot) -- a method calling a sibling slot
       back through the vtable rather than by name, which is legal and
       already established. */
    s32 (*configure)(ClassEAC0Obj *self); /* RENAMED round 61 (was slotDC). */
};
struct ClassEAC0Obj {
    ClassEAC0Methods *methods; /* +0x000 */
    u8 pad004[0x044 - 0x004];
    s32 unk44;                 /* +0x044, OBSERVED: ClassEAC0__FinishConstruct (ctor's own a3) */
    s32 unk48;                 /* +0x048, OBSERVED: ClassEAC0__FinishConstruct, set to 1 */
    s32 unk4C;                 /* +0x04C, OBSERVED: ClassEAC0__FinishConstruct, zeroed */
    u8 pad050[0x058 - 0x050];
    s32 unk58;                 /* +0x058, OBSERVED: ClassEAC0__FinishConstruct, zeroed */
    s16 unk5C;                 /* +0x05C, OBSERVED: ClassEAC0__FinishConstruct, zeroed */
    s16 unk5E;                 /* +0x05E, OBSERVED: ClassEAC0__FinishConstruct, zeroed */
    s16 unk60;                 /* +0x060, OBSERVED: ClassEAC0__FinishConstruct, from a1->0x0 */
    s16 unk62;                 /* +0x062, OBSERVED: ClassEAC0__FinishConstruct, from a1->0x4 */
    u8 pad064[0x06C - 0x064];
    s32 state;                 /* +0x06C, OBSERVED: Class6E99C__Stop/Class6E99C__StartFadeToIndex/
                                   Class6E99C__StartFadeDefault, a small dispatch-state tag
                                   (0 == idle, 1 == fading to an indexed color, 2 == fading
                                   to the default color -- RENAMED round 61, was unk6C;
                                   same field as Class6E99CObj::state below). */
    u8 pad070[0x074 - 0x070];
    s32 step;                  /* +0x074, OBSERVED: Class6E99C__StartFadeToIndex, negated on the
                                   "already had one" path; Class6E99C__SetStep's own
                                   setter also targets this offset on the
                                   Class6E99C leaf, same field (RENAMED round 61, was
                                   unk74; per-tick fade increment, see
                                   Class6E99CObj::step). */
    u8 pad078[0x098 - 0x078];
    s32 altMode;               /* +0x098, OBSERVED: Class6E99C__Stop, truthy-tested;
                                   Class6E99C__SetDivisorMode's own setter also targets this
                                   offset on the Class6E99C leaf, same field (RENAMED
                                   round 61, was unk98; see Class6E99CObj::altMode). */
};

struct Class6E99CMethods {
    s32 header;                                     /* +0x000 */
    void *unk04;                                     /* +0x004, BasicClass__Release, inherited, unused here */
    void (*ctor)(Class6E99CObj *self, void *a1, s32 a2, s32 a3); /* +0x008,
                                Class6E99C__Class6E99C (this unit). `a2` is a RAW
                                index/mode (0 or a small positive count),
                                not a pointer -- Class6E99C__Class6E99C's own body
                                converts it into a tableEntry pointer
                                internally before forwarding to the next
                                ctor down the chain (ClassEAC0Methods::ctor,
                                whose OWN `a2` really is a pointer). */
    void (*dtor)(Class6E99CObj *self);               /* +0x00C, Class6B5CC__Finalize, shared */
    /* +0x010/+0x014/+0x018, IS Class6B5CCMethods's own +0x010/+0x014/+0x018
       (Class6B5CC__AddChild/Class6B5CC__RemoveChild/Class6B5CC__RemoveAllChildren) -- identical addresses in
       both tables per the file banner's classtable.py census. */
    void (*slot10)(Class6E99CObj *self); /* +0x010, OBSERVED: Class6E99C__Configure */
    void (*slot14)(Class6E99CObj *self, s32 a1); /* +0x014, OBSERVED: Class6E99C__Stop */
    u8 pad018[0x030 - 0x018];
    /* +0x030, BasicClass-inherited (per the file banner's census, matches
       D_8006B58C's own +0x030 verbatim) -- OBSERVED: Class6E99C__Stop
       dispatches it as `(self, s32 a1)` with a1 a small literal (5 or 6). */
    void (*slot30)(Class6E99CObj *self, s32 a1);
    u8 pad034[0x040 - 0x034];
    void (*finishConstruct)(Class6E99CObj *self, s32 a1); /* +0x040, Class6E99C__FinishConstruct
                                (this unit). Two args, not four: its own
                                call site (Class6E99C__Class6E99C) only sets `a1`;
                                `a2`/`a3` are leftover from the preceding
                                ctor call and the occupant's own body never
                                reads them. RENAMED round 61 (was slot40). */
    u8 pad044[0x060 - 0x044];
    /* +0x060/+0x064, OBSERVED: Class6E99C__FinishConstruct/Class6E99C__Stop, both dispatched
       as `(self, s32 a1)`. Occupants (code_2cc8c_f, bravo's own functions):
       func_800406E4 (+0x060), func_80040714 (+0x064). */
    void (*slot60)(Class6E99CObj *self, s32 a1);
    void (*slot64)(Class6E99CObj *self, s32 a1);
    /* +0x068, OBSERVED: Class6E99C__Configure, dispatched as `(self, s32 flag)`
       where `flag` is that same function's own locally-computed 1-or-2
       mode value. */
    void (*slot68)(Class6E99CObj *self, s32 a1);
    u8 pad06C_[0x098 - 0x06C];
    /* +0x098, OBSERVED: Class6E99C__Update's own call target when `a2 == 2`.
       This unit's own function. */
    void (*update)(Class6E99CObj *self, void *a1, s32 a2); /* +0x098, Class6E99C__Update.
                                RENAMED round 61 (was slot98). */
    u8 pad09C[0x0B8 - 0x09C];
    /* +0x0B8/+0x0CC, IS ClassEAC0Methods's own +0x0B8/+0x0CC
       (Obj6EAC0__SetColor/Obj6EAC0__SetMask, both code_2cc8c_f) -- identical
       addresses in both tables (this class does not override them), same
       fingerprint as the other shared slots above. OBSERVED:
       Class6E99C__StartFadeToIndex/Class6E99C__StartFadeDefault (both this unit). */
    void (*slotB8)(Class6E99CObj *self, s32 a1, void *tableEntry);
    u8 pad0BC[0x0CC - 0x0BC];
    void (*slotCC)(Class6E99CObj *self, s32 a1);
    void (*setStep)(Class6E99CObj *self, s32 a1);     /* +0x0D0, Class6E99C__SetStep.
                                RENAMED round 61 (was slotD0). */
    void (*startFadeToIndex)(Class6E99CObj *self);             /* +0x0D4, Class6E99C__StartFadeToIndex.
                                RENAMED round 61 (was slotD4). */
    void (*startFadeDefault)(Class6E99CObj *self, s32 a1, s32 a2); /* +0x0D8, Class6E99C__StartFadeDefault.
                                RENAMED round 61 (was slotD8). */
    /* +0x0DC, IS ClassEAC0Methods's own +0x0DC too -- Class6E99C__Configure (this
       unit) is the shared occupant either way. RENAMED round 61 (was slotDC). */
    s32 (*configure)(Class6E99CObj *self);              /* +0x0DC, Class6E99C__Configure */
    /* Round 73 note: the slot is declared `(self)` only, but its occupant
       reads all four argument registers and both StartFade* callers forward
       their own a1..a3 to it; code_2cc8c_e.c calls it through a file-local
       4-argument view (Configure6E99CFn) rather than retyping this shared
       slot. The same holds for startFadeToIndex/startFadeDefault, whose
       definitions take (self, a1, a2, a3). */
    void (*stop)(Class6E99CObj *self, void *a1);   /* +0x0E0, Class6E99C__Stop.
                                RENAMED round 61 (was slotE0). */
    void *(*getColor)(Class6E99CObj *self);            /* +0x0E4, Class6E99C__GetColor.
                                RENAMED round 61 (was slotE4). */
    void (*pushPosition)(Class6E99CObj *self, SkipShort2 *a1, Pair32E99C *a2); /* +0x0E8, Class6E99C__PushPosition.
                                RENAMED round 61 (was slotE8). */
    void (*popPosition)(Class6E99CObj *self);             /* +0x0EC, Class6E99C__PopPosition.
                                RENAMED round 61 (was slotEC). */
    void (*setDivisorMode)(Class6E99CObj *self, s32 a1, s32 a2); /* +0x0F0, Class6E99C__SetDivisorMode.
                                RENAMED round 61 (was slotF0). */
};
struct Class6E99CObj {
    Class6E99CMethods *methods; /* +0x000 */
    u8 pad004[0x00C - 0x004];
    void *unkC;                /* +0x00C, OBSERVED: Class6E99C__PushPosition, truthy-tested only */
    /* +0x010, STALE CITATION FIXED round 61: this comment previously cited
       "func_8003FBF4" as evidence, but that address is `GsDrawOt`
       (`libgs/gs_111.o`, a linked Sony object -- see this file's own
       round-34 banner) and has never been part of this unit; nothing in
       this unit's current 17 functions reads or writes this field at all
       (confirmed: `grep -n 'unk10' src/code_2cc8c_e.c` matches only this
       declaration). The citation predates the round-34 segment split, when
       the address now known as `GsDrawOt` still lived in this file under a
       different, since-reclassified reading. Left untyped and unrenamed --
       there is no live evidence left for it in this unit -- but kept as a
       `void *` (not folded into surrounding padding) since offsets past it
       are load-bearing for +0x050 onward. Same base offset as
       Class6B5CCObj's own inherited `unk10` (code_d294.h, a `u32` packed
       bit-flags word) -- plausibly the same underlying field reused
       opaquely here, but kept independent per this project's
       multiple-local-views convention; that parallel is the only reason to
       keep the slot typed at all. */
    void *unk10;
    u8 pad014[0x050 - 0x014];
    s32 unk50;                 /* +0x050, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 unk54;                 /* +0x054, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    u8 pad058[0x060 - 0x058];
    /* +0x060/+0x062, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition -- `u16`, not
       `s16`: Class6E99C__PushPosition widens these into the `s32` unk88/unk8C fields
       via a plain assignment, and retail's `lhu` there (zero-extending)
       only matches when the source type is unsigned. */
    u16 unk60;
    u16 unk62;
    /* +0x064/+0x065/+0x066, OBSERVED: Class6E99C__Update -- three independent
       byte counters, each incremented by the low byte of `unk74` when the
       corresponding bit of `unk78` (0x4/0x2/0x1) is set. */
    u8 unk64;
    u8 unk65;
    u8 unk66;
    u8 pad067[0x068 - 0x067];
    s32 unk68;                 /* +0x068, OBSERVED: Class6E99C__Configure, a divisor */
    /* +0x06C, OBSERVED: Class6E99C__Stop/Class6E99C__StartFadeToIndex/Class6E99C__StartFadeDefault/
       Class6E99C__FinishConstruct (zeroed by the ctor override) -- a small dispatch-state
       tag: 0 == idle, 1 == fading to an indexed color (StartFadeToIndex),
       2 == fading to the default color (StartFadeDefault). RENAMED round
       61 (was unk6C); same field identity as ClassEAC0Obj::state above. */
    s32 state;
    s32 unk70;                 /* +0x070, OBSERVED: Class6E99C__FinishConstruct, set from
                                   its own `a1` parameter */
    /* +0x074, OBSERVED: Class6E99C__FinishConstruct (ctor override sets it to 0xA),
       Class6E99C__StartFadeToIndex (negated on the "already had one" path), Class6E99C__SetStep
       (a plain setter, `self->step = a1`); also read a BYTE at a time by
       Class6E99C__Update via its low byte -- the per-tick amount added into
       unk64/unk65/unk66 while a fade is running. RENAMED round 61 (was
       unk74); same field identity as ClassEAC0Obj::step above. */
    s32 step;
    s32 unk78;                 /* +0x078, OBSERVED: Class6E99C__Configure/Class6E99C__Update/
                                   Class6E99C__GetColor, a flags/mode word tested
                                   against 0xF and against bit masks
                                   0x1/0x2/0x4 */
    /* +0x07C, OBSERVED: Class6E99C__FinishConstruct (ctor override zeroes it),
       Class6E99C__Update (tested `== 9`), Class6E99C__Configure (set from its own a3
       parameter). */
    s32 unk7C;
    s32 unk80;                 /* +0x080, OBSERVED: Class6E99C__Update, a countdown */
    s32 unk84;                 /* +0x084, OBSERVED: Class6E99C__Configure, a division result */
    /* +0x088/+0x08C, OBSERVED: Class6E99C__PopPosition (read via `lhu`, into `s16`
       unk60/unk62 -- a narrowing read of only the low halfword) and
       Class6E99C__PushPosition (WRITTEN via a plain WORD `sw`, from `lhu`-loaded
       unk60/unk62 -- a genuine `s32` field, widened on write). Retail's
       own `sw` at this offset is why these are `s32`, not `s16` -- an
       earlier reading typed them `s16` from Class6E99C__PopPosition's read alone and
       inserted a 2-byte pad to keep unk90 at the right offset; the pad was
       the wrong fix for the wrong field width. */
    s32 unk88;
    s32 unk8C;
    s32 unk90;                 /* +0x090, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 unk94;                 /* +0x094, OBSERVED: Class6E99C__PopPosition/Class6E99C__PushPosition */
    s32 altMode;               /* +0x098, OBSERVED: Class6E99C__SetDivisorMode, setter arg1;
                                   shared field identity with ClassEAC0Obj's
                                   own altMode above (same base offset).
                                   RENAMED round 61 (was unk98). */
    s32 divisor;               /* +0x09C, OBSERVED: Class6E99C__SetDivisorMode, setter arg2;
                                   Class6E99C__Configure also reads it as a divisor
                                   (gated on altMode != 0). RENAMED round 61
                                   (was unk9C). */
    u8 padA0[0xA0 - 0xA0];
};

/* Round-14 static tables this unit's own functions index into or pass by
 * address -- real element shape not derived (nothing this unit's chosen
 * functions dereference beyond taking the address), so left as opaque
 * byte blobs sized only by their known stride. `Class6E99C__Class6E99C`/
 * `Class6E99C__StartFadeToIndex`/`Class6E99C__StartFadeDefault`/`Class6E99C__GetColor` all compute the index as
 * a raw BYTE offset (`sll v0,i,1; addu v0,v0,i` = `i*3`, added directly to
 * the base address with no further `*4`) -- i.e. `D_8006EA90` holds 3-BYTE
 * entries (plausibly a signed-byte triple, same shape as this file's own
 * `SByte3_d294`), not 0xC-byte ones. `D_8006EAA8` is indexed the SAME way
 * by `Class6E99C__StartFadeDefault` (not a single fixed entry as an earlier reading of
 * `Class6E99C__Class6E99C` alone suggested -- that one just always passes index 0),
 * so left unsized rather than fixed at 3 bytes. `D_8008A924` has only the
 * one (unindexed) use, so kept at a single entry's size. */
extern u8 D_8006EA90[];
extern u8 D_8006EAA8[];
extern u8 D_8008A924[3];

extern Class6E99CMethods D_8006E99C;
/* D_8006EAC0 is declared once, above, as `Obj6EAC0Methods` -- bravo's
 * matched src/code_2cc8c_f.c returns its address. code_2cc8c_e declared
 * it too but never references it, so the duplicate is dropped. */
extern Class6E99CMethods *GetClass6E99CMethods(void); /* this unit's own bare getter
                                                    for &D_8006E99C, same
                                                    idiom as GetClass6B5CCMethods
                                                    (code_d294.h) */
/* Declared ONCE, matching its definition in src/code_2cc8c_f.c
 * (`Obj6EAC0Methods *Obj6EAC0__GetBaseMethods(void)`). code_2cc8c_e declared it
 * returning its own `ClassEAC0Methods *` view of the same table, which
 * collided as `conflicting types`; that unit casts at its two call
 * sites instead. */
extern Obj6EAC0Methods *Obj6EAC0__GetBaseMethods(void);  /* code_2cc8c_f (bravo's own
                                                    function): bare getter
                                                    for &D_8006EAC0 */

/* This unit's own local view of the REAL base, `Class6B5CCObj`'s own table
   (code_d294.h's `GetClass6B5CCMethods`/`gClass6B5CCMethods`) -- ClassEAC0__ClassEAC0 (this
   unit) dispatches only the ctor slot, so only that one is modelled here,
   per this project's independent-local-views convention. */
/* (code_2cc8c_e's own view of GetClass6B5CCMethods's return type was merged
 * into D6B5CCGetterMethodsCC8C above by the head.) */


#endif
