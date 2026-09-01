#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"

/*
 * Two classes carved from the head of the old 415-function class_39e08
 * segment this round (see docs/match-reports for the evidence trail).
 *
 * ClassA's table is D_800865C8 (33 slots): a direct child of BasicClass
 * (D_8006B58C, resolved with tools/classtable.py), overriding the ctor
 * (+0x008, func_80049684), the dtor (+0x00C, func_80049830) and +0x038,
 * and adding new virtuals from +0x040 to +0x084.
 *
 * ClassB's table is D_80086668 (28 slots): confirmed a subclass of ClassA
 * with `classtable.py D_80086668 --vs D_800865C8` -- 19 of 33 slots are
 * byte-identical copies, and ClassB overrides the ctor (func_8004A19C),
 * dtor (func_8004A228), +0x038 (func_8003E030, not this unit's), and the
 * three new-virtual slots +0x040/+0x044/+0x048 (func_8004A294/func_8004A2C4/
 * func_8004A324). It reuses ClassA's own +0x05C/+0x060/+0x06C bodies
 * verbatim (func_8004A364/func_8004A3EC/func_8004A458 appear at the same
 * offsets in both tables) and simply doesn't extend the table past +0x070,
 * so ClassB objects are typed as ClassA here -- no instance field beyond
 * ClassA's is observed from this unit's functions.
 *
 * No FirecatFG name survives for either class (only ordinary func_ names in
 * config/symbols), so fields are named by offset/use, not by hypothesis.
 * Only slots and fields this unit's carved functions actually touch are
 * given concrete types; the rest stay opaque so the struct keeps the right
 * size without requiring every method to be typed up front.
 */

typedef struct ClassA ClassA;
typedef struct ClassAMethods ClassAMethods;

/* BasicClass's own table, reached here via func_8004A4B8 (defined in the
 * class_3ac78 unit, not this one) rather than a direct address load --
 * see func_8004A130's match report. Only the ctor slot this unit calls
 * through it is typed. */
typedef struct BasicClassMethods {
    /* +0x000 */ s32 header;
    /* +0x004 */ void *unk04;
    /* +0x008 */ void *(*ctor)(void *self, void *arg1, void *arg2);
    /* +0x00C */ void *unk0C;
} BasicClassMethods;

extern BasicClassMethods *func_8004A4B8(void);
extern void *func_80017B34(s32 size);

/* An object of unresolved class reached through ClassA::unk18. Only the two
 * slots func_80049C50 dispatches through (+0x074 and +0x090) are typed;
 * everything else is padding to keep the offsets right. */
typedef struct Dispatch18Methods {
    u8 pad00[0x74];
    void (*slot74)(void *self);
    u8 pad78[0x18];
    void (*slot90)(void *self);
} Dispatch18Methods;

typedef struct Dispatch18 {
    Dispatch18Methods *methods;
} Dispatch18;

/* An object of unresolved class, produced by func_80052B70 and stashed at
 * ClassA::unk4C. Only the +0x044 slot func_80049E20 calls is typed. */
typedef struct Dispatch4CMethods {
    u8 pad00[0x44];
    void (*slot44)(void *self, s32 arg1, s32 arg2);
} Dispatch4CMethods;

typedef struct Dispatch4C {
    Dispatch4CMethods *methods;
} Dispatch4C;

/* FIVE arguments, not four: func_80052B70 reads its fifth at 0x48($sp)
 * against a 0x38 frame, i.e. the caller's stack argument slot at
 * 0x10($sp). See docs/match-reports/func_80049E20.md. */
extern void *func_80052B70(void *arg0, s32 arg1, void *arg2, void *arg3, s32 arg4);

/* An object of unresolved class reached through ClassA::unk34 (a listener,
 * notified from ClassB's dtor when unk30 is set). Only the +0x004 slot is
 * typed; it is called with the object itself as its sole argument, the
 * same obj->methods->slotN(obj) shape as Dispatch18/Dispatch4C above. */
typedef struct Dispatch34Methods {
    void *unk00;
    void (*slot4)(void *self);
} Dispatch34Methods;

typedef struct Dispatch34 {
    Dispatch34Methods *methods;
} Dispatch34;

/* func_8003E5C8 (defined outside this unit, asm/code_2cc8c.s -- uncarved)
 * is called from four of this unit's functions and every slot read off its
 * result (+0x00C, +0x044, +0x048, +0x060) matches ClassA's OWN base
 * implementation at that offset, including cases where the caller is
 * ClassB's override of that very slot (func_8004A2C4 is ClassB's +0x044,
 * and reads +0x044 off this call's result to reach ClassA's un-overridden
 * func_80049A1C rather than recursing into itself). Typed as returning
 * ClassAMethods* on that evidence -- see the match reports for
 * func_8004A228/func_8004A2C4/func_8004A324/func_8004A3EC. */
extern ClassAMethods *func_8003E5C8(void);

struct ClassAMethods {
    /* +0x000 */ s32 header;                                       /* class-id/flags word */
    /* +0x004 */ void *unk04;                                      /* BasicClass__func_17eb0, inherited */
    /* +0x008 */ void (*ctor)(ClassA *self, void *arg1, void *arg2, s32 arg3);  /* func_80049684 */
    /* +0x00C */ void *(*dtor)(ClassA *self);                      /* func_80049830 */
    /* +0x010 */ void (*slot10)(ClassA *self, void *arg1);         /* BasicClass__func_17f98, inherited; used by func_80049E20 */
    /* +0x014 */ void *unk14;                                      /* BasicClass__func_17ff0, inherited */
    /* +0x018 */ void *unk18;                                      /* BasicClass__func_18040, inherited */
    /* +0x01C */ void *unk1C;                                      /* BasicClass__func_180bc, inherited */
    /* +0x020 */ void *unk20;                                      /* BasicClass__func_180fc, inherited */
    /* +0x024 */ void *unk24;                                      /* BasicClass__func_1811c, inherited */
    /* +0x028 */ void *unk28;                                      /* BasicClass__func_1813c, inherited */
    /* +0x02C */ void *unk2C;                                      /* BasicClass__func_1816c, inherited */
    /* +0x030 */ void *unk30;                                      /* BasicClass__func_182cc, inherited */
    /* +0x034 */ void *unk34;                                      /* BasicClass__func_18350, inherited */
    /* +0x038 */ void *unk38;                                      /* func_80049958, ClassA override; func_8003E030 in ClassB */
    /* +0x03C */ void *unk3C;                                      /* null slot */
    /* +0x040 */ void (*resetState)(ClassA *self);                 /* func_80049A14; func_8004A294 in ClassB */
    /* +0x044 */ void (*slot44)(ClassA *self, void *arg1, void *arg2);  /* func_80049A1C; func_8004A2C4 in ClassB */
    /* +0x048 */ s32 (*slot48)(ClassA *self);                      /* func_80049AC0; func_8004A324 in ClassB */
    /* +0x04C */ void *unk4C;                                      /* func_80049B54; absent (null run) in ClassB */
    /* +0x050 */ void (*slot50)(ClassA *self);                     /* func_80049C50; absent in ClassB */
    /* +0x054 */ void *unk54;                                      /* func_80049CA8; func_8003E418 in ClassB */
    /* +0x058 */ void (*slot58)(void);                             /* func_8004A35C (empty stub), shared */
    /* +0x05C */ void *unk5C;                                      /* func_8004A364, shared */
    /* +0x060 */ void (*onEvent)(ClassA *self, s32 event);         /* func_8004A3EC, shared */
    /* +0x064 */ void *unk64;                                      /* func_8003E538, shared */
    /* +0x068 */ void *unk68;                                      /* func_8003E578, shared */
    /* +0x06C */ void (*setRadius)(ClassA *self, s32 val);         /* func_8004A458, shared */
    /* +0x070 */ void *unk70;                                      /* func_8004A478 (class_3ac78 unit), shared */
    /* +0x074 */ void *unk74;                                      /* null slot, ClassA only */
    /* +0x078 */ void *unk78;                                      /* null slot, ClassA only */
    /* +0x07C */ void (*slot7C)(ClassA *self);                     /* func_80049EA4 (empty stub), ClassA only; called with self by func_8004A3EC */
    /* +0x080 */ void (*slot80)(void);                             /* func_80049EAC (empty stub), ClassA only */
    /* +0x084 */ void (*slot84)(void);                             /* func_80049EB4, ClassA only -- BLOCKED, see report */
};

struct ClassA {
    /* +0x00 */ ClassAMethods *methods;
    /* +0x04 */ u8 unk04[8];        /* BasicClass instance fields; owned by whatever unit decompiles BasicClass itself */
    /* +0x0C */ void *unk0C;        /* ctor arg1, stored verbatim (func_80049684) */
    /* +0x10 */ u8 unk10[8];        /* unresolved; not touched by this unit's carved functions */
    /* +0x18 */ Dispatch18 *unk18;  /* an unresolved-class object; func_80049C50 dispatches its +0x074/+0x090 */
    /* +0x1C */ u8 unk1C[0xC];      /* unresolved */
    /* +0x28 */ s32 unk28;          /* flag/output word; written 0 then possibly 1 by ClassB's onEvent/slot44 override, read back as a return value */
    /* +0x2C */ s32 radius;         /* set verbatim if negative, else value*20 (func_8004A458) */
    /* +0x30 */ s32 unk30;          /* boolean; gates a notify-on-destroy call in ClassB's dtor */
    /* +0x34 */ Dispatch34 *unk34;  /* a listener object; ->methods->slot4(unk34) called from ClassB's dtor when unk30 is set */
    /* +0x38 */ void *unk38;        /* ctor arg2, stored verbatim; a big-vtable object (func_80049684 calls slots 0x10C/0x114 on it) */
    /* +0x3C */ s32 state;          /* reset to 0 by resetState (func_80049A14), set to 2 by func_80049E20 */
    /* +0x40 */ s32 unk40;          /* func_8004A070's result, forwarded into func_80052B70 */
    /* +0x44 */ void *unk44;        /* func_8003B39C's result, forwarded into func_80052B70 */
    /* +0x48 */ void *unk48;        /* func_80048D74's result, forwarded into func_80052B70 */
    /* +0x4C */ Dispatch4C *unk4C;  /* func_80052B70's result; func_80049E20 dispatches its +0x044 */
};

/* ClassA's own table (still raw asm data, asm/data/76DC8.data.s -- not
 * decompiled to a C initializer by this unit). */
extern ClassAMethods D_800865C8;

/* ClassA's own vtable getter -- a direct address load, not a call (compare
 * func_8004A4B8, which is BasicClass's, defined in the class_3ac78 unit and
 * reached via jal everywhere else in this unit). */
ClassAMethods *func_8004A060(void);

/* ClassB shares ClassA's instance layout; no field beyond it is observed
 * from this unit's carved functions, so it is typed as ClassA at call
 * sites rather than introducing an empty subtype. */

#endif
