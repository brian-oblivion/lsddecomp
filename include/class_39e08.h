#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"

/*
 * The class whose method table is D_800865C8 (33 slots, resolved with
 * tools/classtable.py 0x800865C8 -- diff against 0x8006E878 to see the
 * override set). No FirecatFG name survives for this class, so fields are
 * named by offset until real names are known.
 *
 * Only the slots this unit's functions actually call through are given
 * concrete field types; the rest stay opaque `void *`/`u8 pad` so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/Class6D3C8.h).
 */
typedef struct Obj865C8 Obj865C8;
typedef struct Obj4C Obj4C;

/* What func_80049958 (Class865C8Methods slot +0x038) inspects: `arg1` is a
 * pointer to a small wrapper whose own field 0 is a pointer to some OTHER,
 * unrelated header-tagged object (double indirection confirmed by the
 * disassembly's two chained `lw ..., 0x0(reg)`). Only the one word each
 * struct exposes here is named; neither is this unit's own object type. */
typedef struct HeaderObj {
    s32 header;
} HeaderObj;
typedef struct EventArg {
    HeaderObj *target;
} EventArg;

/* +0x008/+0x00C (ctor/dtor) and most BasicClass-inherited slots are not
 * this round's functions (func_80049684/func_80049830, still INCLUDE_ASM
 * elsewhere in this unit) -- left untyped. */
typedef struct Class865C8Methods {
    s32 header;                                   /* +0x000 */
    void *unk04;                                   /* +0x004 BasicClass__func_17eb0 */
    void *ctor;                                    /* +0x008 func_80049684 */
    void *dtor;                                    /* +0x00C func_80049830 */
    /* BasicClass-inherited (BasicClass__func_17f98 -- same address as
     * Class6D3C8.h's own local unk10 view of this same shared slot).
     * Called by func_80049E20 as self->methods->slot10(self, newObj). */
    void (*slot10)(Obj865C8 *self, Obj4C *arg1);   /* +0x010 */
    void *unk14, *unk18, *unk1C;                   /* BasicClass, inherited */
    void *unk20, *unk24, *unk28, *unk2C;           /* BasicClass, inherited */
    void *unk30, *unk34;                           /* BasicClass, inherited */
    /* Occupied here by func_80049958 itself; only reachable from THIS
     * struct via func_8004A4B8()'s own D_80086668 view of the same offset
     * (Class86668Methods::slot38 below), where it forwards to the inherited
     * func_8003E030. */
    void (*slot38)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x038 func_80049958 */
    void *unk3C;                                   /* +0x03C null slot */
    void (*resetUnk3C)(Obj865C8 *self);            /* +0x040 func_80049A14 */
    void *slot44;                                  /* +0x044 func_80049A1C */
    void *slot48;                                  /* +0x048 func_80049AC0 */
    void *slot4C;                                  /* +0x04C func_80049B54 */
    void (*runSubUpdates)(Obj865C8 *self);         /* +0x050 func_80049C50 */
    void *slot54;                                  /* +0x054 func_80049CA8 */
    void (*noop58)(void);                          /* +0x058 func_8004A35C (no-op, matched) */
    void *slot5C;                                  /* +0x05C func_8004A364 */
    /* Shared with D_80086668 (see Class86668Methods below) -- literally the
     * same function address at the same offset in both tables. */
    void (*onEventArg)(Obj865C8 *self, s32 arg1);  /* +0x060 func_8004A3EC */
    void *unk64, *unk68;                           /* shared base slots (func_8003E538/78) */
    /* Also shared with D_80086668 at the same offset. */
    void (*setUnk2C)(Obj865C8 *self, s32 arg1);    /* +0x06C func_8004A458 */
    void *unk70;                                   /* func_8004A478 */
    void *unk74, *unk78;                           /* null slots */
    void (*noop7C)(Obj865C8 *self);                /* +0x07C func_80049EA4 (no-op, matched) */
    /* Retyped from `void (*noop80)(void)`: func_80049958 dispatches this
     * slot as `self->methods->slot80(self, arg1, arg2)` with real
     * arguments loaded into $a1/$a2 -- func_80049EAC (D_800865C8's own
     * occupant, still matched, `void func_80049EAC(void) {}`) simply
     * ignores them. A no-op BODY is not evidence the SLOT's signature takes
     * no arguments; only THIS slot's other occupants would be. */
    void (*slot80)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x080 func_80049EAC (no-op body, matched) */
    /* Same signature as slot80 by the same call site (func_80049958's other
     * branch); occupant func_80049EB4 is still addiu_at-blocked. */
    void (*slot84)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x084 func_80049EB4, addiu_at-blocked */
} Class865C8Methods;

extern Class865C8Methods D_800865C8;

/* Opaque view of whatever object Obj865C8::subA points to (used only by
 * func_80049C50): its own vtable pointer sits at offset 0, and only the two
 * slots func_80049C50 dispatches through are named here -- declared
 * minimally, locally, for this one call site (same policy as
 * DreamSysEntityObj in include/DreamSys.h). */
typedef struct SubObjAMethods {
    u8 pad00[0x74];
    void (*slot74)(void *self);
    u8 pad78[0x90 - 0x78];
    void (*slot90)(void *self);
} SubObjAMethods;
typedef struct SubObjA {
    SubObjAMethods *methods;
} SubObjA;

/* Opaque view of whatever object Obj865C8::subB points to (used only by
 * func_8004A228, guarded by Obj865C8::unk30): same "vtable at offset 0,
 * only slot +0x004 named" policy. */
typedef struct SubObjBMethods {
    u8 pad00[0x04];
    void (*slot4)(void *self);
} SubObjBMethods;
typedef struct SubObjB {
    SubObjBMethods *methods;
} SubObjB;

/* Opaque view of whatever object func_80052B70 (uncarved, unit class_3bb8c)
 * returns and stores at Obj865C8::unk4C (used only by func_80049E20): same
 * "vtable at offset 0, only the one dispatched slot named" policy as
 * SubObjA/SubObjB above. */
typedef struct Obj4CMethods {
    u8 pad00[0x44];
    void (*slot44)(Obj4C *self, s32 arg1, s32 arg2);
} Obj4CMethods;
struct Obj4C {
    Obj4CMethods *methods;
};

/* Opaque view of whatever object Obj865C8::unk0C points to (used by
 * func_80049AC0/func_80049A1C, which read its own +0x004/+0x008/+0x010
 * fields -- no vtable dispatch through this one, so no methods pointer is
 * declared). */
typedef struct Obj0C {
    u8 pad00[0x04];
    s32 unk4;                     /* +0x004 */
    s32 unk8;                     /* +0x008 */
    u8 padC[0x10 - 0xC];
    s32 unk10;                    /* +0x010 */
} Obj0C;

/* Opaque view of whatever object Obj865C8::unk38 points to (used by
 * func_80049AC0/func_80049A1C): same "vtable at offset 0, only the reached
 * slots named" policy as SubObjA/SubObjB/Obj4C above. */
typedef struct SubObjD SubObjD;
typedef struct SubObjDMethods {
    u8 pad00[0x10];
    void (*slot10)(SubObjD *self, s32 arg1);
    void (*slot14)(SubObjD *self, s32 arg1);
    u8 pad18[0x110 - 0x18];
    void (*slot110)(SubObjD *self, s32 arg1);
} SubObjDMethods;
struct SubObjD {
    SubObjDMethods *methods;
};

/* Object size unconfirmed (this unit never allocates one of these itself --
 * func_8004A130 allocates the SIBLING class below instead). Field offsets
 * are only the ones this round's functions touch. */
struct Obj865C8 {
    Class865C8Methods *methods;   /* +0x000 */
    u8 pad04[0x0C - 0x04];
    Obj0C *unk0C;                 /* +0x00C, func_80049AC0 dereferences (->unk4); passed
                                      through as a plain register value to
                                      func_8003E5C8()->slot44's 2nd arg by func_80049E20 */
    s32 unk10;                    /* +0x010, func_80049AC0 (2nd arg to a slot14 call) */
    u8 pad14[0x18 - 0x14];
    SubObjA *subA;                /* +0x018, func_80049C50 */
    s32 unk1C;                    /* +0x01C, func_8004A364 (compared against unk2C) */
    u8 pad20[0x28 - 0x20];
    s32 unk28;                    /* +0x028, func_8004A3EC */
    s32 unk2C;                    /* +0x02C, func_8004A458 */
    s32 unk30;                    /* +0x030, func_8004A228 (guard) */
    SubObjB *subB;                /* +0x034, func_8004A228 */
    SubObjD *unk38;                /* +0x038, func_80049AC0 dereferences (->methods); passed
                                       through as a plain register value to
                                       func_8003E5C8()->slot44's 3rd arg by func_80049E20 */
    s32 unk3C;                    /* +0x03C, func_80049A14 */
    s32 unk40;                    /* +0x040, func_80049E20 (2nd arg to func_80052B70) */
    s32 unk44;                    /* +0x044, func_80049E20 (3rd arg to func_80052B70) */
    s32 unk48;                    /* +0x048, func_80049E20 (4th arg to func_80052B70) */
    Obj4C *unk4C;                 /* +0x04C, func_80049E20 -- result of func_80052B70 */
};

/* Base class table shared by D_800865C8 and D_80086668 (resolved with
 * tools/classtable.py 0x800865C8 --vs 0x8006E878). Only the slots this
 * unit's functions dispatch through when explicitly calling the BASE
 * implementation are typed. */
typedef struct IntermediateBaseMethods {
    u8 pad00[0x008];
    void *(*ctor)(void *self);             /* +0x008, called by func_8004A19C with only `self` set up */
    void (*dtor)(void *self);              /* +0x00C */
    u8 pad10[0x44 - 0x10];
    /* Same accessor/slot combination code_2c054.h calls
     * `TaskUtilMethods::slot44` on -- there it forwards to
     * `self->unk38 = <base result>` (func_8003C1DC). Here the caller
     * (func_8004A2C4, D_80086668's own +0x044 override) zeroes
     * `self->unk28` immediately before the call and reads it back
     * immediately after: same "default, then base may overwrite" shape. */
    void (*slot44)(void *self, s32 arg1, s32 arg2); /* +0x044 */
    void (*slot48)(void *self);            /* +0x048 */
    u8 pad4C[0x5C - 0x4C];
    void (*slot5C)(void *self, s32 arg1, s32 arg2); /* +0x05C, called by func_8004A364 */
    void (*slot60)(void *self, s32 arg1);  /* +0x060 */
} IntermediateBaseMethods;

/* Uncarved (asm/code_2cc8c.s, not this unit's to write): a plain accessor
 * with no parameters, returning &D_8006E878. Same shape as
 * Get_vtable_DreamSys / func_800269E0 (docs/research/class-framework.md). */
extern IntermediateBaseMethods *func_8003E5C8(void);

/* Allocator in the still-uncarved unit class_3bb8c (asm/class_3bb8c.s):
 * allocates an 0x88-byte instance, ctors it, and dispatches its own slot
 * +0x008 with the 5 forwarded arguments, returning the new instance (or 0
 * on allocation failure). Only the one call site here (func_80049E20)
 * cares about its signature; `a0`'s type is inherited from whatever the
 * caller actually passes (this unit's own `SubObjB *`), the remaining
 * scalar args are untyped beyond their register width. */
extern Obj4C *func_80052B70(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);

/* A sibling class (D_80086668, 28 slots) that overrides several of
 * D_800865C8's slots (+0x008, +0x00C, +0x040, +0x044, +0x048) while sharing
 * the rest verbatim (+0x058..+0x070, confirmed identical function
 * addresses in both tables by tools/classtable.py). Constructed by
 * func_8004A130, a New_X allocator (0x38-byte instance). Instances share
 * `Obj865C8`'s own layout (func_8004A19C writes `unk30`/`subB` at the exact
 * same offsets `Obj865C8`'s other functions already use), so this class's
 * own instances are typed `Obj865C8 *` too rather than inventing a second,
 * parallel struct.
 *
 * `ctor` (+0x008, func_8004A19C) never uses its own return value at either
 * of its two call sites (its own body sets no explicit `$v0` before
 * returning either -- CLAUDE.md's "discarded return is never evidence of
 * void" rule is about NOT assuming void from a discarding caller alone, but
 * here the callee's OWN body never materializes a return value at all, so
 * `void` is the callee-side reading, not an inference from the caller). */
typedef struct Class86668Methods {
    u8 pad00[0x08];
    void (*ctor)(Obj865C8 *self, s32 arg1, SubObjB *arg2); /* +0x008 func_8004A19C */
    u8 pad0C[0x38 - 0x0C];
    /* Inherited, shared verbatim with D_800865C8's own occupant of this
     * offset (func_80049958, this unit): D_80086668's own +0x038 is
     * func_8003E030 (a base/inherited slot, out of this unit's scope).
     * Called by func_80049958 as func_8004A4B8()->slot38(self, arg1, arg2). */
    void (*slot38)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x038 func_8003E030 */
    u8 pad3C[0x44 - 0x3C];
    /* func_8004A2C4 (this unit, matched): zeroes self->unk28, forwards to
     * the base's own slot44, returns self->unk28. Called by func_80049A1C
     * as func_8004A4B8()->slot44(self, self->unk0C, 0), return discarded. */
    s32 (*slot44)(Obj865C8 *self, s32 arg1, s32 arg2);      /* +0x044 func_8004A2C4 */
    /* func_8004A324 (this unit, matched): a thin wrapper forwarding to
     * func_8003E5C8()->slot48(self). Called by func_80049AC0 as
     * func_8004A4B8()->slot48(self). */
    void (*slot48)(Obj865C8 *self);                        /* +0x048 func_8004A324 */
} Class86668Methods;

/* A plain accessor with no parameters, returning &D_80086668. Defined in the
 * class_3ac78 unit, not this one -- matched there as C in round 2026-09-02,
 * so it is no longer INCLUDE_ASM. Declared here only because this unit
 * dispatches through it; class_3ac78.h holds its owning view. */
extern Class86668Methods *func_8004A4B8(void);

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
 * class_16334.h for the other units that also declare it locally. */
extern void *func_80017B34(s32 size);

/* Allocator in the still-uncarved unit code_179d8 (asm/code_179d8.s):
 * allocates a 0x64-byte instance and, on success, ctors it with the single
 * forwarded argument. Only call site here is func_8004A19C, which stores
 * the result straight into `Obj865C8::subB` (`SubObjB *`). */
extern SubObjB *func_8002C480(s32 arg1);

#endif
