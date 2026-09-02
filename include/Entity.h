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
    /* +0x44 */ u8 pad44[0x60 - 0x44];
    /* +0x60 */ void (*slot60)(Entity *self, s32 arg1);   /* called by func_8005D9F4, func_8005DA3C */
    /* +0x64 */ u8 pad64[0x114 - 0x64];
    /* +0x114 */ void (*slot114)(Entity *self);           /* called by func_8005DB8C */
    /* +0x118 */ u8 pad118[0x130 - 0x118];
    /* +0x130 */ void (*slot130)(Entity *self);           /* called by func_8005DB8C */
    /* +0x134 */ u8 pad134[0x148 - 0x134];
    /* +0x148 */ s32 (*slot148)(Entity *self);            /* called by func_8005E480; holds func_8005D864 (still addiu_at-blocked in Entity.c) */
    /* +0x14C */ u8 pad14C[0x15C - 0x14C];
    /* +0x15C */ void (*slot15C)(Entity *self);            /* called by func_8005DBF0 */
    /* +0x160 */ void (*slot160)(Entity *self);             /* called by func_8005D418, func_8005D658, func_8005DD18 */
    /* +0x164 */ void (*slot164)(Entity *self, s32 arg1);    /* called by func_8005DA3C and func_8005DE18 (as slot164(self, 1)) */
    /* +0x168 */ void (*slot168)(Entity *self);               /* called by func_8005DEE0 */
    /* +0x16C */ void (*slot16C)(Entity *self);                /* called by func_8005DA3C */
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
};

struct Unk100Obj {
    Unk100Methods *methods; /* +0x00 */
};

extern Unk100Obj *func_8003FDB0(void *name, s32 arg1, s32 arg2);

/* Default arguments func_8005D108 substitutes when its own `name`/`arg2`
 * parameters are NULL -- both plain 2-word buffers (asm/data/7B3F8.sdata.s),
 * not strings; `D_8008AC14` reads as {0x140, 0xF0} (320, 240, a plausible
 * screen-extent default) and `D_8008AC0C` as {-100, -100}. */
extern s32 D_8008AC14[2];
extern s32 D_8008AC0C[2];

/* A 3-word (x, y, z) position, pointed to by `Entity::unk14`. Only the
 * words at +0x18/+0x20 (x/z) are read by this unit's functions; +0x1C (y)
 * is inferred from func_8005D714 consuming all three as one vector
 * (asm/nonmatchings/Entity/func_8005D714.s, still `addiu_at`-blocked, copies
 * its arg1[0]/[1]/[2] verbatim onto its own stack). */
struct EntityPos {
    u8 pad00[0x18];
    s32 x; /* +0x18 */
    s32 y; /* +0x1C, unconfirmed -- no reader in this unit */
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
    /* +0x28 */ u8 pad28[0x44 - 0x28];
    /* +0x44 */ s32 unk44;              /* gates func_8005DBF0's whole body when == 1 */
    /* +0x48 */ u8 pad48[0x4C - 0x48];
    /* +0x4C */ s32 unk4C;               /* cleared by func_8005D418 */
    /* +0x50 */ u8 pad50[0x58 - 0x50];
    /* +0x58 */ s32 unk58;             /* passed to func_8002CD08/func_8002CC84 */
    /* +0x5C */ u8 pad5C[0x94 - 0x5C];
    /* +0x94 */ void *unk94;            /* passed as func_8001EACC's (still INCLUDE_ASM, code_d294.s) second argument by func_8005DE18; that callee dereferences it at +0xC/+0x14, so it is a pointer to SOME object, real type unconfirmed */
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
    s32 unk4;    /* +0x04, gate flag read by func_8005E480/func_8005E7A8 */
    u8 pad08[0x08];
    s32 unk10;    /* +0x10, written by func_8005ED10/func_8005E480/func_8005E7A8 */
    u8 pad14[0x08];
    s32 unk1C;     /* +0x1C, written by func_8005ED10/func_8005E480/func_8005E7A8 */
    u8 pad20[0x10];
    s32 unk30;      /* +0x30, written by func_8005E7A8 only */
    u8 pad34[0x10];
    s32 unk44;       /* +0x44, written by func_8005E7A8 only */
};

#endif
