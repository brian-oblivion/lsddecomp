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

/* +0x008/+0x00C (ctor/dtor) and most BasicClass-inherited slots are not
 * this round's functions (func_80049684/func_80049830, still INCLUDE_ASM
 * elsewhere in this unit) -- left untyped. */
typedef struct Class865C8Methods {
    s32 header;                                   /* +0x000 */
    void *unk04;                                   /* +0x004 BasicClass__func_17eb0 */
    void *ctor;                                    /* +0x008 func_80049684 */
    void *dtor;                                    /* +0x00C func_80049830 */
    void *unk10, *unk14, *unk18, *unk1C;           /* BasicClass, inherited */
    void *unk20, *unk24, *unk28, *unk2C;           /* BasicClass, inherited */
    void *unk30, *unk34;                           /* BasicClass, inherited */
    void *slot38;                                  /* +0x038 func_80049958 */
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
    void (*noop80)(void);                          /* +0x080 func_80049EAC (no-op, matched) */
    void *slot84;                                  /* +0x084 func_80049EB4, addiu_at-blocked */
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

/* Object size unconfirmed (this unit never allocates one of these itself --
 * func_8004A130 allocates the SIBLING class below instead). Field offsets
 * are only the ones this round's functions touch. */
struct Obj865C8 {
    Class865C8Methods *methods;   /* +0x000 */
    u8 pad04[0x18 - 0x04];
    SubObjA *subA;                /* +0x018, func_80049C50 */
    u8 pad1C[0x28 - 0x1C];
    s32 unk28;                    /* +0x028, func_8004A3EC */
    s32 unk2C;                    /* +0x02C, func_8004A458 */
    s32 unk30;                    /* +0x030, func_8004A228 (guard) */
    SubObjB *subB;                /* +0x034, func_8004A228 */
    u8 pad38[0x3C - 0x38];
    s32 unk3C;                    /* +0x03C, func_80049A14 */
};

/* Base class table shared by D_800865C8 and D_80086668 (resolved with
 * tools/classtable.py 0x800865C8 --vs 0x8006E878). Only the slots this
 * unit's functions dispatch through when explicitly calling the BASE
 * implementation are typed. */
typedef struct IntermediateBaseMethods {
    u8 pad00[0x0C];
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
    u8 pad4C[0x60 - 0x4C];
    void (*slot60)(void *self, s32 arg1);  /* +0x060 */
} IntermediateBaseMethods;

/* Uncarved (asm/code_2cc8c.s, not this unit's to write): a plain accessor
 * with no parameters, returning &D_8006E878. Same shape as
 * Get_vtable_DreamSys / func_800269E0 (docs/research/class-framework.md). */
extern IntermediateBaseMethods *func_8003E5C8(void);

/* A sibling class (D_80086668, 28 slots) that overrides several of
 * D_800865C8's slots (+0x008, +0x00C, +0x040, +0x044, +0x048) while sharing
 * the rest verbatim (+0x058..+0x070, confirmed identical function
 * addresses in both tables by tools/classtable.py). Constructed by
 * func_8004A130, a New_X allocator (0x38-byte instance). Only the ctor
 * slot is typed; everything else is out of this round's scope
 * (func_8004A19C/func_8004A2C4, still INCLUDE_ASM elsewhere in this unit). */
typedef struct Class86668Methods {
    u8 pad00[0x08];
    void *(*ctor)(void *self, void *arg1, void *arg2); /* +0x008 func_8004A19C */
} Class86668Methods;

/* A plain accessor with no parameters, returning &D_80086668. Defined in the
 * class_3ac78 unit, not this one -- matched there as C in round 2026-09-02,
 * so it is no longer INCLUDE_ASM. Declared here only because this unit
 * dispatches through it; class_3ac78.h holds its owning view. */
extern Class86668Methods *func_8004A4B8(void);

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
 * class_16334.h for the other units that also declare it locally. */
extern void *func_80017B34(s32 size);

#endif
