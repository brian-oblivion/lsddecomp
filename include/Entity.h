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
 */
typedef struct Entity Entity;
typedef struct EntityMethods EntityMethods;

struct EntityMethods {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *unk04;
    /* +0x08 */ void *(*ctor)(Entity *self, void *arg0, void *arg1, void *arg2); /* New_Entity's call */
    /* +0x0C */ u8 pad0C[0x30 - 0x0C];
    /* +0x30 */ void (*slot30)(Entity *self, s32 arg1);   /* func_8005DAAC */
    /* +0x34 */ u8 pad34[0x60 - 0x34];
    /* +0x60 */ void (*slot60)(Entity *self, s32 arg1);   /* func_8005D9F4 */
    /* +0x64 */ u8 pad64[0x114 - 0x64];
    /* +0x114 */ void (*slot114)(Entity *self);           /* func_8005DB8C */
    /* +0x118 */ u8 pad118[0x130 - 0x118];
    /* +0x130 */ void (*slot130)(Entity *self);           /* func_8005DB8C */
};

/* Field offsets derived from this unit's own functions (func_8005D6D4,
 * func_8005D9F4, func_8005DAAC, func_8005DB8C, and the Get*Effect/Stage/Video
 * family). `unk9C` is only ever address-taken (passed as an output buffer to
 * two still-uncarved functions, func_8002CD08/func_8002CC84), never read
 * here, so its true size/shape is unconfirmed -- it's padded out only as far
 * as +0xF0, where the next known field starts. */
struct Entity {
    /* +0x00 */ EntityMethods *methods;
    /* +0x04 */ u8 pad04[0x24 - 0x04];
    /* +0x24 */ s32 unk24;             /* cleared by func_8005D9F4 */
    /* +0x28 */ u8 pad28[0x58 - 0x28];
    /* +0x58 */ s32 unk58;             /* passed to func_8002CD08/func_8002CC84 */
    /* +0x5C */ u8 pad5C[0x98 - 0x5C];
    /* +0x98 */ s32 moodIndex;         /* selects a 16-byte row in the D_80089EAxx tables */
    /* +0x9C */ u8 unk9C[0xF0 - 0x9C]; /* address-of target only, see above */
    /* +0xF0 */ s32 unkF0;             /* set to 1 by func_8005D9F4 */
    /* +0xF4 */ s32 unkF4;             /* set from func_8005DAAC's arg1 */
    /* +0xF8 */ s32 unkF8;             /* cleared by func_8005DB8C */
    /* +0xFC */ s32 unkFC;             /* incremented by func_8005D6D4 */
};

extern EntityMethods *Get_vtable_Entity(void);
extern void *func_80017B34(s32 size);
extern void func_80017CFC(void *arg);

/* Both still uncarved (no asm/nonmatchings file -- library or not-yet-carved
 * game code); called directly by name (jal), not through a vtable, so they
 * need a real extern prototype per CLAUDE.md's "calling into a function that
 * is still INCLUDE_ASM" guidance. Neither call site here uses the return
 * value, so void is a safe read regardless of the real return type. */
extern void func_8002CD08(s32 arg0, void *arg1);
extern void func_8002CC84(s32 arg0, void *arg1);

/* The four mood-indexed table lookups (D_80089EA4/A6/AB/AC), all sharing the
 * `this->moodIndex * 16` index computed independently in each function --
 * retail recomputes it every time rather than caching, so each of these is
 * written the same way. Table element types are `s8` (signed byte loads),
 * not `char`, despite `-funsigned-char` making plain `char` unsigned project-
 * wide -- these tables are explicitly `lb`, not `lbu`, in every user seen so
 * far (contrast Entity__GetUnlockEffect's `D_80089EA6`, also `lb`). */
extern u8 D_80089EA4[]; /* GetMoodEffect: base of an array of unknown-shaped entries, 16 bytes/entry */
extern s8 D_80089EA6[];  /* GetUnlockEffect */
extern s8 D_80089EAB[];  /* GetLinkStage */
extern s8 D_80089EAC[];  /* GetEventVideo */

void *Entity__GetMoodEffect(Entity *this);
s32 Entity__GetEventVideo(Entity *this);
s32 Entity__GetUnlockEffect(Entity *this);
s32 Entity__GetLinkStage(Entity *this);

#endif
