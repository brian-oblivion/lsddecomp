#ifndef CLASS_39E08_H
#define CLASS_39E08_H

#include "common.h"
#include "Class86668.h"

/* Round 73 head correction: slots +0x058..+0x06C (Noop58, CheckTimeout,
 * OnEventArg, SetTimeout) sit at the same offsets in BOTH gClass86668Methods
 * (0x80086668, 28 slots, the base) and Obj865C8's table (0x800865C8, 33
 * slots), and lie in ROM between Class86668's dtor and
 * Class86668__PlaySound: they are Class86668 methods that Obj865C8
 * inherits, so they carry the Class86668__ prefix. */
/*
 * round 73 (alpha), track-3 naming pass: 25 of this unit's 26 functions
 * renamed (func_8004A070 left as-is, see below); no INCLUDE_ASM remained to
 * carry over. All tier B unless noted.
 *
 * The class whose method table is D_800865C8 (33 slots, `Obj865C8` --
 * resolved with tools/classtable.py 0x800865C8 -- diff against 0x8006E878
 * to see the override set), plus the methods of its parent, Class86668
 * (gClass86668Methods, 0x230, whose header is include/Class86668.h; track 4,
 * round 84), from New_Class86668 on. Obj865C8 and Class865C8Methods below are
 * D_800865C8's own views and still spell out the parent's fields and slots
 * (a later track-4 job). No FirecatFG name survives for either class, so
 * fields are named by offset until real names are known.
 *
 * `Obj865C8__Obj865C8` (the ctor) constructs one `SubObjD` handed in by the
 * caller, one `SubObjB` VAB sound stream (`New_VabStreamObj`), and loads two
 * named resources verbatim from the ctor body -- "ETC\ETC.TIM" and
 * "ETC\DREAMER.TMD" (`D_800113EC`/`D_800113F8`) -- into `unk44`/`unk48`.
 * The object also carries a small state machine (`state`, 0-3, advanced by
 * `Obj865C8__AdvanceState`/`Obj865C8__OnTag2Notify`) and a frame-based
 * timeout (`frameCounter`/`timeoutFrames`, checked by
 * `Class86668__CheckTimeout`, set by `Class86668__SetTimeout`/
 * `Class86668__CancelTimeout`) whose in-game purpose is not established
 * beyond that mechanism. `Obj865C8__OnNotify` dispatches by a header tag
 * (0x1F34 / 0x2F230) to `Obj865C8__Noop80`/`Obj865C8__OnTag2Notify`
 * respectively; neither tag's game meaning is known.
 *
 * `func_8004A070` (this unit, called once from the ctor) is left unrenamed:
 * it manages a pair of file-scope globals (`D_8008A978`/`D_8008A97C`) and
 * loops on `RegisterFileTableEntries`, but nothing in its own body or its
 * one other caller (`code_1677c.c`) pins down what it is registering.
 *
 * Only the slots this unit's functions actually call through are given
 * concrete field types; the rest stay opaque `void *`/`u8 pad` so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/Class6D3C8.h).
 */
/* Tag-only forward declaration: Obj0C is fully defined further down, but
 * Class865C8Methods::ctor below needs to name it first. A tag-only decl
 * does not collide with the later `typedef struct Obj0C {...} Obj0C;`. */
struct Obj0C;
typedef struct Obj865C8 Obj865C8;
typedef struct Obj4C Obj4C;
typedef struct SubObjA SubObjA;
typedef struct SubObjD SubObjD;
typedef struct SubObjF SubObjF;

/* What Obj865C8__OnNotify (Class865C8Methods slot +0x038) inspects: `arg1` is a
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

/* +0x008 (ctor) and most BasicClass-inherited slots are not this round's
 * functions (Obj865C8__Obj865C8, still INCLUDE_ASM elsewhere in this unit) --
 * left untyped. */
typedef struct Class865C8Methods {
    s32 header;                                   /* +0x000 */
    void *unk04;                                   /* +0x004 BasicClass__Release */
    /* Typed because New_Obj865C8 (this unit) dispatches it as
     * GetObj865C8Methods()->ctor(self, arg1, arg2, arg3); the signature is
     * Obj865C8__Obj865C8's own, defined just below. */
    void (*ctor)(Obj865C8 *self, struct Obj0C *arg1, SubObjD *arg2, s32 arg3); /* +0x008 Obj865C8__Obj865C8 */
    void (*dtor)(Obj865C8 *self);                  /* +0x00C Obj865C8__Dtor */
    /* BasicClass-inherited (BasicClass__AddChild -- same address as
     * Class6D3C8.h's own local unk10 view of this same shared slot).
     * Called by Obj865C8__EnterState2 as self->methods->slot10(self, newObj). */
    void (*slot10)(Obj865C8 *self, Obj4C *arg1);   /* +0x010 */
    /* Called by Obj865C8__Dtor as self->methods->slot14(self, self->unk38). */
    void (*slot14)(Obj865C8 *self, SubObjD *arg1); /* +0x014 */
    void *unk18, *unk1C;                           /* BasicClass, inherited */
    void *unk20, *unk24, *unk28, *unk2C;           /* BasicClass, inherited */
    void *unk30, *unk34;                           /* BasicClass, inherited */
    /* Occupied here by Obj865C8__OnNotify itself; only reachable from THIS
     * struct via GetClass86668Methods() (Class86668Methods::onNotify,
     * include/Class86668.h), where it forwards to the inherited
     * IntermediateBase__OnNotify. */
    void (*slot38)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x038 Obj865C8__OnNotify */
    void *unk3C;                                   /* +0x03C null slot */
    void (*resetState)(Obj865C8 *self);            /* +0x040 Obj865C8__ResetState */
    void *slot44;                                  /* +0x044 Obj865C8__Init */
    void *slot48;                                  /* +0x048 Obj865C8__Deinit */
    void *slot4C;                                  /* +0x04C Obj865C8__StartSubA */
    void (*runSubUpdates)(Obj865C8 *self);         /* +0x050 Obj865C8__RunSubUpdates */
    void *slot54;                                  /* +0x054 Obj865C8__AdvanceState */
    void (*noop58)(void);                          /* +0x058 Class86668__NoOpSlot58 (no-op, matched) */
    void *slot5C;                                  /* +0x05C Class86668__CheckTimeout */
    /* Shared with gClass86668Methods (include/Class86668.h) -- literally the
     * same function address at the same offset in both tables. */
    void (*onEventArg)(Obj865C8 *self, s32 arg1);  /* +0x060 Class86668__SetState */
    void *unk64, *unk68;                           /* shared base slots (IntermediateBase__OnState2 / IntermediateBase__OnState3) */
    /* Also shared with gClass86668Methods at the same offset. */
    void (*setTimeout)(Obj865C8 *self, s32 arg1);    /* +0x06C Class86668__SetTimeout */
    void *unk70;                                   /* Class86668__PlaySound */
    void *unk74, *unk78;                           /* null slots */
    void (*noop7C)(Obj865C8 *self);                /* +0x07C Obj865C8__Noop7C (no-op, matched) */
    /* Retyped from `void (*noop80)(void)`: Obj865C8__OnNotify dispatches this
     * slot as `self->methods->slot80(self, arg1, arg2)` with real
     * arguments loaded into $a1/$a2 -- Obj865C8__Noop80 (D_800865C8's own
     * occupant, still matched, `void Obj865C8__Noop80(void) {}`) simply
     * ignores them. A no-op BODY is not evidence the SLOT's signature takes
     * no arguments; only THIS slot's other occupants would be. */
    void (*slot80)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x080 Obj865C8__Noop80 (no-op body, matched) */
    /* Same signature as slot80 by the same call site (Obj865C8__OnNotify's other
     * branch); occupant Obj865C8__OnTag2Notify is still addiu_at-blocked. */
    void (*slot84)(Obj865C8 *self, EventArg *arg1, s32 arg2); /* +0x084 Obj865C8__OnTag2Notify, addiu_at-blocked */
} Class865C8Methods;

extern Class865C8Methods D_800865C8;

/* Opaque view of whatever object Obj865C8::subA points to (used only by
 * Obj865C8__RunSubUpdates): its own vtable pointer sits at offset 0, and only the two
 * slots Obj865C8__RunSubUpdates dispatches through are named here -- declared
 * minimally, locally, for this one call site (same policy as
 * DreamSysEntityObj in include/DreamSys.h). */
typedef struct SubObjAMethods {
    u8 pad00[0x44];
    void (*slot44)(SubObjA *self, s32 arg1);                /* +0x044, Obj865C8__StartSubA */
    u8 pad48[0x4C - 0x48];
    void (*slot4C)(SubObjA *self, s32 arg1);                /* +0x04C, Obj865C8__StartSubA */
    u8 pad50[0x70 - 0x50];
    /* Obj865C8__StartSubA's 5-arg call: 4 register args plus a literal 0 in the
     * 5th (stack) slot. arg1/arg2/arg3 types are just "address taken, never
     * dereferenced here" -- SubObjD* for arg1 because that's what
     * Obj865C8::unk38 already is, void* for the two rodata symbol
     * addresses (arg2/arg3, real element type unknown). */
    void (*slot70)(SubObjA *self, SubObjD *arg1, void *arg2, void *arg3, s32 arg4); /* +0x070 */
    void (*slot74)(void *self);
    u8 pad78[0x8C - 0x78];
    void (*slot8C)(SubObjA *self);                          /* +0x08C, Obj865C8__StartSubA */
    void (*slot90)(void *self);
    u8 pad94[0xAC - 0x94];
    /* Returns an object with its OWN 1-slot-known vtable (SubObjF below);
     * Obj865C8__StartSubA immediately dispatches the result's own +0x060. */
    SubObjF *(*slot0xAC)(SubObjA *self);                    /* +0x0AC, Obj865C8__StartSubA */
} SubObjAMethods;
struct SubObjA {
    SubObjAMethods *methods;
};

/* Opaque view of whatever object SubObjAMethods::slot0xAC returns (used
 * only by Obj865C8__StartSubA): same "vtable at offset 0, only the reached slot
 * named" policy as SubObjE above. */
typedef struct SubObjFMethods {
    u8 pad00[0x60];
    void (*slot60)(SubObjF *self, s32 arg1);
} SubObjFMethods;
struct SubObjF {
    SubObjFMethods *methods;
};

/* Opaque view of whatever object Obj865C8::subB points to (used only by
 * Class86668__Finalize, guarded by Obj865C8::unk30): same "vtable at offset 0,
 * only slot +0x004 named" policy. */
typedef struct SubObjBMethods {
    u8 pad00[0x04];
    void (*slot4)(void *self);
} SubObjBMethods;
typedef struct SubObjB {
    SubObjBMethods *methods;
} SubObjB;

/* Opaque view of whatever object New_ObjM (uncarved, unit class_3bb8c)
 * returns and stores at Obj865C8::unk4C (used only by Obj865C8__EnterState2): same
 * "vtable at offset 0, only the one dispatched slot named" policy as
 * SubObjA/SubObjB above. */
typedef struct Obj4CMethods {
    u8 pad00[0x4];
    void (*slot4)(Obj4C *self);                       /* +0x004, Obj865C8__AdvanceState, return discarded */
    u8 pad8[0x44 - 0x8];
    void (*slot44)(Obj4C *self, s32 arg1, s32 arg2);
    void (*slot48)(Obj4C *self);                       /* +0x048, Obj865C8__AdvanceState, return discarded */
} Obj4CMethods;
struct Obj4C {
    Obj4CMethods *methods;
};

/* Opaque view of whatever object Obj0C::obj (below) points to (used only by
 * Obj865C8__StartSubA): same "vtable at offset 0, only the reached slot named"
 * policy as SubObjA/SubObjB/Obj4C. */
typedef struct SubObjE SubObjE;
typedef struct SubObjEMethods {
    u8 pad00[0x7C];
    s32 (*slot7C)(SubObjE *self, s32 arg1);
} SubObjEMethods;
struct SubObjE {
    SubObjEMethods *methods;
};

/* Opaque view of an object family that gets "stepped" through a single
 * self-consuming call, `obj = obj->methods->slot4(obj)` -- confirmed at SIX
 * independent call sites in Obj865C8__Dtor alone (three `Obj0C` fields below,
 * plus `Obj865C8::unk40/unk44/unk48`), all identical shape. Same "vtable at
 * offset 0, only the reached slot named" policy as SubObjA/SubObjB/Obj4C. */
typedef struct SubObjG SubObjG;
typedef struct SubObjGMethods {
    u8 pad00[0x04];
    SubObjG *(*slot4)(SubObjG *self);
    u8 pad8[0x5C - 0x8];
    /* Obj865C8__Obj865C8 (ctor): called on a just-constructed instance,
     * return discarded. */
    void (*slot5C)(SubObjG *self);         /* +0x05C */
    u8 pad60[0x78 - 0x60];
    /* Obj865C8__Obj865C8 (ctor): called immediately after construction on
     * self->unk44, return discarded. */
    void (*slot78)(SubObjG *self);         /* +0x078 */
} SubObjGMethods;
struct SubObjG {
    SubObjGMethods *methods;
};

/* Opaque view of whatever object Obj865C8::unk0C points to (used by
 * Obj865C8__Deinit/Obj865C8__Init, which read its own +0x004/+0x008/+0x010
 * fields, Obj865C8__StartSubA, which dereferences +0x000, and Obj865C8__Dtor,
 * which dereferences +0x008/+0x00C/+0x010 as `SubObjG *` -- no vtable
 * dispatch through Obj0C ITSELF, so no methods pointer is declared for
 * Obj0C; its own fields point at other objects that have one).
 *
 * +0x008/+0x010 were typed `s32` from Obj865C8__Init/Obj865C8__Deinit alone,
 * which only ever forward them as opaque register values through a vtable
 * call that never dereferences them -- consistent with EITHER a scalar or a
 * pointer. Obj865C8__Dtor dereferences both directly (`->methods->slot4`),
 * settling it: they are `SubObjG *`. Obj865C8__Init's own forwarding call
 * sites got an explicit `(s32)` cast rather than staying wrong. */
typedef struct Obj0C {
    SubObjE *obj;                 /* +0x000, Obj865C8__StartSubA */
    s32 unk4;                     /* +0x004 -- untouched by Obj865C8__Dtor, still unconfirmed either way */
    SubObjG *unk8;                /* +0x008, Obj865C8__Dtor (was s32) */
    SubObjG *unkC;                 /* +0x00C, Obj865C8__Dtor (new) */
    SubObjG *unk10;                /* +0x010, Obj865C8__Dtor (was s32) */
} Obj0C;

/* Opaque view of whatever object Obj865C8::unk38 points to (used by
 * Obj865C8__Deinit/Obj865C8__Init): same "vtable at offset 0, only the reached
 * slots named" policy as SubObjA/SubObjB/Obj4C above. */
typedef struct SubObjDMethods {
    u8 pad00[0x10];
    void (*slot10)(SubObjD *self, s32 arg1);
    void (*slot14)(SubObjD *self, s32 arg1);
    u8 pad18[0x10C - 0x18];
    /* Obj865C8__Obj865C8 (ctor): called as arg2->methods->slot10C(arg2,
     * self->subB). */
    void (*slot10C)(SubObjD *self, SubObjB *arg1);   /* +0x10C */
    void (*slot110)(SubObjD *self, s32 arg1);
    /* Obj865C8__Obj865C8 (ctor): called as arg2->methods->slot114(arg2,
     * self->unk44). */
    void (*slot114)(SubObjD *self, SubObjG *arg1);   /* +0x114 */
    u8 pad118[0x1B4 - 0x118];
    /* Obj865C8__AdvanceState's `case 1`: return value used (`>= 0` check), so this
     * is genuinely non-void. */
    s32 (*slot1B4)(SubObjD *self);                /* +0x1B4 */
    /* +0x1B8. RETYPED void -> s32 in round 23: Obj865C8__OnTag2Notify branches on the
     * return value (`bnez $v0` straight off the `jalr`), which is positive
     * evidence the slot is non-void. Obj865C8__AdvanceState, the other caller in this
     * unit and already matched, DISCARDS it -- so this is the round-7 shared-slot
     * hazard; re-verified byte-exact after the retype (whole-image SHA1). */
    s32 (*slot1B8)(SubObjD *self, s32 arg1);
    /* +0x1BC. Returns an 8-byte struct BY VALUE. GCC 2.6.3 returns any struct
     * through a hidden pointer passed as the invisible FIRST argument, which is
     * why Obj865C8__OnTag2Notify's call site reads `(&buf, sub)` and not `(sub)` -- the
     * object is arg2 in the bytes. The struct's own shape is a unit-local view
     * (SubObjDPos in src/class_39e08.c); only its size and the s16 at +2 are
     * established. Pad split below is additive and preserves the 0x24 total. */
    struct SubObjDPos (*slot1BC)(SubObjD *self);
    u8 pad1C0[0x1E0 - 0x1C0];
    /* Obj865C8__AdvanceState's `case 3`: return value used (forwarded straight into
     * Obj865C8__EnterState2's own 2nd argument). */
    s32 (*slot1E0)(SubObjD *self);                 /* +0x1E0 */
} SubObjDMethods;
struct SubObjD {
    SubObjDMethods *methods;
};

/* 0x50 bytes (New_Obj865C8). Offsets +0x000..+0x037 are Class86668's
 * (include/Class86668.h: result, timeoutFrames, soundBankPath and sound are
 * this view's eventCode, timeoutFrames, unk30 and subB). Field offsets are
 * only the ones this unit's functions touch. */
struct Obj865C8 {
    Class865C8Methods *methods;   /* +0x000 */
    u8 pad04[0x0C - 0x04];
    Obj0C *unk0C;                 /* +0x00C, Obj865C8__Deinit dereferences (->unk4); passed
                                      through as a plain register value to
                                      unk4C->methods->slot44's 2nd arg (IntermediateBase's init slot) by Obj865C8__EnterState2 */
    s32 unk10;                    /* +0x010, Obj865C8__Deinit (2nd arg to a slot14 call) */
    u8 pad14[0x18 - 0x14];
    SubObjA *subA;                /* +0x018, Obj865C8__RunSubUpdates */
    /* +0x01C. Renamed from `unk1C`: code_2cc8c.h's own local view of this
     * SAME base offset (then Obj86B60, TaskCore's view; IntermediateBase.h now) names it
     * `frameCounter` (round 12, "a running count"). Class86668__CheckTimeout
     * reads it directly after forwarding to the base's own +0x05C
     * (IncrementFrameCounter), and compares it against `timeoutFrames`. */
    s32 frameCounter;              /* +0x01C, Class86668__CheckTimeout */
    u8 pad20[0x28 - 0x20];
    /* +0x028. Renamed from `unk28`: carries a small code (1/2/3) that
     * Obj865C8__OnTag2Notify and Class86668__SetState set before dispatching
     * `self->methods->onEventArg(self, 3)` -- an event/result code, not a
     * state (see `state` below, a separate field). */
    s32 eventCode;                 /* +0x028, Class86668__SetState */
    /* +0x02C. Renamed from `unk2C`: set only by Class86668__SetTimeout, which
     * stores its argument verbatim if negative (disabled) or multiplied by
     * 20 otherwise (a units-to-frames conversion); compared against
     * `frameCounter` by Class86668__CheckTimeout. */
    s32 timeoutFrames;             /* +0x02C, Class86668__SetTimeout */
    s32 unk30;                    /* +0x030, Class86668__Finalize (guard) */
    SubObjB *subB;                /* +0x034, Class86668__Finalize */
    SubObjD *unk38;                /* +0x038, Obj865C8__Deinit dereferences (->methods); passed
                                       through as a plain register value to
                                       unk4C->methods->slot44's 3rd arg (IntermediateBase's init slot) by Obj865C8__EnterState2 */
    /* +0x03C. Renamed from `unk3C`: the class's own small state machine
     * (0/1/2/3), reset by Obj865C8__ResetState and advanced by
     * Obj865C8__AdvanceState/Obj865C8__OnTag2Notify. */
    s32 state;                     /* +0x03C, Obj865C8__ResetState */
    /* Retyped from `s32` (Obj865C8__EnterState2's own usage only ever forwards
     * these as opaque register values into New_ObjM, never
     * dereferencing them): Obj865C8__Dtor dereferences all three directly
     * as `SubObjG *` (`self->unkNN->methods->slot4(self->unkNN)`, result
     * discarded). Obj865C8__EnterState2's call site got an explicit `(s32)` cast. */
    SubObjG *unk40;                /* +0x040, Obj865C8__EnterState2 (2nd arg to New_ObjM), Obj865C8__Dtor */
    SubObjG *unk44;                /* +0x044, Obj865C8__EnterState2 (3rd arg to New_ObjM), Obj865C8__Dtor */
    SubObjG *unk48;                /* +0x048, Obj865C8__EnterState2 (4th arg to New_ObjM), Obj865C8__Dtor */
    Obj4C *unk4C;                 /* +0x04C, Obj865C8__EnterState2 -- result of New_ObjM */
};

/* The parent class of D_800865C8 (and of D_80087034) is Class86668
 * (gClass86668Methods): include/Class86668.h (track 4). Obj865C8's fields up
 * to +0x038 and its table's slots up to +0x07C are that class's. */

/* Allocator in the still-uncarved unit class_3bb8c (asm/class_3bb8c.s):
 * allocates an 0x88-byte instance, ctors it, and dispatches its own slot
 * +0x008 with the 5 forwarded arguments, returning the new instance (or 0
 * on allocation failure). Only the one call site here (Obj865C8__EnterState2)
 * cares about its signature; `a0`'s type is inherited from whatever the
 * caller actually passes (this unit's own `SubObjB *`), the remaining
 * scalar args are untyped beyond their register width. */
extern Obj4C *New_ObjM(SubObjB *a0, s32 a1, s32 a2, s32 a3, s32 a4);

/* Defined in this unit at ROM order 0x3A860, i.e. AFTER New_Obj865C8,
 * which dispatches through it -- so it needs a prototype here. */
extern Class865C8Methods *GetObj865C8Methods(void);

/* BasicClass-family allocator; see code_171e0.h / code_55dd4.h / Entity.h /
 * class_16334.h for the other units that also declare it locally. */
extern void *BMemPMgrAlloc(s32 size);

/* VabStreamObj's allocator (src/code_179d8_e.c): a 0x64-byte instance ctored
 * with the one forwarded path. Only call site here is Class86668__Class86668,
 * which stores the result in Class86668::sound. */
extern BasicClass *New_VabStreamObj(char *path);

/* Rodata symbols right next to this unit's own gClass86668Methods/D_800865C8
 * vtables (0x80086650, 0x8008665C -- 0x18 and 0xC bytes before gClass86668Methods
 * respectively). Only their ADDRESSES are taken, as the 2nd/3rd args to
 * SubObjAMethods::slot70 (Obj865C8__StartSubA); real element type/size unknown. */
extern u8 D_80086650[];
extern u8 D_8008665C[];

/* Matched in code_4cd08.c, called here as a plain global (per CLAUDE.md's
 * "calling into a function that is still INCLUDE_ASM in another unit is
 * fine" convention, extended to an already-matched cross-unit function:
 * only a local extern prototype is needed). No return value used. */
extern void TickDreamAuxSlots(void);

/* The ctor, Obj865C8__Obj865C8. A large web of external calls; each is declared
 * locally with the minimal signature its call site here demonstrates (per
 * CLAUDE.md's "calling into a function that is still INCLUDE_ASM elsewhere
 * is fine" convention -- several of these ARE already declared, with a
 * different but ABI-compatible return type, in OTHER units' own local
 * views; this unit keeps its own, per the project's established
 * multiple-independent-local-views policy). */

/* MATCHED, src/code_39094.c (`char *GetSoundEffectDir(void)`): returns the
 * "SND\\SE" directory string pointer; forwarded as the sibling ctor's arg1
 * (which that ctor stores directly into `Obj865C8::unk30`, an `s32`). */
extern s32 GetSoundEffectDir(s32 arg1); /* arity-ok: the definition takes no parameter and reads no argument register, but this dead argument IS byte-load-bearing -- retail emits `move a0,zero` at 0x800496A8 ahead of the jal at 0x800496B0 */

/* Matched in code_4cd08.c (still called `InitDreamAux` there, STALLED at
 * 40/56 -- see docs/match-reports/InitDreamAux.md). Takes no arguments,
 * return value (if any) unused here. */
extern void InitDreamAux(void);

/* "New_X"-shaped allocator (uncarved, in the Psy-Q SPU/SND block at
 * 0x272C8..0x2C054): allocates,
 * ctors with the one forwarded argument, returns the new instance (or 0).
 * Stored into `Obj865C8::unk44` here, which this function's own body then
 * immediately dispatches through `SubObjGMethods::slot78`/`slot5C` --
 * consistent with the existing `SubObjG` family. */
extern SubObjG *func_8003B39C(const char *path);

/* Filenames right next to each other in the same rodata blob
 * (asm/data/1A90.rodata.s): "ETC\\ETC.TIM" and "ETC\\DREAMER.TMD". */
extern const char D_800113EC[];
extern const char D_800113F8[];

/* Same request-block shape `Class6D3C8.h` already established at
 * `Class6D3C8__Class6D3C8`'s call site (`LoadModelRequest`, learned there to be
 * 0x10 bytes even though only the first two fields are ever written --
 * "local struct SIZE matters, not shape"). This unit's own local view,
 * not a shared header, per this project's per-unit-view convention. */
typedef struct LoadRequest {
    s32 type;
    const char *path;
    s32 unk08;
    s32 unk0C;
} LoadRequest;

/* Uncarved, asm/psyq_memset.s: loads a resource named by `req->path`,
 * returns a handle/object. Already declared elsewhere (Class6D3C8.h) as
 * `extern void *New_LinkResource(void *arg)`; this unit's own local view types
 * the return `SubObjG *` to match where it's stored here
 * (`Obj865C8::unk48`). */
extern SubObjG *New_LinkResource(LoadRequest *req);

/* Uncarved, asm/psyq_memset.s: a magic-multiply division idiom over its one
 * argument (not an allocator -- no `BMemPMgrAlloc` call in its own body).
 * Its return value is forwarded as `New_WBgm`'s own 1st argument. */
extern s32 PickWeeklyGroup(s32 arg1);

/* "New_X"-shaped allocator (uncarved, in the Psy-Q SPU/SND block at
 * 0x272C8..0x2C054), 3 forwarded
 * arguments. Stored into `Obj865C8::unk40`. */
extern SubObjG *New_WBgm(s32 arg1, s32 arg2, s32 arg3);

/* Already declared elsewhere (code_1677c.c) as `extern s32
 * func_8004A070(s32 a0)`; this unit's own local view, same signature.
 * Return value discarded at this call site. */
extern s32 func_8004A070(s32 arg1);

/* Already declared elsewhere (Class6D3C8.h) with this exact signature; this
 * unit's own local view. Return value discarded at this call site. */
extern s32 SetActiveDataSourceDriverMode(s32 arg1, s32 arg2, s32 arg3);

/* "New_X"-shaped allocator (uncarved, asm/class_3bb8c.s), zero forwarded
 * arguments (its own ctor call passes only the new instance). Stored into
 * `Obj0C::unk10`. */
extern SubObjG *New_Class869D8(void);

/* Same "New_X" shape, zero arguments (uncarved, asm/psyq_memset.s). Stored
 * into `Obj0C::unk8`. */
extern SubObjG *New_D8006EF50(void);

/* Already declared, fully typed, in class_3ac78.h as `Class866E8
 * *New_Class866E8(s32, s32)` (an established New_X allocator for a
 * DIFFERENT, richer-typed class). Stored into `Obj0C::unkC` here, which
 * this unit's own local view types `SubObjG *` -- an explicit cast is used
 * at the one call site rather than importing class_3ac78's type, per this
 * project's per-unit-view convention (same policy as the other externs on
 * this page). */
extern void *New_Class866E8(s32 arg1, s32 arg2);

#endif
