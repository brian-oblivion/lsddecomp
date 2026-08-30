#ifndef CODE_55DD4_H
#define CODE_55DD4_H

#include "common.h"

/*
 * The class allocated by New_class_65650 / constructed by
 * class_65650__Constructor. Method table is D_8008A6C4 (80 slots, header
 * word 0x234). No FirecatFG name survives for this class (only
 * New_class_65650 and class_65650__Constructor are named in the symbol
 * file), so fields are named by offset until real names are known.
 *
 * Inheritance, resolved with tools/classtable.py (never by counting):
 * BasicClass (D_8006B58C, 14 slots) -> an intermediate class at D_800878D4
 * (59 slots, header 0x34: overrides BasicClass's ctor/dtor/+0x10/+0x14/+0x18
 * and +0x038, adds 0x040..0x0EC) -> this class (D_8008A6C4, 80 slots:
 * overrides the intermediate class's ctor (+0x008) and dtor (+0x00C), keeps
 * every other inherited slot verbatim, and adds 0x0F0..0x140 of its own).
 * The SAME intermediate class D_800878D4 is DreamSys's immediate base too
 * (see docs/research/class-framework.md's own worked example) -- this class
 * and DreamSys are siblings under one shared, still-unnamed middle class.
 *
 * Only the slots this unit's functions actually call through are given
 * concrete field types; the rest stay opaque `void *` so the struct keeps
 * the right size/offsets without requiring every method to be typed up
 * front.
 */

typedef struct Class65650 Class65650;

/* The intermediate base class at D_800878D4. Same policy as Class6D3C8.h's
 * MiddleClassMethods: only slots this unit actually dispatches through are
 * typed. Resolved via func_80057C84, which returns &D_800878D4 (a plain
 * lui/addiu address-of, not a gp_rel load). */
typedef struct D800878D4Methods {
    s32 header;                                    /* +0x000 */
    void *unk04;                                    /* +0x004 BasicClass__func_17eb0, inherited */
    Class65650 *(*ctor)(Class65650 *self);            /* +0x008 func_80057044 */
    void (*dtor)(Class65650 *self);                    /* +0x00C func_8001CBA4 */
    void (*slot10)(void *self, void *arg);             /* +0x010 func_800570B4 */
    void *unk14;                                        /* +0x014 func_80057130 */
    void *unk18;                                         /* +0x018 func_800571A8 */
} D800878D4Methods;

extern D800878D4Methods *func_80057C84(void);

/* Whatever class self->arg2 (below) points at: unidentified, only its
 * vtable slot +0x080 is needed so far, by func_800661D4. */
typedef struct UnkArg2Methods {
    u8 pad00[0x80];                                       /* +0x000 .. +0x07C, unknown */
    void (*slot80)(void *self, void *arg1, s32 a2, s32 a3); /* +0x080 */
} UnkArg2Methods;

typedef struct UnkArg2Obj {
    UnkArg2Methods *methods;
} UnkArg2Obj;

typedef struct Class65650Methods {
    s32 header;                                                    /* +0x000 */
    void *unk04;                                                    /* +0x004 BasicClass__func_17eb0, inherited */
    Class65650 *(*ctor)(Class65650 *self, void *arg1, void *arg2);   /* +0x008 class_65650__Constructor */
    void (*dtor)(Class65650 *self);                                   /* +0x00C func_8006573C */
    void (*slot10)(Class65650 *self, void *arg);                       /* +0x010 func_800570B4, inherited from D_800878D4 */
    u8 pad14[0x2C];                                                     /* +0x014 .. +0x03C, inherited from D_800878D4 */
    void (*slot40)(Class65650 *self);                                    /* +0x040 func_80065830, this class's own */
    u8 pad44[0xB0];                                                       /* +0x044 .. +0x0F0, inherited/not yet needed */
    s32 (*slot_setup5C)(Class65650 *self, void *arg1);                     /* +0x0F4 func_80065BFC */
    void (*slot_teardown5C)(Class65650 *self);                              /* +0x0F8 func_80065C2C */
} Class65650Methods;

/* Object size is 0x98 (from the allocator call in New_class_65650). Field
 * offsets below are only the ones observed so far in this unit's functions. */
struct Class65650 {
    Class65650Methods *methods;   /* +0x00 */
    u8 unk04[0x54];                 /* +0x04 .. +0x57, BasicClass/intermediate-class instance fields, not this unit's to name */

    UnkArg2Obj *arg2;               /* +0x58 the constructor's third parameter, stashed verbatim; read by func_800661D4, which calls arg2->methods->slot80(arg2, forwardedArg, 0x6E, 0x6E) when non-NULL */
    void *unk5C;                   /* +0x5C lazily-populated sub-object; guarded by unk60, set up by func_80065C5C, torn down by func_80065CEC (both still INCLUDE_ASM this round) */
    s32 unk60;                     /* +0x60 guard flag for unk5C: 0 if unk5C already existed and was borrowed rather than allocated, 1 if this instance owns it */
    s32 unk64;                     /* +0x64 plain s32 field; set verbatim by func_80065BF4(self, value) */
    void *unk68;                   /* +0x68 zeroed in the constructor; consumed by func_80065830's tail (a pointer whose +0x20 is a callback arg) */
    s32 unk6C;                     /* +0x6C count, paired with the unk70/unk74 arrays */
    void *unk70;                   /* +0x70 array of item pointers, allocated by func_80065E1C */
    void *unk74;                   /* +0x74 parallel byte array, allocated by func_80065E1C */
    u8 pad78[0x14];                 /* +0x78 .. +0x8B, not yet decoded */

    s32 unk8C;                     /* +0x8C boolean-ish flag; set to 0 by func_80066148, to 1 (and returned) by func_8006613C */
    s32 unk90;                     /* +0x90 boolean-ish flag; set to 0 by func_800662B4, to 1 (and returned) by func_800662A8 */
    s32 unk94;                     /* +0x94 zeroed in the constructor; no other observed use yet */
};

extern Class65650Methods D_8008A6C4;
extern Class65650Methods *func_80066818(void);

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);

/* Same-unit helpers called directly by name (still INCLUDE_ASM this round).
 * func_80065C5C/func_80065CEC are the +0x5C sub-object's setup/teardown
 * bodies (dispatched through slot_setup5C/slot_teardown5C); func_80065E1C/
 * func_80065F2C are the +0x70/+0x74 arrays' setup/teardown bodies
 * (dispatched through slot_setup70/slot_teardown70). Neither E1C nor F2C
 * reference their own $a1 anywhere in their bodies, so they take only
 * `self`; C5C does use its own second argument (a1->0xC), so it keeps one. */
extern s32 func_80065C5C(Class65650 *self, void *arg1);
extern void func_80065CEC(Class65650 *self);
extern s32 func_80065E1C(Class65650 *self);
extern void func_80065F2C(Class65650 *self);

#endif
