#ifndef CODE_4CD08_H
#define CODE_4CD08_H

#include "common.h"

/* This unit is lsddecomp's "DreamAux". Fully matched, round 43 (0
 * INCLUDE_ASM); track 3 naming pass round 63. It owns the 0x206C rodata
 * slot (its switch jump tables) and manages a small "trigger record" system:
 * a table of 8-byte TriggerRecord entries, each gating on a caller-supplied
 * `value` (CheckDreamAuxTriggerCondition) and a coordinate parity
 * (CheckTriggerParity), that on success spawns or despawns an Entity into
 * one of two 14-slot object-tracking families (SpawnDreamAuxTriggerEntity /
 * DespawnDreamAuxEntity, backed by gDreamAuxSlots / gDreamAuxSlots2) and can
 * gate the game's teleport flag (EnableTeleportsForKind, SetTeleportsEnabled
 * in DreamSys.c). InitDreamAux/TickDreamAuxSlots/TickDreamAuxSlots2 are the
 * construct/tick/destruct hooks a caller in class_39e08.c and
 * class_3bb8c_l.c drives this subsystem through. `gDreamAuxStage`,
 * `gDreamAuxWorld` and three sibling globals SetDreamAuxWorld installs are
 * the shared context every other function in the unit reads.
 */

/* An object whose method table pointer sits at offset 0 (every object in
 * this game's class framework, per CLAUDE.md's "Writing a class method").
 * Only slot 1 (offset 0x4 in the table) is known here: a "tick" method that
 * takes the object and returns a (possibly new/updated) object pointer. */
typedef struct DreamAuxObj {
    void **vtable;
} DreamAuxObj;

typedef DreamAuxObj *(*DreamAuxTickFn)(DreamAuxObj *self);

/* A slot in the 0x80088D28 / 0x80088D2C families: one live-object pointer
 * (ticked once per call by calling obj->vtable[1](obj) and storing the
 * result back into the same slot); an Entity at +0x4 (include/Entity.h)
 * that SetDreamAuxWorld makes with New_Entity and DespawnDreamAuxEntity
 * detaches and re-attaches (detachFromParent, attachToParent with the
 * player gDreamAuxWorld as the peer);
 * and a 3-word position vector at +0x8 that DespawnDreamAuxEntity passes as
 * `Class6B5CC__LocalOffsetToWorldPos`'s `src` (that function's own signature, `code_d294.h`,
 * takes `s32 *src` and treats it as a 3-word vector). Stride is 0x14,
 * confirmed by SetDreamAuxWorld's walk over gDreamAuxSlots. */
typedef struct DreamAuxSlot {
    void *obj;
    struct Entity *entity;
    s32 pos[3];
} DreamAuxSlot;

extern DreamAuxSlot gDreamAuxSlots[14];
extern DreamAuxSlot gDreamAuxSlots2[14];

/* A tiny fixed-size record family read by InitDreamAux: 14 (0xE) parallel
 * groups, gDreamAuxGroupCounts[i] a signed count and gDreamAuxGroupRecords[i] a pointer to an
 * array of count 8-byte records whose first byte InitDreamAux clears. The
 * record's remaining 7 bytes are not accessed here. */
typedef struct DreamAuxGroupRecord {
    s8 flag;
    u8 pad1[7];
} DreamAuxGroupRecord;

extern s8 gDreamAuxGroupCounts[];
extern DreamAuxGroupRecord *gDreamAuxGroupRecords[];

/* A second parallel-group family, same "count + pointer to array" shape as
 * DreamAuxGroupRecord above but a different stride and a different index
 * space: 14 (0xE) groups selected by `gDreamAuxStage` (not a loop index),
 * gDreamAuxTriggerCounts[i] a signed count, gDreamAuxTriggerEntries[i] a pointer to an array of
 * count 6-byte records whose first 2 bytes (`key`, read with `lh`) are the
 * only field LookupDreamAuxTrigger accesses. The remaining 4 bytes are undiscovered
 * from this unit alone. */
typedef struct DreamAuxTriggerEntry {
    s16 key;
    u8 unk2[4];
} DreamAuxTriggerEntry;

extern s8 gDreamAuxTriggerCounts[];
extern DreamAuxTriggerEntry *gDreamAuxTriggerEntries[];

/* A small signed-byte lookup table read by CheckDreamAuxWorldState, indexed by its
 * `idx` parameter. Layout beyond "one signed byte per entry" is not known
 * from this unit alone. */
extern s8 D_80088D16[];

/* "ETC\\SYMSPY.MOM" / "ETC\\SYMDOG.MOM" -- MOM = this game's audio-stream
 * format (per lsddecomp naming elsewhere in the project). Defined in
 * code_4cd08.c, right before InitDreamAux which is their only reader. */
extern const char gMomPathSymSpy[];
extern const char gMomPathSymDog[];

/* A 3-word request record, physically the same shape as code_171e0.h's
 * Vec3_171e0 (SetVec3 there does `this->x=x; this->y=y; this->z=z;
 * return this;` regardless of what the caller's fields actually mean) but
 * used here to hold a load flag, a MOM filename pointer, and a mode byte for
 * New_ModelData. Declared locally because code_4cd08.c does not otherwise
 * need code_171e0.h. */
typedef struct DreamAuxLoadReq {
    s32 flag;
    const char *name;
    s32 mode;
} DreamAuxLoadReq;

extern DreamAuxLoadReq *SetVec3(DreamAuxLoadReq *this, s32 flag, const char *name, s32 mode);

/* A trigger/spawn record walked by ProcessDreamAuxTriggerRecord and CheckDreamAuxTriggerCondition. Only
 * three fields and the overall stride (0x38 -- ProcessDreamAuxTriggerRecord recurses on
 * `record + 1`, i.e. the next record in what is evidently an array) are
 * established:
 *  - offset 0x0 (`triggered`): 0 until CheckDreamAuxTriggerCondition's `success`
 *    path sets it to 1; while `sel < 0`, a nonzero value here short-circuits
 *    the whole condition check to `false` instead of re-testing `sel`. Reads
 *    as a "fire once" latch, though nothing here explains WHY only the
 *    `sel < 0` path consults it.
 *  - offset 0x1: a selector CheckDreamAuxTriggerCondition switches on (its own param, not
 *    yet named the same as `kind` below -- may or may not be the same
 *    logical field; not proven either way).
 *  - offset 0x2 (`parity`): compared against a caller-supplied coordinate
 *    parity by CheckTriggerParity's `entry` parameter -- same struct, most
 *    likely, given the shared 8-byte-ish record shape in this unit, but
 *    that function takes a raw `s8 *` and was matched without this type.
 *  - offset 0x3 (`kind`): read by ProcessDreamAuxTriggerRecord for EnableTeleportsForKind and
 *    SpawnDreamAuxTriggerEntity's first argument, and compared against the literal `2`
 *    to decide whether to recurse into the next record.
 *  - offset 0x4..0x7 (`entries`): up to 4 signed bytes, terminated early by
 *    a `-1` sentinel, each tried against SpawnDreamAuxTriggerEntity.
 * Everything else is undiscovered padding. */
typedef struct TriggerRecord {
    s8 triggered;
    s8 sel;
    s8 parity;
    u8 kind;
    s8 entries[4];
    u8 unk8[0x30];
} TriggerRecord;

/* ProcessDreamAuxTriggerRecord's `world` is a TriggerWorld (D_8006F40C,
 * include/TriggerWorld.h; FireDreamAuxTriggerEntries gets it from
 * New_TriggerWorld), unified in track 4 (round 88); code_4cd08.c includes
 * that header. This file's former `TriggerWorld { void **vtable; }` view
 * is gone. */

/* gDreamAuxWorld is the player DreamSys (include/DreamSys.h, track 4
 * round 88): the +0x200 its table is called at is getDreamColor, and
 * class_3bb8c_l hands SetDreamAuxWorld its DreamSys `target`. code_4cd08.c
 * declares it; the DreamAuxWorld view that stood here is gone. */

extern bool CheckDreamAuxTriggerCondition(s32 value, TriggerRecord *record);
/* `out` is a 4-word (0x10-byte) caller stack scratch buffer, reused across
 * every call in ProcessDreamAuxTriggerRecord's loop. Its LAST word is pre-populated by the
 * caller with the return value of the TriggerWorld getModelData (+0x088) call before
 * the loop starts (`scratch[3] = (s32)callResult;` in ProcessDreamAuxTriggerRecord) --
 * confirmed load-bearing: the match was 19/69 without it, 69/69 with it, no
 * other change. SpawnDreamAuxTriggerEntity itself MATCHED round 43 (once the
 * gp-relative blocker was resolved, see docs/research/gp-relative-blocker.md)
 * and never reads `out` -- it only forwards it untouched to `New_Entity`'s
 * 2nd argument; its own outgoing buffer is a separate local `outBuf[4]`. */
extern bool SpawnDreamAuxTriggerEntity(s32 kind, void *out, void *ctx, s32 entry);
extern void EnableTeleportsForKind(s32 kind);
extern bool IsStyleVariantEven(void);
extern bool CheckDreamAuxWorldState(s32 idx);
extern bool MatchesDreamAuxProgression(s32 a0, s32 a1);

#endif
