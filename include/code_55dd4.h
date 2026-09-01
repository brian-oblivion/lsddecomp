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
    u8 pad1C[0x1C];                                       /* +0x01C .. +0x037, not yet needed */
    void (*slot38)(Class65650 *self, void *arg1, s32 arg2); /* +0x038 -- called by func_80065790 as slot38(self, arg1, arg2) */
    u8 pad3C[0x10];                                         /* +0x03C .. +0x04B, not yet needed */
    void (*slot4C)(Class65650 *self, void *arg1, void *arg2); /* +0x04C -- called by func_80065918 as slot4C(self, arg3, arg5) */
    void (*slot50)(Class65650 *self);                       /* +0x050 -- called by func_800659D0 */
    u8 pad54[0x0C];                                          /* +0x054 .. +0x05F, not yet needed */
    void (*slot60)(Class65650 *self, s32 arg);                /* +0x060 -- called by func_80065830 as slot60(self, 0) */
    u8 pad64[0x0C];                                             /* +0x064 .. +0x06F, not yet needed */
    void (*slot70)(Class65650 *self, void *arg);              /* +0x070 -- called by func_80065AE0 via func_80057C84()->slot70(self, arg) */
} D800878D4Methods;

extern D800878D4Methods *func_80057C84(void);

/* func_80065790's `arg1`: unidentified, only its own +0x000 field (itself
 * a pointer) is needed so far -- that pointed-at object's +0x000 field is
 * a u16 "type tag", checked against the magic value 0x5F03. */
typedef struct TaggedObj {
    u16 tag;   /* +0x000 */
} TaggedObj;

typedef struct TagCheckArg {
    TaggedObj *tagged;   /* +0x000 */
} TagCheckArg;

/* Whatever class self->arg2 (below) points at: unidentified, only its
 * vtable slot +0x080 is needed so far, by func_800661D4. */
typedef struct UnkArg2Methods {
    u8 pad00[0x80];                                       /* +0x000 .. +0x07C, unknown */
    void (*slot80)(void *self, void *arg1, s32 a2, s32 a3); /* +0x080 */
} UnkArg2Methods;

typedef struct UnkArg2Obj {
    UnkArg2Methods *methods;
} UnkArg2Obj;

/* Whatever class self->unk68 (below) points at: unidentified, only its
 * vtable slot +0x088 is needed so far, by func_80066150. */
typedef struct Unk68Methods {
    u8 pad00[0x88];                                    /* +0x000 .. +0x084, unknown */
    void (*slot88)(void *self, s32 arg);                /* +0x088 */
} Unk68Methods;

typedef struct Unk68Obj {
    Unk68Methods *methods;   /* +0x00 */
    u8 pad04[0x1C];            /* +0x04 .. +0x1F, unknown */
    s32 unk20;                  /* +0x20 -- read directly (not via ->methods) by func_80065830, passed as func_8001E770's second argument */
} Unk68Obj;

/* Whatever class each self->unk70[i] (below) points at: unidentified,
 * only its vtable slot +0x060 is needed so far, by func_80065A5C. */
typedef struct Unk70ElemMethods {
    u8 pad00[0x04];                          /* +0x000, unknown */
    void (*slot4)(void *self);                /* +0x004 -- called by func_80065F2C's teardown loop */
    u8 pad08[0x58];                            /* +0x008 .. +0x05C, unknown */
    void (*slot60)(void *self, void *arg);    /* +0x060 */
    u8 pad64[0x0C];                            /* +0x064 .. +0x06C, unknown */
    void (*slot70)(void *self, void *arg);      /* +0x070 -- called by func_80065AE0's loop, same signature as slot60 */
} Unk70ElemMethods;

typedef struct Unk70ElemObj {
    Unk70ElemMethods *methods;
} Unk70ElemObj;

/* Whatever class self->unk5C (below) points at: unidentified, only its
 * vtable slot +0x004 is needed so far, by func_80065CEC (a "release,
 * returns the new value to store back" idiom -- typically NULL). */
typedef struct Unk5CObj Unk5CObj;
typedef struct Unk5CMethods {
    u8 pad00[0x04];                          /* +0x000, unknown */
    Unk5CObj *(*slot4)(Unk5CObj *self);       /* +0x004 */
} Unk5CMethods;

/* self->unk5C->unk30's element chain (func_80066214 only):
 * Unk30Obj->arr is a header pointer; the element array itself starts 8
 * bytes past it (array[i] = *(GroupObj **)(arr + 8 + i * 4)). Each
 * GroupObj's own +0x10 field is an EntryObj*, whose +0x4 field is the
 * scalar func_80066214 copies into self->unk80, and whose address + 8
 * (NOT its +0x8 field's value -- the pointer itself, offset) is what
 * self->unk88 is set to, i.e. "the start of this entry's own inline data,
 * same +8-past-a-2-word-header shape yet again". */
typedef struct EntryObj2 {
    u8 pad00[0x04];
    s32 unk4;            /* +0x004 -- copied verbatim into self->unk80 */
} EntryObj2;

typedef struct GroupObj {
    u8 pad00[0x10];
    EntryObj2 *entry;    /* +0x010 */
} GroupObj;

typedef struct Unk30Obj {
    u8 pad00[0x10];
    u8 *arr;             /* +0x010 -- element i lives at *(GroupObj **)(arr + 8 + i * 4) */
} Unk30Obj;

struct Unk5CObj {
    Unk5CMethods *methods;   /* +0x00 */
    u8 pad04[0x2C];            /* +0x04 .. +0x2F, not this unit's to name */
    Unk30Obj *unk30;            /* +0x30 -- func_80066214's index table, see Unk30Obj above */
};

/* The constructor's `arg1` (forwarded through slot_setup5C into
 * func_80065C5C): unidentified, only its +0x00C field is needed so far --
 * a Unk5CObj* this class can either borrow (if already set) or allocate
 * a fresh one of its own (via func_8004468C) and own outright. */
typedef struct UnkArg1Obj {
    u8 pad00[0x0C];
    Unk5CObj *unk0C;
} UnkArg1Obj;

extern Unk5CObj *func_8004468C(UnkArg1Obj *arg);

typedef struct Class65650Methods {
    s32 header;                                                    /* +0x000 */
    void (*slot04)(Class65650 *self);                               /* +0x004 BasicClass__func_17eb0, inherited -- used by func_80065B80 when its `val` == 4 */
    Class65650 *(*ctor)(Class65650 *self, void *arg1, void *arg2);   /* +0x008 class_65650__Constructor */
    void (*dtor)(Class65650 *self);                                   /* +0x00C func_8006573C */
    void (*slot10)(Class65650 *self, void *arg);                       /* +0x010 func_800570B4, inherited -- "link" companion of slot14, see func_80066748 */
    void (*slot14)(Class65650 *self, void *arg);                        /* +0x014 func_80057130, inherited -- "unlink" companion of slot10, see func_800667B0 */
    u8 pad18[0x28];                                                      /* +0x018 .. +0x03C, inherited from D_800878D4, not yet needed */
    void (*slot40)(Class65650 *self);                                    /* +0x040 func_80065830, this class's own */
    u8 pad44[0x80];                                                       /* +0x044 .. +0x0C0, inherited/not yet needed */
    void (*slotC4)(Class65650 *self, s32 arg1, s32 arg2);                  /* +0x0C4 -- called by func_80066150 as slotC4(self, -0x1E, 0) */
    u8 padC8[0x1C];                                                        /* +0x0C8 .. +0x0E3, inherited/not yet needed */
    void (*slotE4)(Class65650 *self, s32 arg);                              /* +0x0E4 -- called by func_80065830 as slotE4(self, 0x12C) */
    u8 padE8[0x08];                                                          /* +0x0E8 .. +0x0EF, inherited/not yet needed */
    void (*slotF0)(Class65650 *self, s32 arg);                                /* +0x0F0 -- called by func_80065830 as slotF0(self, 1) */
    s32 (*slot_setup5C)(Class65650 *self, void *arg1);                     /* +0x0F4 func_80065BFC */
    void (*slot_teardown5C)(Class65650 *self);                              /* +0x0F8 func_80065C2C */
    u8 padFC[0x04];                                                          /* +0x0FC, this unit's own slot (func_80065D64), not dispatched through here */
    s32 (*slot100)(Class65650 *self);                                         /* +0x100 -- called by func_80065C5C on success (its own return value) */
    void (*slot_teardown70)(Class65650 *self);                                /* +0x104 func_80065DEC -- called by func_80065CEC */
    void (*slot108)(Class65650 *self);                                        /* +0x108 func_80065FD8 -- used by func_80065B80 when its `val` == 2 */
    void (*slot10C)(Class65650 *self, s32 arg);                                 /* +0x10C -- called by func_80065830 as slot10C(self, 0x41) */
    u8 pad110[0x04];                                                             /* +0x110 .. +0x113, not yet needed as a call target */
    void (*slot114)(Class65650 *self);                                           /* +0x114 -- called by func_80065830 as slot114(self) */
    void *slot118;                                                              /* +0x118 -- only ever taken as a pointer VALUE (func_800660BC), never called from this unit, so left untyped-as-function */
    void *slot11C;                                                               /* +0x11C ditto */
    void *slot120;                                                               /* +0x120 ditto */
    u8 pad124[0x04];                                                             /* +0x124 .. +0x127, this unit's own slot, not dispatched through here */
    void (*slot128)(Class65650 *self, s32 arg);                                    /* +0x128 -- called by func_80065830 as slot128(self, 0) */
    u8 pad12C[0x04];                                                                /* +0x12C .. +0x12F, this unit's own slot, not dispatched through here */
    void (*slot130)(Class65650 *self);                                               /* +0x130 -- called by func_80065830 as slot130(self) */
    void (*slot134)(Class65650 *self, void *ptr, s32 flag);                        /* +0x134 -- called by func_80066214 as slot134(self, self->unk88, 0) */
    void *(*slot138)(Class65650 *self, void *acc, void *extra);                    /* +0x138 func_80066340 -- called by func_800662BC in a fold/reduce; func_80066340 itself is out of scope this round (258 words) */
    void (*slot13C)(Class65650 *self, Class65650 *other);                            /* +0x13C func_80066748, this unit's own -- confirmed as a real dispatch target by func_80065918's self->methods->slot13C(self, arg1) call; func_80066748's own signature (self, other) matches */
    void (*slot140)(Class65650 *self);                                               /* +0x140 func_800667B0 -- called by func_800659D0 */
} Class65650Methods;

/* Object size is 0x98 (from the allocator call in New_class_65650). Field
 * offsets below are only the ones observed so far in this unit's functions. */
struct Class65650 {
    Class65650Methods *methods;   /* +0x00 */
    u8 pad04[0x08];                  /* +0x04 .. +0x0B, BasicClass/intermediate-class instance fields, not this unit's to name */
    s32 unk0C;                       /* +0x0C guard flag gating func_800659D0's whole body (slot140/slot14/base-slot50 cleanup); parallels DreamSys's own unk_0xC gate field in the SAME shared base class, though not proven equivalent */
    u8 pad10[0x40];                   /* +0x10 .. +0x4F, not this unit's to name */
    Class65650 *unk50;                 /* +0x50 companion-object pointer, unlinked via slot14 in func_800659D0 when set -- a second link slot distinct from unk94's */
    u8 pad54[0x04];                     /* +0x54 .. +0x57, not this unit's to name */

    UnkArg2Obj *arg2;               /* +0x58 the constructor's third parameter, stashed verbatim; read by func_800661D4, which calls arg2->methods->slot80(arg2, forwardedArg, 0x6E, 0x6E) when non-NULL */
    Unk5CObj *unk5C;                /* +0x5C lazily-populated sub-object; guarded by unk60, set up by func_80065C5C, torn down by func_80065CEC */
    s32 unk60;                     /* +0x60 guard flag for unk5C: 0 if unk5C already existed and was borrowed rather than allocated, 1 if this instance owns it */
    s32 unk64;                     /* +0x64 plain s32 field; set verbatim by func_80065BF4(self, value) */
    Unk68Obj *unk68;                /* +0x68 zeroed in the constructor; consumed by func_80066150, which calls unk68->methods->slot88(unk68, 6) when self->unk64 == 1 and unk68 is set */
    s32 unk6C;                     /* +0x6C count, paired with the unk70/unk74 arrays */
    Unk70ElemObj **unk70;           /* +0x70 array of item pointers, allocated by func_80065E1C; each element's own slot +0x060 is invoked (element, arg) by func_80065A5C */
    u8 *unk74;                     /* +0x74 parallel byte array (one byte per unk70 entry), allocated by func_80065E1C; linearly searched by func_80065D64 */
    void *unk78;                    /* +0x78 callback pointer; func_800660BC copies one of slot118/11C/120's VALUE (never calls it) here based on a small dispatch value */

    s32 unk7C;                      /* +0x7C set verbatim from func_80066214's `index` argument */
    s32 unk80;                       /* +0x80 set by func_80066214 from the resolved GroupObj's EntryObj2->unk4 */
    s32 unk84;                        /* +0x84 reset to 0 by func_80066214 -- looks like an iteration count paired with unk88 */
    u8 *unk88;                         /* +0x88 set by func_80066214 to (u8 *)entry + 8, then passed to methods->slot134 -- an iterator "current" pointer */

    s32 unk8C;                     /* +0x8C boolean-ish flag; set to 0 by func_80066148, to 1 (and returned) by func_8006613C */
    s32 unk90;                     /* +0x90 boolean-ish flag; set to 0 by func_800662B4, to 1 (and returned) by func_800662A8 */
    Class65650 *unk94;              /* +0x94 companion-object pointer; zeroed in the constructor, linked via slot10/slot14 in func_80066748/func_800667B0 (a symmetric buddy-link, not the s32 the first pass guessed) */
};

extern Class65650Methods D_8008A6C4;
extern Class65650Methods *func_80066818(void);

extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);
extern void func_8001E770(Class65650 *self, s32 arg);

/* Same-unit helpers called directly by name (still INCLUDE_ASM this round).
 * func_80065C5C/func_80065CEC are the +0x5C sub-object's setup/teardown
 * bodies (dispatched through slot_setup5C/slot_teardown5C); func_80065E1C/
 * func_80065F2C are the +0x70/+0x74 arrays' setup/teardown bodies
 * (dispatched through slot_setup70/slot_teardown70). Neither E1C nor F2C
 * reference their own $a1 anywhere in their bodies, so they take only
 * `self`; C5C does use its own second argument (a1->0xC), so it keeps one. */
extern s32 func_80065C5C(Class65650 *self, UnkArg1Obj *other);
extern void func_80065CEC(Class65650 *self);
extern s32 func_80065E1C(Class65650 *self);
extern void func_80065F2C(Class65650 *self);

#endif
