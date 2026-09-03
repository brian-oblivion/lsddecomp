#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"

/* The Entity class. `src/Entity.c` is the first 25 of a 142-function block
 * split at func_8005DE18; the remainder is Entity_b, still a monolithic asm
 * segment. Its own vtable is fetched via `Get_vtable_Entity` (still asm,
 * address 0x8005E150, per config/symbols.slps01556.lsdde.txt -- New_Entity's
 * disassembly calls it directly by name, `jal Get_vtable_Entity`, the same
 * `Get_vtable_X`-named-accessor shape as `Get_vtable_DreamSys` in
 * docs/research/class-framework.md). Only the vtable slots this unit's
 * queued functions actually dispatch through are typed below; the struct is
 * NOT padded out to its full size (New_Entity allocates 0x108 bytes, but
 * nothing here needs the whole layout).
 *
 * The vtable itself is `D_80089AD4` (asm/data/79528.data.s, offsets 0x000..
 * 0x180) and it doubles as a legible cross-check: whichever function each
 * slot currently HOLDS is the function that OVERRIDES/OCCUPIES that slot,
 * not the function that CALLS it. Do not confuse the two -- the "called by"
 * comments on `slot30`/`slot60`/`slot114`/`slot130` below name a CALLER
 * of that slot (found by reading that caller's own disassembly), while
 * "this unit's own slot" comments elsewhere name the function occupying a
 * slot in the table. Early slots (+0x004, +0x010, +0x014, +0x01C..+0x038)
 * hold `BasicClass__func_*`/`func_800570B4`/`func_80057130` -- the SAME
 * addresses `code_55dd4.h`'s `Class65650Methods` holds at its own +0x004/
 * +0x010/+0x014, confirming Entity and Class65650 share the identical
 * "BasicClass" ancestor and its vtable layout convention (ctor at +0x008,
 * dtor at +0x00C universally). `func_80066818()` (matched in code_55dd4.c)
 * returns that SHARED ancestor's own vtable (`&D_8008A6C4`) directly -- so
 * a `func_80066818()->slotNN(...)` call from Entity's own functions is a
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
    /* +0x30 */ void (*slot30)(Entity *self, s32 arg1);   /* called by func_8005DAAC, func_8005DF9C */
    /* +0x34 */ u8 pad34[0x40 - 0x34];
    /* +0x40 */ void (*slot40)(Entity *self);              /* called by Entity__Entity, right after this->methods is (re)assigned */
    /* +0x44 */ void (*slot44)(Entity *self, s32 arg1, void *arg2); /* called by func_8005E4D0 as slot44(this, 0, D_80089CA0) */
    /* +0x48 */ s32 (*slot48)(Entity *self, s32 arg1, void *arg2); /* called by func_8005E694 (result discarded) and func_8005EF20 (a tail call that returns it), both with arg1==1 -- see CLAUDE.md's "one-line wrapper" rule, func_8005EF20 has no positive evidence of void */
    /* +0x4C */ u8 pad4C[0x60 - 0x4C];
    /* +0x60 */ void (*slot60)(Entity *self, s32 arg1);   /* called by func_8005D9F4, func_8005DA3C */
    /* +0x64 */ u8 pad64[0xB8 - 0x64];
    /* +0xB8 */ void (*slotB8)(Entity *self, void *arg1);  /* called by func_8005F800 as slotB8(this, &this->unk94->unk14->x) -- a vector-pointer argument, same shape as func_8005D714's still-INCLUDE_ASM arg1 */
    /* +0xBC */ void (*slotBC)(Entity *self, void *arg1);  /* called by func_8005E694 */
    /* +0xC0 */ u8 padC0[0xC4 - 0xC0];
    /* +0xC4 */ void (*slotC4)(Entity *self, s32 arg1, s32 arg2); /* called by func_8005E7A8/func_8005EBB4/func_8005EC98/func_8005E160/etc (all discard the result) and func_8005FA64 (its tail call) -- same shared BasicClass-inherited slot as Class65650Methods.slotC4 in code_55dd4.h (both tables hold func_8005748C at +0xC4, confirmed with tools/classtable.py). CLAUDE.md's "one-line wrapper" rule would normally push this toward s32-returning on func_8005FA64's strength alone -- but retyping it to s32 changes func_8005E160's OWN codegen (verified: GCC stops tail-merging its two identical slotC4(this,0x32,0)-reached-from-different-branches call sites, costing that ALREADY-MATCHED function 4 words and shifting every later function in the unit). func_8005E160's own bytes are the stronger, more direct evidence and they require void. Kept void; func_8005FA64 is void too, with a bare statement call, not `return`. (slotCC below faced the same question and was independently verified NOT to have this problem.) */
    /* +0xC8 */ void (*slotC8)(Entity *self, s32 arg1, s32 arg2); /* called by func_8005FC58 (discards the result); same (self,arg1,arg2) shape as slotC4/slotCC, no other caller yet so kept void by default like slotC4 */
    /* +0xCC */ s32 (*slotCC)(Entity *self, s32 arg1, s32 arg2); /* called by func_8005E7F8 (discards the result) and func_8005FEC8 (a tail call that returns it) -- see CLAUDE.md's "one-line wrapper" rule, func_8005FEC8 has no positive evidence of void. Verified this retype does NOT perturb func_8005E7F8's own codegen (unlike slotC4 above, which does) */
    /* +0xD0 */ void (*slotD0)(Entity *self, s32 arg1, s32 arg2); /* called by func_8005ED30 as slotD0(this, -0x176, rand() % 2), and by func_8005E7F8 as slotD0(this, this->unk48, 0) */
    /* +0xD4 */ u8 padD4[0x114 - 0xD4];
    /* +0x114 */ void (*slot114)(Entity *self);           /* called by func_8005DB8C */
    /* +0x118 */ u8 pad118[0x128 - 0x118];
    /* +0x128 */ void (*slot128)(Entity *self, s32 arg1); /* called by func_80062A40 (Entity_e) as slot128(this, 1); return value unused at this, its only known call site, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    /* +0x12C */ void (*slot12C)(Entity *self);           /* called by func_800603C4 */
    /* +0x130 */ void (*slot130)(Entity *self);           /* called by func_8005DB8C, func_800603C4 */
    /* +0x134 */ u8 pad134[0x144 - 0x134];
    /* +0x144 */ s32 (*slot144)(Entity *self, Unk94Obj *arg1); /* called by func_8005E02C, as slot144(this, this->unk94) -- arg1 stays live in $a1 from its own first use all the way to this call, which is WHY retail keeps this->unk94 in $a1 rather than a scratch register (see the match report's now-superseded "register identity" stall write-up); compared with slt -- value-returning, not void */
    /* +0x148 */ s32 (*slot148)(Entity *self);            /* called by func_8005E480; holds func_8005D864 (still addiu_at-blocked in Entity.c) */
    /* +0x14C */ u8 pad14C[0x15C - 0x14C];
    /* +0x15C */ void (*slot15C)(Entity *self);            /* called by func_8005DBF0 */
    /* +0x160 */ void (*slot160)(Entity *self);             /* called by func_8005D418, func_8005D658, func_8005DD18 */
    /* +0x164 */ void (*slot164)(Entity *self, s32 arg1);    /* called by func_8005DA3C and func_8005DE18 (as slot164(self, 1)) */
    /* +0x168 */ void (*slot168)(Entity *self);               /* called by func_8005DEE0 */
    /* +0x16C */ void (*slot16C)(Entity *self);                /* called by func_8005DA3C, func_8005E0B0, func_80062A40 (Entity_e) */
    /* +0x170 */ s32 (*slot170)(Entity *self);                  /* called by func_8005D480 */
    /* +0x174 */ void (*slot174)(Entity *self);                  /* called by func_8005D480 */
    /* +0x178 */ s32 (*slot178)(Entity *self);                    /* called by func_8005D480; holds func_8005DE18, which ends `return this->unkF4;` -- NOT void despite the one known caller discarding it, see CLAUDE.md's "discarded return is never evidence of void" */
    /* +0x17C */ s32 (*slot17C)(Entity *self);                     /* called by func_8005D480; holds func_8005DEE0, which ends `return this->unkF8;` */
    /* +0x180 */ void (*slot180)(Entity *self);                     /* called by func_8005D480 */
};

/* Entity's own local view of the shared "BasicClass" ancestor vtable
 * returned by `func_80066818()` (matched in code_55dd4.c/code_55dd4.h,
 * which owns the canonical `Class65650Methods` view of this SAME table --
 * see the big comment above). Only the offsets this unit's functions reach
 * through it are named; everything else is inherited/not-yet-needed
 * padding, same convention as `Class65650Methods`. */
struct BasicClassMethods {
    /* +0x000 */ u8 pad00[0x08];
    /* +0x008 */ void *(*ctor)(void *self, s32 arg1, s32 arg2); /* Entity__Entity's base-class construction call */
    /* +0x00C */ void (*dtor)(void *self);                       /* called by func_8005D1EC */
    /* +0x010 */ u8 pad10[0x50 - 0x10];
    /* +0x050 */ void (*slot50)(void *self);                       /* called by func_8005D418 */
    /* +0x054 */ u8 pad54[0x98 - 0x54];
    /* +0x098 */ void (*slot98)(void *self, s32 arg1, s32 arg2);      /* called by func_8005D480 */
    /* +0x09C */ u8 pad9C[0xE0 - 0x9C];
    /* +0x0E0 */ void (*slotE0)(void *self, s32 arg1, s32 arg2);        /* called by func_8005D658 */
};

extern BasicClassMethods *func_80066818(void);

/* An object cached in `Entity::unk100`/`unk104`, unrelated to `EntityMethods`
 * -- its own method table, dispatched through in func_8005D1EC/func_8005D108.
 * Real shape unknown beyond the slots reached here. */
struct Unk100Methods {
    /* +0x00 */ u8 pad00[0x04];
    /* +0x04 */ void (*slot04)(Unk100Obj *self);                          /* called by func_8005D1EC */
    /* +0x08 */ u8 pad08[0x4C - 0x08];
    /* +0x4C */ void (*slot4C)(Unk100Obj *self, Entity *arg1, void *arg2); /* called by func_8005D108 */
    /* +0x50 */ void (*slot50)(Unk100Obj *self);                           /* called by func_8005D108 */
    /* +0x54 */ u8 pad54[0xD0 - 0x54];
    /* +0xD0 */ void (*slotD0)(Unk100Obj *self, void *arg1);                /* called by func_8005D108 */
    /* +0xD4 */ void (*slotD4)(Unk100Obj *self, s32 arg1, s32 arg2, s32 arg3); /* called by func_80061198 (Entity_d) as slotD4(this->unk100, this->unk50, 4, 0), and by func_80063874 (Entity_f) as slotD4(this->unk100, this->unk50, 7, 0) */
    /* +0xD8 */ void (*slotD8)(Unk100Obj *self, s32 arg1, s32 arg2, s32 arg3); /* called by func_80063874 (Entity_f) as slotD8(this->unk100, this->unk50, 0, 0); return value unused at this, its only known call site */
};

struct Unk100Obj {
    Unk100Methods *methods; /* +0x00 */
};

extern Unk100Obj *func_8003FDB0(void *name, s32 arg1, s32 arg2);

/* Already matched in Entity.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- func_80061198 (Entity_d) is its first cross-unit
 * caller, hence the extern here rather than only a file-local definition. */
extern Unk100Obj *func_8005D108(Entity *this, void *name, void *arg2, void *arg3, s32 arg4);

/* Object pointed to by `Entity::unk94`. NOT another `Entity`, despite +0x14
 * also holding an `EntityPos *` (same convention as `Entity::unk14`):
 * `func_8001EACC` (still INCLUDE_ASM, code_d294.s) dereferences this object
 * at +0xC, and Entity's OWN +0xC (`Entity::unk0C`) is a plain `s32` flag,
 * not a pointer -- that mismatch rules Entity itself out. Its vtable slot
 * +0x130 takes an extra `s32` argument at the one call site reached so far
 * (func_8005E3C4, `ori $a1, $zero, 0x1` before the `jalr`), unlike Entity's
 * OWN +0x130 slot (`EntityMethods::slot130`, self-only per func_8005DB8C in
 * Entity.c) -- two different functions in two different tables that merely
 * share a numeric offset; do not conflate them. */
struct Unk94Methods {
    u8 pad000[0x44];
    void (*slot44)(Unk94Obj *self, s32 arg1, void *arg2); /* called by func_80060B34 (Entity_d) as slot44(unk94, 1, D_80089C94); return value unused at this, its only known call site */
    u8 pad048[0xB8 - 0x48];
    void (*slotB8)(Unk94Obj *self, void *arg1); /* called by func_80060B34 (Entity_d), arg1 is either NULL or &this->unk14->unk38 depending on this->unk0C; return value unused at this, its only known call site */
    u8 pad0BC[0xC4 - 0xBC];
    void (*slotC4)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by func_80062730 (Entity_e) as slotC4(unk94, 0x80, 0) and slotC4(unk94, -N, 1); return value unused at both known call sites, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    void (*slotC8)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by func_80061400 (Entity_d) as slotC8(unk94, (this->unkFC % 40 < 0x14) ? -5 : 5, 0); return value unused at this, its only known call site */
    void (*slotCC)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by func_8006090C (Entity_d) as slotCC(unk94, -0x64, 0) and func_80061400 (Entity_d) as slotCC(unk94, -0x14, 0); return value unused at either call site, so void is a safe read regardless of the real return type (same caveat as the other such wrappers in this unit) */
    u8 pad0D0[0x100 - 0xD0];
    s32 (*slot100)(Unk94Obj *self);            /* called by func_8005E7F8, compared against 0 -- value-returning, not void */
    u8 pad104[0x130 - 0x104];
    void (*slot130)(Unk94Obj *self, s32 arg1); /* called by func_8005E3C4 and func_80061400 (Entity_d), as slot130(unk94, 0), and by func_80061778 (Entity_d) as slot130(unk94, 1) */
    void (*slot134)(Unk94Obj *self, s32 arg1, s32 arg2); /* called by func_80061778 (Entity_d) and func_80062730 (Entity_e), both as slot134(unk94, 1, 1); return value unused at either call site, so void is a safe read regardless of the real return type (same caveat as this table's other such wrappers) */
    u8 pad138[0x1A0 - 0x138];
    s32 (*slot1A0)(Unk94Obj *self, s32 arg1);  /* called by func_80060800 (Entity_d), its return value taken mod 3 -- value-returning, not void */
    u8 pad1A4[0x200 - 0x1A4];
    s32 (*slot200)(Unk94Obj *self);            /* called by func_8005E160, compared against the literal 5, and by func_80061400 (Entity_d), compared against 6 -- value-returning, not void */
};

struct Unk94Obj {
    Unk94Methods *methods; /* +0x00 */
    u8 pad04[0x14 - 0x04];
    EntityPos *unk14;        /* +0x14, read by func_8005E02C (its own +0x1C, i.e. y) */
    u8 pad18[0x5C - 0x18];
    Unk5CObj *unk5C;          /* +0x5C, dereferenced through its OWN vtable (see Unk5CObj's own comment) by func_80062730 (Entity_e) -- yet another instance of the class-framework object-pointer-at-a-field convention, same shape as Entity::unk4C/Entity::unk100 */
};

/* Object pointed to by `Unk94Obj::unk5C` -- named for the offset it sits at
 * in ITS parent, same convention as `Unk94Obj`/`Unk4CObj`/`Unk100Obj` being
 * named for the offset they sit at in THEIRS (`Entity`). Shape beyond the
 * one slot reached so far is unknown. */
struct Unk5CMethods {
    u8 pad000[0x64];
    void (*slot64)(Unk5CObj *self, void *arg1); /* called by func_80062730 (Entity_e) as slot64(unk94->unk5C, D_8008AC1C); no other argument passed (only self+arg1 set up before the jalr) */
};

struct Unk5CObj {
    Unk5CMethods *methods; /* +0x00 */
};

/* Object pointed to by `Entity::unk4C` (previously modeled as a plain `s32`
 * on the strength of func_8005D418's `this->unk4C = 0;`, which type-checks
 * against a pointer just as well). func_8005EA94 dereferences it at +0x00 as
 * a method-table pointer (the same class-framework idiom as `Unk94Obj`) and
 * calls its own +0x138 slot with two extra literal-1 arguments. Shape beyond
 * that single slot is unknown. */
struct Unk4CMethods {
    u8 pad000[0x138];
    void (*slot138)(Unk4CObj *self, s32 arg1, s32 arg2); /* called by func_8005EA94; return value discarded at this one call site, so void is a safe read for THIS call's bytes regardless of the real return type (same caveat as func_8002CD08/func_8002CC84 elsewhere in this unit -- a discarded return is never positive evidence of void) */
};

struct Unk4CObj {
    Unk4CMethods *methods; /* +0x00 */
};

/* Object pointed to by `Entity::unk70`. func_80061400 (Entity_d) dereferences
 * its own +0x04 to get a second, further object (`Unk70Sub`) and calls that
 * one's own vtable slot +0x60 -- the same two-level class-framework idiom as
 * `Unk94Obj`/`Unk100Obj`, just one hop deeper. Shape beyond the one slot
 * reached here is unknown. */
struct Unk70SubMethods {
    u8 pad00[0x60];
    void (*slot60)(Unk70Sub *self, s32 arg1); /* called by func_80061400 (Entity_d) as slot60(this->unk70->unk4, 0); return value unused at this, its only known call site */
};

struct Unk70Sub {
    Unk70SubMethods *methods; /* +0x00 */
};

struct Unk70Obj {
    u8 pad00[0x04];
    Unk70Sub *unk4; /* +0x04, read by func_80061400 (Entity_d) */
};

/* Default arguments func_8005D108 substitutes when its own `name`/`arg2`
 * parameters are NULL -- both plain 2-word buffers (asm/data/7B3F8.sdata.s),
 * not strings; `D_8008AC14` reads as {0x140, 0xF0} (320, 240, a plausible
 * screen-extent default) and `D_8008AC0C` as {-100, -100}. */
extern s32 D_8008AC14[2];
extern s32 D_8008AC0C[2];

/* A 3-word (x, y, z) position, pointed to by `Entity::unk14` (and by
 * `Unk94Obj::unk14`, same convention). +0x1C (y) is now confirmed by a
 * direct reader: func_8005E02C's own disassembly compares `this->unk14->y`
 * against `this->unk94->unk14->y` +/- 0x200 (that function itself stalled
 * on a register-identity residue, see its match report, but this field
 * derivation is unaffected). It is also inferred from func_8005D714
 * consuming all three as one vector (asm/nonmatchings/Entity/func_8005D714.s,
 * still `addiu_at`-blocked, copies its arg1[0]/[1]/[2] verbatim onto its own
 * stack). */
struct EntityPos {
    u8 pad00[0x18];
    s32 x; /* +0x18 */
    s32 y; /* +0x1C, read by func_8005E02C */
    s32 z; /* +0x20 */
};

/* func_8005D7FC's second argument: a flag plus a pointer to an array of
 * 0x38-byte slots (element [1] is the only one read here). Not an Entity
 * type -- func_8005D7FC never dereferences `this->methods`, and this
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

/* Field offsets derived from this unit's own functions (func_8005D6D4,
 * func_8005D9F4, func_8005DAAC, func_8005DB8C, and the Get*Effect/Stage/Video
 * family). `unk9C` is only ever address-taken (passed as an output buffer to
 * two still-uncarved functions, func_8002CD08/func_8002CC84), never read
 * here beyond its first word (zeroed by Entity__Entity), so its true
 * size/shape past that first s32 is still unconfirmed -- it's padded out
 * only as far as +0xF0, where the next known field starts. */
struct Entity {
    /* +0x00 */ EntityMethods *methods;
    /* +0x04 */ u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 unk0C;              /* gate flag checked by func_8005D418 -- parallels Class65650/DreamSys's own shared-base +0xC gate, see code_55dd4.h */
    /* +0x10 */ u8 pad10[0x14 - 0x10];
    /* +0x14 */ EntityPos *unk14;        /* the 3-word position func_8005D714/func_8005D7FC read via +0x18 */
    /* +0x18 */ u8 pad18[0x24 - 0x18];
    /* +0x24 */ s32 unk24;             /* cleared by func_8005D9F4; xored against a mood-row-derived value in func_8005DD18 */
    /* +0x28 */ s32 unk28;             /* read by func_8005E7F8, gates its final slotCC call */
    /* +0x2C */ u8 pad2C[0x44 - 0x2C];
    /* +0x44 */ s32 unk44;              /* gates func_8005DBF0's whole body when == 1; also a small state code compared against several other literals (0xB, 0xC, 0x24, ...) by this unit's mood-dispatch handlers, and incremented directly by func_8005EBB4 */
    /* +0x48 */ s16 unk48;               /* a HALFWORD field (sh/lh, not the full-word sw/lw every other field here uses) -- func_8005E7F8 both writes it (-0x14, -0x78) and reads it back (as slotD0's arg1) */
    /* +0x4A */ u8 pad4A[0x4C - 0x4A];
    /* +0x4C */ Unk4CObj *unk4C;          /* cleared (NULL) by func_8005D418; dereferenced through its own vtable by func_8005EA94 -- see Unk4CObj's own comment */
    /* +0x50 */ s32 unk50;              /* read by func_80061198 (Entity_d), passed opaquely to this->unk100->methods->slotD4 as its arg1 */
    /* +0x54 */ u8 pad54[0x58 - 0x54];
    /* +0x58 */ s32 unk58;             /* passed to func_8002CD08/func_8002CC84 */
    /* +0x5C */ u8 pad5C[0x70 - 0x5C];
    /* +0x70 */ Unk70Obj *unk70;       /* read by func_80061400 (Entity_d), see Unk70Obj's own comment */
    /* +0x74 */ u8 pad74[0x7C - 0x74];
    /* +0x7C */ s32 unk7C;             /* gate flag read by func_80062A40 (Entity_e); when 0, that function returns immediately after its unkFC==unk80/slot128/rand() dice-roll block */
    /* +0x80 */ s32 unk80;             /* read by func_8005E6F0/func_8005E7F8 (halved via the signed-divide-by-2 idiom, `(x + (unsigned)x>>31) >> 1`) and func_8005EBB4 (compared to `out->unk4` as `this->unk80 - 1`) */
    /* +0x84 */ s32 unk84;             /* compared against a literal (func_8005EC98: `== 0xA`) or against `this->unk80 / 2` (func_8005E4D0) */
    /* +0x88 */ u8 pad88[0x94 - 0x88];
    /* +0x94 */ Unk94Obj *unk94;         /* passed as func_8001EACC's (still INCLUDE_ASM, code_d294.s) second argument by func_8005DE18/func_8005E3C4; see Unk94Obj's own comment for why it is NOT another Entity despite sharing the +0x14 EntityPos* convention */
    /* +0x98 */ s32 moodIndex;         /* selects a 16-byte row in the D_80089EAxx tables */
    /* +0x9C */ s32 unk9C;             /* zeroed by Entity__Entity; address-taken by func_8005D6D4/func_8005DB8C */
    /* +0xA0 */ u8 padA0[0xF0 - 0xA0];
    /* +0xF0 */ s32 unkF0;             /* set to 1 by func_8005D9F4; gate flag for func_8005DBF0/func_8005DD18 */
    /* +0xF4 */ s32 unkF4;             /* set from func_8005DAAC's arg1 */
    /* +0xF8 */ s32 unkF8;             /* cleared by func_8005DB8C */
    /* +0xFC */ s32 unkFC;             /* incremented by func_8005D6D4 */
    /* +0x100 */ Unk100Obj *unk100;      /* lazily created/cached by func_8005D108; torn down by func_8005D1EC */
    /* +0x104 */ Unk100Obj *unk104;       /* torn down by func_8005D1EC, never set within this unit */
};

extern EntityMethods *Get_vtable_Entity(void);
extern void *func_80017B34(s32 size);
extern void func_80017CFC(void *arg);

/* All three still uncarved (no asm/nonmatchings file -- library or
 * not-yet-carved game code); called directly by name (jal), not through a
 * vtable, so they need a real extern prototype per CLAUDE.md's "calling
 * into a function that is still INCLUDE_ASM" guidance. func_8005D714 itself
 * DOES have a carved (but addiu_at-blocked, still-INCLUDE_ASM) .s file in
 * this unit; its param shape is read directly off that disassembly: a0 is
 * `this` (dereferences ->0x98/->0x94, both known Entity fields), a1 points
 * at a 3-word vector copied onto its own stack. a2/a3 are `s32`, NOT `s8`:
 * every known caller (func_8005DD18, func_8005DE18, func_8005DEE0) happens
 * to pass a byte-range value, but func_8005D714's own body (dividing 0x800
 * by a3 and shifting the quotient by 11) treats them as full words with no
 * narrowing on entry, and declaring them `s8` forces a spurious sign-extend
 * at any call site whose argument is already a full-width computed `s32`
 * (found via func_8005DE18's own residue -- see its match report).
 * func_8002CD08/func_8002CC84's return values are unused at both call
 * sites, so void is a safe read regardless of the real return type. */
extern void func_8002CD08(s32 arg0, void *arg1);
extern void func_8002CC84(s32 arg0, void *arg1);
extern s32 func_8005D714(Entity *this, void *pos, s32 arg2, s32 arg3);
extern void func_8005DF9C(Entity *this, s32 arg1);
extern s32 rand(void);

/* The mood-indexed table lookups. `D_80089EA4` is a real struct array (16
 * bytes/entry, `this->moodIndex` selects the row) -- func_8005DBF0 reads
 * its +0x3 (signed) and func_8005DD18 its +0x4 (UNSIGNED) as two DIFFERENT
 * small-enum fields, not the same byte reinterpreted; both also read +0x5
 * (signed) and +0x9 (signed). `D_80089EA6`/`D_80089EAB`/`D_80089EAC` are
 * SEPARATE global arrays (own base symbols, own `lui`/`addiu`), each also
 * 16-byte/entry and independently `this->moodIndex`-indexed -- despite the
 * base addresses' proximity, they are not sub-fields of the D_80089EA4 row.
 * Table element types past what's listed here are `s8` (signed byte loads),
 * not `char`, despite `-funsigned-char` making plain `char` unsigned project-
 * wide -- these tables are explicitly `lb`, not `lbu`, in every user seen so
 * far (contrast `linkKind`, `D_80089EA6`, both `lbu`/`lb`-mixed by design,
 * not by the project's usual char convention). */
struct EntityMoodRow {
    u8 pad00[0x03];
    s8 detachKind;   /* +0x03, read by func_8005DBF0 */
    u8 linkKind;      /* +0x04, read by func_8005DD18 (unsigned load) */
    s8 unk5;           /* +0x05 */
    s8 unk6;            /* +0x06, read by func_8005DE18: sign selects whether func_8001EACC also fires, magnitude (after abs) is func_8005D714's distance arg */
    u8 pad07[0x02];
    s8 unk9;              /* +0x09, distance-fixup byte shared by func_8005DE18/func_8005DEE0/func_8005E0B0 */
    u8 pad0A[0x01];
    s8 unkB;                /* +0x0B, read by func_8005DEE0/func_8005E0B0 -- SEPARATE field from unk6, not the same byte reread (different functions, different offsets) */
    u8 pad0C[0x04];
};

extern EntityMoodRow D_80089EA4[];
extern s8 D_80089EA6[];  /* GetUnlockEffect */
extern s8 D_80089EAB[];  /* GetLinkStage */
extern s8 D_80089EAC[];  /* GetEventVideo */

void *Entity__GetMoodEffect(Entity *this);
s32 Entity__GetEventVideo(Entity *this);
s32 Entity__GetUnlockEffect(Entity *this);
s32 Entity__GetLinkStage(Entity *this);

/* The vtable data slot Get_vtable_Entity returns the address of. Still a raw
 * asm data blob (asm/data/79528.data.s, offsets 0x000..0x180) -- only an
 * extern of the right TYPE is needed here, the bytes stay splat-generated. */
extern EntityMethods D_80089AD4;

/* Still uncarved (code_d294.s). func_8005DE18 calls it with this->unk94 as
 * the second argument, a literal 1 as the third, and 0 for both the fourth
 * argument and a fifth argument passed on the stack; the callee itself
 * dereferences that second argument at +0xC/+0x14, confirming it is a
 * pointer, not a plain word. Return value unused at this call site, so void
 * is a safe read regardless of the real return type (same caveat as
 * func_8002CD08/func_8002CC84 above). */
extern void func_8001EACC(Entity *this, void *arg1, s32 arg2, s32 arg3, s32 arg4);

/* Already matched in Entity_b.c (not INCLUDE_ASM), but not previously called
 * from outside that unit -- func_80061778 (Entity_d) is its first cross-unit
 * caller. */
extern s32 func_8005E02C(Entity *this, s32 arg1);

/* Second argument threaded through the moodIndex-selected event-dispatch
 * handlers (func_8005ED10, func_8005E480, func_8005E7A8, and the sibling
 * handlers this unit hasn't reached yet -- all reachable as {handler,
 * data0, data1, data2} 16-byte rows of the D_80089EB0 table in
 * asm/data/79528.data.s, immediately after D_80089EAC). Not an Entity --
 * these handlers only ever read a gate flag out of it and write result
 * codes back in. Real name/size unknown; only the offsets touched so far
 * are given. */
typedef struct EntityMoodHandlerArg EntityMoodHandlerArg;
struct EntityMoodHandlerArg {
    u8 pad00[0x04];
    s32 unk4;    /* +0x04, gate flag read by func_8005E480/func_8005E7A8/func_8005E6F0/func_8005EBB4, and by func_8005E4D0 (as `out->unk4 % 90`) */
    u8 pad08[0x08];
    s32 unk10;    /* +0x10, written by func_8005ED10/func_8005E480/func_8005E7A8/... */
    u8 pad14[0x08];
    s32 unk1C;     /* +0x1C, written by func_8005ED10/func_8005E480/func_8005E7A8/... */
    s32 unk20;      /* +0x20, written by func_8005E4D0 only (paired with unk1C the same round) */
    s32 unk24;       /* +0x24, written by func_80060148 (Entity_d) */
    s32 unk28;        /* +0x28, written by func_80060148 (Entity_d) */
    u8 pad2C[0x04];
    s32 unk30;      /* +0x30, written by func_8005E7A8/func_8005E4D0 */
    s32 unk34;       /* +0x34, written by func_8005E4D0 only (paired with unk30) */
    u8 pad38[0x0C];
    s32 unk44;       /* +0x44, written by func_8005E7A8/func_8005E4D0 */
    s32 unk48;        /* +0x48, written by func_8005E4D0 only (paired with unk44) */
};

#endif
