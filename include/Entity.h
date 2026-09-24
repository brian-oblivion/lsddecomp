#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"

/* The Entity class. `src/Entity.c` is the first 25 of a 142-function block
 * split at Entity__UpdateTargetProximity; the remainder is Entity_b, still a monolithic asm
 * segment. Its own vtable is fetched via `Get_vtable_Entity` (still asm,
 * address 0x8005E150, per config/symbols.slps01556.lsdde.txt -- New_Entity's
 * disassembly calls it directly by name, `jal Get_vtable_Entity`, the same
 * `Get_vtable_X`-named-accessor shape as `Get_vtable_DreamSys` in
 * docs/research/class-framework.md). Only the vtable slots this unit's
 * queued functions actually dispatch through are typed below; the struct is
 * NOT padded out to its full size (New_Entity allocates 0x108 bytes, but
 * nothing here needs the whole layout).
 *
 * The vtable itself is `ENTITY_METHODS` (asm/data/79528.data.s, offsets 0x000..
 * 0x180) and it doubles as a legible cross-check: whichever function each
 * slot currently HOLDS is the function that OVERRIDES/OCCUPIES that slot,
 * not the function that CALLS it. Do not confuse the two -- the "called by"
 * comments on `notifyParents`/`slot60`/`slot114`/`slot130` below name a CALLER
 * of that slot (found by reading that caller's own disassembly), while
 * "this unit's own slot" comments elsewhere name the function occupying a
 * slot in the table. Early slots (+0x004, +0x010, +0x014, +0x01C..+0x038)
 * hold `BasicClass__func_*`/`BaseObjO__LinkCompanion`/`BaseObjO__UnlinkCompanion` -- the SAME
 * addresses `code_55dd4.h`'s `Class65650Methods` holds at its own +0x004/
 * +0x010/+0x014, confirming Entity and Class65650 share the identical
 * "BasicClass" ancestor and its vtable layout convention (ctor at +0x008,
 * dtor at +0x00C universally). `Get_vtable_Class65650()` (matched in code_55dd4.c)
 * returns that SHARED ancestor's own vtable (`&gClass65650Methods`) directly -- so
 * a `Get_vtable_Class65650()->slotNN(...)` call from Entity's own functions is a
 * call into a function inherited from the same base as Class65650, not a
 * Class65650-specific call, even though the accessor's name and declared
 * return type come from that unit. `BasicClassMethods` below is Entity's
 * OWN minimal, local view of exactly that shared table -- only the slots
 * this unit's functions actually reach through it, independent of (and not
 * editing) `code_55dd4.h`'s own `Class65650Methods` view of the same table.
 */
typedef struct Entity Entity;
typedef struct EntityMethods EntityMethods;
typedef struct Unk100Obj Unk100Obj;
typedef struct Unk100Methods Unk100Methods;
typedef struct Unk94Obj Unk94Obj;
typedef struct Unk94Methods Unk94Methods;
typedef struct Unk5CObj Unk5CObj;
typedef struct Unk5CMethods Unk5CMethods;
typedef struct Unk4CObj Unk4CObj;
typedef struct Unk4CMethods Unk4CMethods;
typedef struct Unk70Obj Unk70Obj;
typedef struct Unk70Sub Unk70Sub;
typedef struct Unk70SubMethods Unk70SubMethods;
typedef struct BasicClassMethods BasicClassMethods;
typedef struct EntityPos EntityPos;
typedef struct EntityMoodRow EntityMoodRow;
typedef struct EntityRegionSlot EntityRegionSlot;
typedef struct EntityRegionRef EntityRegionRef;

struct EntityMethods {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *unk04;
    /* +0x08 */ void *(*ctor)(Entity *self, void *arg0, void *arg1, void *arg2); /* New_Entity's call */
    /* +0x0C */ u8 pad0C[0x30 - 0x0C];
    /* +0x30 */ void (*notifyParents)(Entity *self, s32 arg1);   /* called by Entity__SetUnkF4, Entity__NotifyIfTargetInRange */
    /* +0x34 */ u8 pad34[0x40 - 0x34];
    /* +0x40 */ void (*initState)(Entity *self);              /* self-only slot: occupant is Entity__InitState (tools/classtable.py), called by Entity__Entity right after this->methods is (re)assigned */
    /* +0x44 */ void (*updateRotation)(Entity *self, s32 arg1, void *arg2); /* called by Entity__MoodCue07 as slot44(this, 0, ROTATION_YAW_PLUS2) */
    /* +0x48 */ s32 (*updateScale)(Entity *self, s32 arg1, void *arg2); /* called by Entity__MoodCue08 (result discarded) and Entity__MoodCue17 (a tail call that returns it), both with arg1==1 -- see CLAUDE.md's "one-line wrapper" rule, Entity__MoodCue17 has no positive evidence of void */
    /* +0x4C */ u8 pad4C[0x60 - 0x4C];
    /* +0x60 */ void (*slot60)(Entity *self, s32 arg1);   /* called by Entity__Activate, Entity__Deactivate */
    /* +0x64 */ u8 pad64[0x70 - 0x64];
    /* +0x70 */ void (*slot70)(Entity *self, s32 arg1);   /* called by Entity__InitState as slot70(this, 1), gated on `gEntityUnlockKindTable[this->moodIndex*0x10]-1` being unsigned-less-than 9 */
    /* +0x74 */ u8 pad74[0xB8 - 0x74];
    /* +0xB8 */ void (*setVec14)(Entity *self, void *arg1);  /* called by Entity__MoodCue30 as setVec14(this, &this->target->unk14->x) -- a vector-pointer argument, same shape as Entity__IsNearTarget's still-INCLUDE_ASM arg1 */
    /* +0xBC */ void (*addVec14)(Entity *self, void *arg1);  /* called by Entity__MoodCue08 */
    /* +0xC0 */ u8 padC0[0xC4 - 0xC0];
    /* +0xC4 */ void (*slotC4)(Entity *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue10/Entity__MoodCue13/Entity__MoodCue14/Entity__MoodCue00/etc (all discard the result) and Entity__MoodCue32 (its tail call) -- same shared BasicClass-inherited slot as Class65650Methods.slotC4 in code_55dd4.h (both tables hold BaseObjO__func_5748c at +0xC4, confirmed with tools/classtable.py). CLAUDE.md's "one-line wrapper" rule would normally push this toward s32-returning on Entity__MoodCue32's strength alone -- but retyping it to s32 changes Entity__MoodCue00's OWN codegen (verified: GCC stops tail-merging its two identical slotC4(this,0x32,0)-reached-from-different-branches call sites, costing that ALREADY-MATCHED function 4 words and shifting every later function in the unit). Entity__MoodCue00's own bytes are the stronger, more direct evidence and they require void. Kept void; Entity__MoodCue32 is void too, with a bare statement call, not `return`. (slotCC below faced the same question and was independently verified NOT to have this problem.) */
    /* +0xC8 */ void (*slotC8)(Entity *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue35 (discards the result); same (self,arg1,arg2) shape as slotC4/slotCC, no other caller yet so kept void by default like slotC4 */
    /* +0xCC */ void (*slotCC)(Entity *self, s32 arg1, s32 arg2); /* RETYPED BACK s32 -> void, round 68, and this one is settled by retail's own bytes rather than by a convention. It had been s32 on the strength of Entity__MoodCue37 (Entity_c) being a lone `return this->methods->slotCC(this, -0x5A, 0);` wrapper -- CLAUDE.md's "one-line wrapper" rule. But Entity__MoodCue37 compiles BYTE-IDENTICALLY as `void Entity__MoodCue37(Entity *this) { this->methods->slotCC(this, -0x5A, 0); }`, so its bytes were never evidence either way, while Entity__MoodCue115 (Entity_d) has POSITIVE evidence for void: retail tail-merges its slotCC call with sibling slot134/slotC4 calls that are void, and GCC 2.6.3 cannot cross-jump a `(set v0 (call ...))` against a bare `(call ...)`. s32 cost that function 6 words and was the whole of its two-round stall; void matches it 198/198. Same mechanism as slotC4 above, measured in the opposite direction -- see docs/match-reports/Entity__MoodCue115.md. */
    /* +0xD0 */ void (*slotD0)(Entity *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue16 as slotD0(this, -0x176, rand() % 2), and by Entity__MoodCue11 as slotD0(this, this->unk48, 0) */
    /* +0xD4 */ u8 padD4[0x10C - 0xD4];
    /* +0x10C */ void (*slot10C)(Entity *self, s32 arg1);  /* called by Entity__InitState as slot10C(this, 0x42), unconditionally */
    /* +0x110 */ void (*slot110)(Entity *self);            /* called by Entity__StartSoundCue */
    /* +0x114 */ void (*slot114)(Entity *self);           /* called by Entity__StopSoundCue */
    /* +0x118 */ u8 pad118[0x128 - 0x118];
    /* +0x128 */ void (*slot128)(Entity *self, s32 arg1); /* called by Entity__MoodCue77 (Entity_e) as slot128(this, 1); return value unused at this, its only known call site, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    /* +0x12C */ void (*slot12C)(Entity *self);           /* called by Entity__MoodCue42 */
    /* +0x130 */ void (*slot130)(Entity *self);           /* called by Entity__StopSoundCue, Entity__MoodCue42 */
    /* +0x134 */ s32 (*slot134)(Entity *self, s32 arg1, s32 arg2); /* called by func_80063ED4 and func_80064078 (both Entity_f) in an identical loop, `this->unk88 = slot134(this, this->unk88, 0)` while `this->unk84++ < 0x18` -- value-returning, not void */
    /* +0x138 */ u8 pad138[0x144 - 0x138];
    /* +0x144 */ s32 (*slot144)(Entity *self, Unk94Obj *arg1); /* called by Entity__IsTargetInRange, as slot144(this, this->target) -- arg1 stays live in $a1 from its own first use all the way to this call, which is WHY retail keeps this->target in $a1 rather than a scratch register (see the match report's now-superseded "register identity" stall write-up); compared with slt -- value-returning, not void */
    /* +0x148 */ s32 (*getProximityRatio)(Entity *self);            /* called by Entity__MoodCue05; holds Entity__GetProximityRatio (this unit, MATCHED round 44) */
    /* +0x14C */ u8 pad14C[0x15C - 0x14C];
    /* +0x15C */ void (*activate)(Entity *self);            /* self-only slot: occupant is Entity__Activate (tools/classtable.py -- a self-referential vtable dispatch, same idiom initState/slot60/etc. use throughout this table). Called by Entity__AttachUnk4C (unconditionally once its two per-mood skip-flag gates pass) and by Entity__UpdateActivationState (when its detachKind-derived condition fires) */
    /* +0x160 */ void (*deactivate)(Entity *self);             /* CROSS-UNIT (Entity_c/d/e/f/g also dispatch through this slot -- see docs/match-reports/Entity__Deactivate.md's Proposed field names) -- occupant is Entity__Deactivate, same self-referential idiom as slot15C above. Called by Entity__DetachUnk4C, Entity__NotifyReset, Entity__UpdateDeactivationState */
    /* +0x164 */ void (*setUnkF4)(Entity *self, s32 arg1);    /* called by Entity__Deactivate and Entity__UpdateTargetProximity (as slot164(self, 1)) */
    /* +0x168 */ void (*startSoundCue)(Entity *self);               /* called by Entity__UpdateSoundCueStart */
    /* +0x16C */ void (*stopSoundCue)(Entity *self);                /* called by Entity__Deactivate, Entity__UpdateSoundCueStop, Entity__MoodCue77 (Entity_e) */
    /* +0x170 */ s32 (*activationState)(Entity *self);                  /* self-only slot: occupant is Entity__UpdateActivationState, called by Entity__Update as `if (this->methods->activationState(this) != 0) ...` */
    /* +0x174 */ s32 (*deactivationState)(Entity *self);                  /* self-only slot: occupant is Entity__UpdateDeactivationState, which returns s32 (`this->unkF0`) -- retyped from `void` to `s32` to match (CLAUDE.md: a discarded return, which is all Entity__Update does with it, is never evidence of void). Retype re-verified byte-exact; this slot has no other caller to perturb. Called by Entity__Update */
    /* +0x178 */ s32 (*updateTargetProximity)(Entity *self);                    /* called by Entity__Update; holds Entity__UpdateTargetProximity, which ends `return this->unkF4;` -- NOT void despite the one known caller discarding it, see CLAUDE.md's "discarded return is never evidence of void" */
    /* +0x17C */ s32 (*updateSoundCueStart)(Entity *self);                     /* called by Entity__Update; holds Entity__UpdateSoundCueStart, which ends `return this->unkF8;` */
    /* +0x180 */ void (*updateSoundCueStop)(Entity *self);                     /* called by Entity__Update */
};

/* Entity's own local view of the shared "BasicClass" ancestor vtable
 * returned by `Get_vtable_Class65650()` (matched in code_55dd4.c/code_55dd4.h,
 * which owns the canonical `Class65650Methods` view of this SAME table --
 * see the big comment above). Only the offsets this unit's functions reach
 * through it are named; everything else is inherited/not-yet-needed
 * padding, same convention as `Class65650Methods`. */
struct BasicClassMethods {
    /* +0x000 */ u8 pad00[0x08];
    /* +0x008 */ void *(*ctor)(void *self, s32 arg1, s32 arg2); /* Entity__Entity's base-class construction call */
    /* +0x00C */ void (*dtor)(void *self);                       /* called by Entity__Destructor */
    /* +0x010 */ u8 pad10[0x4C - 0x10];
    /* +0x04C */ void (*slot4C)(void *self, s32 arg1, s32 arg2, void *arg3, s32 arg4); /* called by Entity__AttachUnk4C, forwarding (arg1, arg2, arg3, arg4) straight through; arg3 is also stored into this->unk4C by the caller right after, so it is Unk4CObj* at that call site even though this shared ancestor slot takes it opaquely */
    /* +0x050 */ void (*slot50)(void *self);                       /* called by Entity__DetachUnk4C */
    /* +0x054 */ u8 pad54[0x98 - 0x54];
    /* +0x098 */ void (*slot98)(void *self, s32 arg1, s32 arg2);      /* called by Entity__Update */
    /* +0x09C */ u8 pad9C[0xDC - 0x9C];
    /* +0x0DC */ void (*slotDC)(void *self, s32 arg1, s32 arg2);      /* called by Entity__NotifyLinkStage, forwarding (arg1, arg2) straight through */
    /* +0x0E0 */ void (*slotE0)(void *self, s32 arg1, s32 arg2);        /* called by Entity__NotifyReset */
};

extern BasicClassMethods *Get_vtable_Class65650(void);

/* An object cached in `Entity::unk100`/`unk104`, unrelated to `EntityMethods`
 * -- its own method table, dispatched through in Entity__Destructor/Entity__GetOrCreateUnk100.
 * Real shape unknown beyond the slots reached here. */
struct Unk100Methods {
    /* +0x00 */ u8 pad00[0x04];
    /* +0x04 */ void (*slot04)(Unk100Obj *self);                          /* called by Entity__Destructor */
    /* +0x08 */ u8 pad08[0x4C - 0x08];
    /* +0x4C */ void (*slot4C)(Unk100Obj *self, Entity *arg1, void *arg2); /* called by Entity__GetOrCreateUnk100 */
    /* +0x50 */ void (*slot50)(Unk100Obj *self);                           /* called by Entity__GetOrCreateUnk100 */
    /* +0x54 */ u8 pad54[0xD0 - 0x54];
    /* +0xD0 */ void (*slotD0)(Unk100Obj *self, void *arg1);                /* called by Entity__GetOrCreateUnk100 */
    /* +0xD4 */ void (*slotD4)(Unk100Obj *self, s32 arg1, s32 arg2, s32 arg3); /* called by Entity__MoodCue57 (Entity_d) as slotD4(this->unk100, this->unk50, 4, 0), and by func_80063874 (Entity_f) as slotD4(this->unk100, this->unk50, 7, 0) */
    /* +0xD8 */ void (*slotD8)(Unk100Obj *self, s32 arg1, s32 arg2, s32 arg3); /* called by func_80063874 (Entity_f) as slotD8(this->unk100, this->unk50, 0, 0); return value unused at this, its only known call site */
};

struct Unk100Obj {
    Unk100Methods *methods; /* +0x00 */
};

extern Unk100Obj *New_Class6E99C(void *name, s32 arg1, s32 arg2);

/* Already matched in Entity.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- Entity__MoodCue57 (Entity_d) is its first cross-unit
 * caller, hence the extern here rather than only a file-local definition. */
extern Unk100Obj *Entity__GetOrCreateUnk100(Entity *this, void *name, void *arg2, void *arg3, s32 arg4);

/* Object pointed to by `Entity::target`. NOT another `Entity`, despite +0x14
 * also holding an `EntityPos *` (same convention as `Entity::unk14`):
 * `Class6B5CC__FaceTarget` (still INCLUDE_ASM, code_d294.s) dereferences this object
 * at +0xC, and Entity's OWN +0xC (`Entity::unk0C`) is a plain `s32` flag,
 * not a pointer -- that mismatch rules Entity itself out. Its vtable slot
 * +0x130 takes an extra `s32` argument at the one call site reached so far
 * (Entity__MoodCue01, `ori $a1, $zero, 0x1` before the `jalr`), unlike Entity's
 * OWN +0x130 slot (`EntityMethods::slot130`, self-only per Entity__StopSoundCue in
 * Entity.c) -- two different functions in two different tables that merely
 * share a numeric offset; do not conflate them. */
struct Unk94Methods {
    u8 pad000[0x44];
    void (*slot44)(Unk94Obj *self, s32 arg1, void *arg2); /* called by Entity__MoodCue49 (Entity_d) as slot44(target, 1, ROTATION_YAW_MINUS90); return value unused at this, its only known call site */
    u8 pad048[0x94 - 0x48];
    void (*slot94)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue123 (Entity_g), twice, as slot94(target, 0, 2) and slot94(target, 0, 7); return value unused at either call site */
    u8 pad098[0xB8 - 0x98];
    void (*slotB8)(Unk94Obj *self, void *arg1); /* called by Entity__MoodCue49 (Entity_d), arg1 is either NULL or &this->unk14->unk38 depending on this->unk0C; return value unused at this, its only known call site */
    u8 pad0BC[0xC4 - 0xBC];
    void (*slotC4)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue74 (Entity_e) as slotC4(target, 0x80, 0) and slotC4(target, -N, 1); return value unused at both known call sites, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    void (*slotC8)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue58 (Entity_d) as slotC8(target, (this->unkFC % 40 < 0x14) ? -5 : 5, 0); return value unused at this, its only known call site */
    void (*slotCC)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue47 (Entity_d) as slotCC(target, -0x64, 0) and Entity__MoodCue58 (Entity_d) as slotCC(target, -0x14, 0); return value unused at either call site, so void is a safe read regardless of the real return type (same caveat as the other such wrappers in this unit) */
    u8 pad0D0[0x100 - 0xD0];
    s32 (*slot100)(Unk94Obj *self);            /* called by Entity__MoodCue11, compared against 0 -- value-returning, not void */
    u8 pad104[0x120 - 0x104];
    s32 (*slot120)(Unk94Obj *self, s32 arg1, s32 arg2, void *arg3, s32 arg4); /* called by Entity__IsNearTarget as a TAIL CALL, `return this->target->methods->slot120(this->target, 0, arg2<<11, &localVec, distValue)` -- value-returning per CLAUDE.md's one-line-wrapper rule, no positive evidence of void */
    u8 pad124[0x130 - 0x124];
    void (*slot130)(Unk94Obj *self, s32 arg1); /* called by Entity__MoodCue01 and Entity__MoodCue58 (Entity_d), as slot130(target, 0), and by Entity__MoodCue115 (Entity_d) as slot130(target, 1) */
    void (*slot134)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue115 (Entity_d) and Entity__MoodCue74 (Entity_e), both as slot134(target, 1, 1); return value unused at either call site, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    u8 pad138[0x1A0 - 0x138];
    s32 (*slot1A0)(Unk94Obj *self, s32 arg1);  /* called by Entity__MoodCue46 (Entity_d), its return value taken mod 3 -- value-returning, not void */
    u8 pad1A4[0x200 - 0x1A4];
    s32 (*slot200)(Unk94Obj *self);            /* called by Entity__MoodCue00, compared against the literal 5, and by Entity__MoodCue58 (Entity_d), compared against 6 -- value-returning, not void */
    u8 pad204[0x21C - 0x204];
    void (*slot21C)(Unk94Obj *self);           /* called by Entity__MoodCue98 (Entity_g); return value unused at this, its only known call site */
};

struct Unk94Obj {
    Unk94Methods *methods; /* +0x00 */
    u8 pad04[0x14 - 0x04];
    EntityPos *unk14;        /* +0x14, read by Entity__IsTargetInRange (its own +0x1C, i.e. y) */
    u8 pad18[0x5C - 0x18];
    Unk5CObj *unk5C;          /* +0x5C, dereferenced through its OWN vtable (see Unk5CObj's own comment) by Entity__MoodCue74 (Entity_e) -- yet another instance of the class-framework object-pointer-at-a-field convention, same shape as Entity::unk4C/Entity::unk100 */
};

/* Object pointed to by `Unk94Obj::unk5C` -- named for the offset it sits at
 * in ITS parent, same convention as `Unk94Obj`/`Unk4CObj`/`Unk100Obj` being
 * named for the offset they sit at in THEIRS (`Entity`). Shape beyond the
 * one slot reached so far is unknown. */
struct Unk5CMethods {
    u8 pad000[0x64];
    void (*slot64)(Unk5CObj *self, void *arg1); /* called by Entity__MoodCue74 (Entity_e) as slot64(target->unk5C, D_8008AC1C); no other argument passed (only self+arg1 set up before the jalr) */
};

struct Unk5CObj {
    Unk5CMethods *methods; /* +0x00 */
};

/* Object pointed to by `Entity::unk4C` (previously modeled as a plain `s32`
 * on the strength of Entity__DetachUnk4C's `this->unk4C = 0;`, which type-checks
 * against a pointer just as well). Entity__MoodCue12 dereferences it at +0x00 as
 * a method-table pointer (the same class-framework idiom as `Unk94Obj`) and
 * calls its own +0x138 slot with two extra literal-1 arguments. Shape beyond
 * that single slot is unknown. */
struct Unk4CMethods {
    u8 pad000[0x138];
    void (*slot138)(Unk4CObj *self, s32 arg1, s32 arg2); /* called by Entity__MoodCue12; return value discarded at this one call site, so void is a safe read for THIS call's bytes regardless of the real return type (same caveat as ServiceSoundCueSet/FlushSoundCueSet elsewhere in this unit -- a discarded return is never positive evidence of void) */
};

struct Unk4CObj {
    Unk4CMethods *methods; /* +0x00 */
};

/* Object pointed to by `Entity::unk70`. Entity__MoodCue58 (Entity_d) dereferences
 * its own +0x04 to get a second, further object (`Unk70Sub`) and calls that
 * one's own vtable slot +0x60 -- the same two-level class-framework idiom as
 * `Unk94Obj`/`Unk100Obj`, just one hop deeper. Shape beyond the one slot
 * reached here is unknown. */
struct Unk70SubMethods {
    u8 pad00[0x60];
    void (*slot60)(Unk70Sub *self, s32 arg1); /* called by Entity__MoodCue58 (Entity_d) as slot60(this->unk70->unk4, 0); return value unused at this, its only known call site */
};

struct Unk70Sub {
    Unk70SubMethods *methods; /* +0x00 */
};

struct Unk70Obj {
    u8 pad00[0x04];
    Unk70Sub *unk4; /* +0x04, read by Entity__MoodCue58 (Entity_d) */
};

/* Default arguments Entity__GetOrCreateUnk100 substitutes when its own `name`/`arg2`
 * parameters are NULL -- both plain 2-word buffers (asm/data/7B3F8.sdata.s),
 * not strings; `gEntityDefaultPos` reads as {0x140, 0xF0} (320, 240, a plausible
 * screen-extent default) and `gEntityDefaultOffset` as {-100, -100}. */
extern s32 gEntityDefaultPos[2];
extern s32 gEntityDefaultOffset[2];

/* A 3-word (x, y, z) position, pointed to by `Entity::unk14` (and by
 * `Unk94Obj::unk14`, same convention). +0x1C (y) is now confirmed by a
 * direct reader: Entity__IsTargetInRange's own disassembly compares `this->unk14->y`
 * against `this->target->unk14->y` +/- 0x200 (that function itself stalled
 * on a register-identity residue, see its match report, but this field
 * derivation is unaffected). It is also inferred from Entity__IsNearTarget
 * consuming all three as one vector (asm/nonmatchings/Entity/Entity__IsNearTarget.s,
 * still `addiu_at`-blocked, copies its arg1[0]/[1]/[2] verbatim onto its own
 * stack). */
struct EntityPos {
    u8 pad00[0x18];
    s32 x; /* +0x18 */
    s32 y; /* +0x1C, read by Entity__IsTargetInRange */
    s32 z; /* +0x20 */
};

/* Entity__DistanceToRegion's second argument: a flag plus a pointer to an array of
 * 0x38-byte slots (element [1] is the only one read here). Not an Entity
 * type -- Entity__DistanceToRegion never dereferences `this->methods`, and this
 * pointer's own fields don't match anything else in this unit. */
struct EntityRegionSlot {
    s32 x0;             /* +0x00 */
    u8 pad04[0x04];
    s32 z0;              /* +0x08 */
    u8 pad0C[0x38 - 0x0C];
};

struct EntityRegionRef {
    u8 pad00[0x0C];
    s32 flag;                    /* +0x0C */
    u8 pad10[0x04];
    EntityRegionSlot *slots;       /* +0x14 */
};

/* Field offsets derived from this unit's own functions (Entity__TickSoundCue,
 * Entity__Activate, Entity__SetUnkF4, Entity__StopSoundCue, and the Get*Effect/Stage/Video
 * family). `soundCueSet` is only ever address-taken (passed as an output buffer to
 * two still-uncarved functions, ServiceSoundCueSet/FlushSoundCueSet), never read
 * here beyond its first word (zeroed by Entity__Entity), so its true
 * size/shape past that first s32 is still unconfirmed -- it's padded out
 * only as far as +0xF0, where the next known field starts. */
struct Entity {
    /* +0x00 */ EntityMethods *methods;
    /* +0x04 */ u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 unk0C;              /* gate flag checked by Entity__DetachUnk4C -- parallels Class65650/DreamSys's own shared-base +0xC gate, see code_55dd4.h */
    /* +0x10 */ u8 pad10[0x14 - 0x10];
    /* +0x14 */ EntityPos *unk14;        /* the 3-word position Entity__IsNearTarget/Entity__DistanceToRegion read via +0x18 */
    /* +0x18 */ u8 pad18[0x24 - 0x18];
    /* +0x24 */ s32 unk24;             /* cleared by Entity__Activate; xored against a mood-row-derived value in Entity__UpdateDeactivationState */
    /* +0x28 */ s32 unk28;             /* read by Entity__MoodCue11, gates its final slotCC call */
    /* +0x2C */ u8 pad2C[0x44 - 0x2C];
    /* +0x44 */ s32 moodState;             /* gates Entity__UpdateActivationState's whole body when == 1; also a small state code compared against several other literals (0xB, 0xC, 0x24, ...) by this unit's mood-dispatch handlers, and incremented directly by Entity__MoodCue13 */
    /* +0x48 */ s16 unk48;               /* a HALFWORD field (sh/lh, not the full-word sw/lw every other field here uses) -- Entity__MoodCue11 both writes it (-0x14, -0x78) and reads it back (as slotD0's arg1) */
    /* +0x4A */ u8 pad4A[0x4C - 0x4A];
    /* +0x4C */ Unk4CObj *unk4C;          /* cleared (NULL) by Entity__DetachUnk4C; dereferenced through its own vtable by Entity__MoodCue12 -- see Unk4CObj's own comment */
    /* +0x50 */ s32 unk50;              /* read by Entity__MoodCue57 (Entity_d), passed opaquely to this->unk100->methods->slotD4 as its arg1 */
    /* +0x54 */ u8 pad54[0x58 - 0x54];
    /* +0x58 */ s32 soundCueChannel;             /* passed to ServiceSoundCueSet/FlushSoundCueSet */
    /* +0x5C */ u8 pad5C[0x70 - 0x5C];
    /* +0x70 */ Unk70Obj *unk70;       /* read by Entity__MoodCue58 (Entity_d), see Unk70Obj's own comment */
    /* +0x74 */ u8 pad74[0x7C - 0x74];
    /* +0x7C */ s32 unk7C;             /* gate flag read by Entity__MoodCue77 (Entity_e); when 0, that function returns immediately after its unkFC==unk80/slot128/rand() dice-roll block */
    /* +0x80 */ s32 moodDuration;           /* read by Entity__MoodCue09/Entity__MoodCue11 (halved via the signed-divide-by-2 idiom, `(x + (unsigned)x>>31) >> 1`) and Entity__MoodCue13 (compared to `out->unk4` as `this->moodDuration - 1`) ; round 78 head: renamed from unk80 by type scope (echo, Entity__MoodCue21.md) */
    /* +0x84 */ s32 unk84;             /* compared against a literal (Entity__MoodCue14: `== 0xA`) or against `this->unk80 / 2` (Entity__MoodCue07); also a loop counter in func_80063ED4/func_80064078 (Entity_f), incremented past `< 0x18` while `slot134` is called each iteration */
    /* +0x88 */ s32 unk88;             /* func_80063ED4/func_80064078 (Entity_f): threaded through the slot134 loop as its own running arg1/return value */
    /* +0x8C */ u8 pad8C[0x90 - 0x8C];
    /* +0x90 */ s32 unk90;             /* Entity__MoodCue78 (Entity_e): nonzero gates `out->unk1C = 0x1C` when `out->unk4 & 3` is also 0 */
    /* +0x94 */ Unk94Obj *target;         /* passed as Class6B5CC__FaceTarget's (still INCLUDE_ASM, code_d294.s) second argument by Entity__UpdateTargetProximity/Entity__MoodCue01; see Unk94Obj's own comment for why it is NOT another Entity despite sharing the +0x14 EntityPos* convention */
    /* +0x98 */ s32 moodIndex;         /* selects a 16-byte row in the D_80089EAxx tables */
    /* +0x9C */ s32 soundCueSet;             /* zeroed by Entity__Entity; address-taken by Entity__TickSoundCue/Entity__StopSoundCue */
    /* +0xA0 */ u8 padA0[0xB0 - 0xA0];
    /* +0xB0 */ s32 proximityDivisor;             /* divisor in Entity__GetProximityRatio's (getProximityRatio) computation, this unit */
    /* +0xB4 */ u8 padB4[0xF0 - 0xB4];
    /* +0xF0 */ s32 active;             /* set to 1 by Entity__Activate; gate flag for Entity__UpdateActivationState/Entity__UpdateDeactivationState */
    /* +0xF4 */ s32 unkF4;             /* set from Entity__SetUnkF4's arg1 */
    /* +0xF8 */ s32 soundCueActive;             /* cleared by Entity__StopSoundCue */
    /* +0xFC */ s32 moodTimer;             /* incremented by Entity__TickSoundCue */
    /* +0x100 */ Unk100Obj *unk100;      /* lazily created/cached by Entity__GetOrCreateUnk100; torn down by Entity__Destructor */
    /* +0x104 */ Unk100Obj *unk104;       /* torn down by Entity__Destructor, never set within this unit */
};

extern EntityMethods *Get_vtable_Entity(void);
extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);

/* All three still uncarved (no asm/nonmatchings file -- library or
 * not-yet-carved game code); called directly by name (jal), not through a
 * vtable, so they need a real extern prototype per CLAUDE.md's "calling
 * into a function that is still INCLUDE_ASM" guidance. Entity__IsNearTarget itself
 * DOES have a carved (but addiu_at-blocked, still-INCLUDE_ASM) .s file in
 * this unit; its param shape is read directly off that disassembly: a0 is
 * `this` (dereferences ->0x98/->0x94, both known Entity fields), a1 points
 * at a 3-word vector copied onto its own stack. a2/a3 are `s32`, NOT `s8`:
 * every known caller (Entity__UpdateDeactivationState, Entity__UpdateTargetProximity, Entity__UpdateSoundCueStart) happens
 * to pass a byte-range value, but Entity__IsNearTarget's own body (dividing 0x800
 * by a3 and shifting the quotient by 11) treats them as full words with no
 * narrowing on entry, and declaring them `s8` forces a spurious sign-extend
 * at any call site whose argument is already a full-width computed `s32`
 * (found via Entity__UpdateTargetProximity's own residue -- see its match report).
 * ServiceSoundCueSet/FlushSoundCueSet's return values are unused at both call
 * sites, so void is a safe read regardless of the real return type. */
extern void ServiceSoundCueSet(s32 arg0, void *arg1);
extern void FlushSoundCueSet(s32 arg0, void *arg1);
extern s32 Entity__IsNearTarget(Entity *this, void *pos, s32 arg2, s32 arg3);
extern void Entity__NotifyIfTargetInRange(Entity *this, s32 arg1);
extern s32 rand(void);

/* The mood-indexed table lookups. `gEntityMoodTable` is a real struct array (16
 * bytes/entry, `this->moodIndex` selects the row) -- Entity__UpdateActivationState reads
 * its +0x3 (signed) and Entity__UpdateDeactivationState its +0x4 (UNSIGNED) as two DIFFERENT
 * small-enum fields, not the same byte reinterpreted; both also read +0x5
 * (signed) and +0x9 (signed). `gEntityUnlockKindTable`/`gEntityLinkStageTable`/`gEntityEventVideoTable` are
 * SEPARATE global arrays (own base symbols, own `lui`/`addiu`), each also
 * 16-byte/entry and independently `this->moodIndex`-indexed -- despite the
 * base addresses' proximity, they are not sub-fields of the gEntityMoodTable row.
 * Table element types past what's listed here are `s8` (signed byte loads),
 * not `char`, despite `-funsigned-char` making plain `char` unsigned project-
 * wide -- these tables are explicitly `lb`, not `lbu`, in every user seen so
 * far (contrast `linkKind`, `gEntityUnlockKindTable`, both `lbu`/`lb`-mixed by design,
 * not by the project's usual char convention). */
struct EntityMoodRow {
    u8 pad00[0x03];
    s8 detachKind;   /* +0x03, read by Entity__UpdateActivationState */
    u8 linkKind;      /* +0x04, read by Entity__UpdateDeactivationState (unsigned load) */
    s8 unk5;           /* +0x05 */
    s8 proximityRange;  /* +0x06, read by Entity__UpdateTargetProximity only (compiler-checked, round 71): magnitude (after abs) is Entity__IsNearTarget's distance arg for raising unkF4 via setUnkF4 (slot164); a NEGATIVE value also makes the entity face its target every tick */
    u8 pad07[0x02];
    s8 unk9;              /* +0x09, distance-fixup byte shared by Entity__UpdateTargetProximity/Entity__UpdateSoundCueStart/Entity__UpdateSoundCueStop */
    u8 pad0A[0x01];
    s8 cueRange;            /* +0x0B, read by Entity__UpdateSoundCueStart/Entity__UpdateSoundCueStop only (compiler-checked, round 71): 0 = the cue never auto-starts; magnitude (after abs) is Entity__IsNearTarget's distance arg for starting the sound cue; a NEGATIVE value also stops it again once the target leaves that range. SEPARATE field from proximityRange (+0x06) */
    u8 pad0C[0x04];
};

extern EntityMoodRow gEntityMoodTable[];
extern s8 gEntityUnlockKindTable[];  /* GetUnlockEffect */
extern s8 D_80089EA7[];  /* read by Entity__AttachUnk4C, own base symbol immediately after gEntityUnlockKindTable, moodIndex*0x10-indexed like the rest of this family */
extern s8 gEntityLinkStageTable[];  /* GetLinkStage */
extern s8 gEntityEventVideoTable[];  /* GetEventVideo */
extern s8 gEntityProximityThresholdTable[];  /* read by Entity__GetProximityRatio (getProximityRatio), own base symbol immediately before D_80089EAF, moodIndex*0x10-indexed like the rest of this family */
extern s8 D_80089EAF[];  /* read by Entity__AttachUnk4C, own base symbol immediately after gEntityEventVideoTable, moodIndex*0x10-indexed like the rest of this family */

void *Entity__GetMoodEffect(Entity *this);
s32 Entity__GetEventVideo(Entity *this);
s32 Entity__GetUnlockEffect(Entity *this);
s32 Entity__GetLinkStage(Entity *this);

/* The vtable data slot Get_vtable_Entity returns the address of. Still a raw
 * asm data blob (asm/data/79528.data.s, offsets 0x000..0x180) -- only an
 * extern of the right TYPE is needed here, the bytes stay splat-generated. */
extern EntityMethods ENTITY_METHODS;

/* Still uncarved (code_d294.s). Entity__UpdateTargetProximity calls it with this->target as
 * the second argument, a literal 1 as the third, and 0 for both the fourth
 * argument and a fifth argument passed on the stack; the callee itself
 * dereferences that second argument at +0xC/+0x14, confirming it is a
 * pointer, not a plain word. Return value unused at this call site, so void
 * is a safe read regardless of the real return type (same caveat as
 * ServiceSoundCueSet/FlushSoundCueSet above). */
extern void Class6B5CC__FaceTarget(Entity *this, void *arg1, s32 arg2, s32 arg3, s32 arg4);

/* Already matched in Entity_b.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- Entity__MoodCue115 (Entity_d) is its first cross-unit
 * caller. */
extern s32 Entity__IsTargetInRange(Entity *this, s32 arg1);

/* Second argument threaded through the moodIndex-selected event-dispatch
 * handlers (Entity__MoodCue15, Entity__MoodCue05, Entity__MoodCue10, and the sibling
 * handlers this unit hasn't reached yet -- all reachable as {handler,
 * data0, data1, data2} 16-byte rows of the gEntityMoodHandlerTable table in
 * asm/data/79528.data.s, immediately after gEntityEventVideoTable). Not an Entity --
 * these handlers only ever read a gate flag out of it and write result
 * codes back in. Real name/size unknown; only the offsets touched so far
 * are given. */
typedef struct EntityMoodHandlerArg EntityMoodHandlerArg;
struct EntityMoodHandlerArg {
    u8 pad00[0x04];
    s32 unk4;    /* +0x04, gate flag read by Entity__MoodCue05/Entity__MoodCue10/Entity__MoodCue09/Entity__MoodCue13, and by Entity__MoodCue07 (as `out->unk4 % 90`) */
    u8 pad08[0x08];
    s32 unk10;    /* +0x10, written by Entity__MoodCue15/Entity__MoodCue05/Entity__MoodCue10/... */
    u8 pad14[0x08];
    s32 unk1C;     /* +0x1C, written by Entity__MoodCue15/Entity__MoodCue05/Entity__MoodCue10/... */
    s32 unk20;      /* +0x20, written by Entity__MoodCue07 only (paired with unk1C the same round) */
    s32 unk24;       /* +0x24, written by Entity__MoodCue40 (Entity_d) */
    s32 unk28;        /* +0x28, written by Entity__MoodCue40 (Entity_d) */
    u8 pad2C[0x04];
    s32 unk30;      /* +0x30, written by Entity__MoodCue10/Entity__MoodCue07 */
    s32 unk34;       /* +0x34, written by Entity__MoodCue07 only (paired with unk30) */
    u8 pad38[0x0C];
    s32 unk44;       /* +0x44, written by Entity__MoodCue10/Entity__MoodCue07 */
    s32 unk48;        /* +0x48, written by Entity__MoodCue07 only (paired with unk44) */
};

/* Already matched in Entity_e.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- Entity__MoodCue108 (Entity_g) is its first cross-unit
 * caller, forwarding its own (this, out) straight through. */
extern void Entity__MoodCue71(Entity *this, EntityMoodHandlerArg *out);

/* Already matched in Entity_d.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- Entity__MoodCue113 (Entity_g) is its first cross-unit
 * caller, forwarding its own (this, out) straight through. */
extern void Entity__MoodCue51(Entity *this, EntityMoodHandlerArg *out);

#endif
