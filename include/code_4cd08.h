#ifndef CODE_4CD08_H
#define CODE_4CD08_H

#include "common.h"

/* This unit is lsddecomp's "DreamAux". It owns the 0x206C rodata slot (its
 * switch jump tables) and a small family of "tick this class instance"
 * slots living at 0x80088D28 / 0x80088D2C -- see match reports for
 * func_8005C508, func_8005C5E8 and func_8005C76C for how these were derived.
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
 * result back into the same slot); a second field at +0x4 that
 * func_8005C650 (MATCHED round 43) sets to the result of a `New_Entity`
 * call and func_8005CF34 (MATCHED round 43) dispatches through its vtable;
 * and a 3-word position vector at +0x8 that func_8005CF34 passes as
 * `func_8001E600`'s `src` (that function's own signature, `code_d294.h`,
 * takes `s32 *src` and treats it as a 3-word vector). Stride is 0x14,
 * confirmed by func_8005C650's walk over D_80088D28. */
typedef struct DreamAuxSlot {
    void *obj;
    DreamAuxObj *entity;
    s32 pos[3];
} DreamAuxSlot;

extern DreamAuxSlot D_80088D28[14];
extern DreamAuxSlot D_80088D2C[14];

/* Two more vtable slots on the DreamAuxObj family (see func_8005CF34):
 * slot 0x14 (byte offset 0x50) takes only self, slot 0x13 (byte offset
 * 0x4C) takes self plus four opaque values. This is the SAME slot-0x4C
 * shared-ancestor entry `include/Entity.h` documents on `EntityMethods`'
 * base (`void (*slot4C)(void *self, s32, s32, void *, s32)`) -- but that
 * call site's 4th argument is a scalar where func_8005CF34's is a pointer
 * to a locally-filled 3-word vector, so this unit keeps its own local
 * view rather than importing Entity.h's. */
typedef void (*DreamAuxObjFn14)(DreamAuxObj *self);
typedef void (*DreamAuxObjFn13)(DreamAuxObj *self, s32 arg1, s32 arg2, void *arg3, void *arg4);

/* A tiny fixed-size record family read by func_8005C508: 14 (0xE) parallel
 * groups, D_80089A7C[i] a signed count and D_80089A44[i] a pointer to an
 * array of count 8-byte records whose first byte func_8005C508 clears. The
 * record's remaining 7 bytes are not accessed here. */
typedef struct DreamAuxGroupRecord {
    s8 flag;
    u8 pad1[7];
} DreamAuxGroupRecord;

extern s8 D_80089A7C[];
extern DreamAuxGroupRecord *D_80089A44[];

/* A second parallel-group family, same "count + pointer to array" shape as
 * DreamAuxGroupRecord above but a different stride and a different index
 * space: 14 (0xE) groups selected by `D_8008ABF8` (not a loop index),
 * D_80089AC4[i] a signed count, D_80089A8C[i] a pointer to an array of
 * count 6-byte records whose first 2 bytes (`key`, read with `lh`) are the
 * only field func_8005C8AC accesses. The remaining 4 bytes are undiscovered
 * from this unit alone. */
typedef struct DreamAuxTriggerEntry {
    s16 key;
    u8 unk2[4];
} DreamAuxTriggerEntry;

extern s8 D_80089AC4[];
extern DreamAuxTriggerEntry *D_80089A8C[];

/* A small signed-byte lookup table read by func_8005CD58, indexed by its
 * `idx` parameter. Layout beyond "one signed byte per entry" is not known
 * from this unit alone. */
extern s8 D_80088D16[];

/* "ETC\\SYMSPY.MOM" / "ETC\\SYMDOG.MOM" -- MOM = this game's audio-stream
 * format (per lsddecomp naming elsewhere in the project). Defined in
 * code_4cd08.c, right before func_8005C508 which is their only reader. */
extern const char D_8001186C[];
extern const char D_8001187C[];

/* A 3-word request record, physically the same shape as code_171e0.h's
 * Vec3_171e0 (func_80026CE8 there does `this->x=x; this->y=y; this->z=z;
 * return this;` regardless of what the caller's fields actually mean) but
 * used here to hold a load flag, a MOM filename pointer, and a mode byte for
 * func_8004468C. Declared locally because code_4cd08.c does not otherwise
 * need code_171e0.h. */
typedef struct DreamAuxLoadReq {
    s32 flag;
    const char *name;
    s32 mode;
} DreamAuxLoadReq;

extern DreamAuxLoadReq *func_80026CE8(DreamAuxLoadReq *this, s32 flag, const char *name, s32 mode);
extern void *func_8004468C(DreamAuxLoadReq *req);

/* A trigger/spawn record walked by func_8005CAB4 and func_8005CBC8. Only
 * three fields and the overall stride (0x38 -- func_8005CAB4 recurses on
 * `record + 1`, i.e. the next record in what is evidently an array) are
 * established:
 *  - offset 0x1: a selector func_8005CBC8 switches on (its own param, not
 *    yet named the same as `kind` below -- may or may not be the same
 *    logical field; not proven either way).
 *  - offset 0x2 (`parity`): compared against a caller-supplied coordinate
 *    parity by func_8005C9A4's `entry` parameter -- same struct, most
 *    likely, given the shared 8-byte-ish record shape in this unit, but
 *    that function takes a raw `s8 *` and was matched without this type.
 *  - offset 0x3 (`kind`): read by func_8005CAB4 for func_8005C714 and
 *    func_8005CDF8's first argument, and compared against the literal `2`
 *    to decide whether to recurse into the next record.
 *  - offset 0x4..0x7 (`entries`): up to 4 signed bytes, terminated early by
 *    a `-1` sentinel, each tried against func_8005CDF8.
 * Everything else is undiscovered padding. */
typedef struct TriggerRecord {
    s8 unk0;
    s8 sel;
    s8 parity;
    u8 kind;
    s8 entries[4];
    u8 unk8[0x30];
} TriggerRecord;

/* The `a3` object func_8005CAB4 receives: method table at offset 0 (see
 * CLAUDE.md's "every object's method table pointer lives at offset 0"),
 * slot 0x88 (index 0x22 as a pointer array) called with (self, parity). Not
 * resolved against tools/classtable.py -- the concrete class is unknown
 * from this function alone. */
typedef struct TriggerWorld {
    void **vtable;
} TriggerWorld;

typedef void *(*TriggerWorldFn)(TriggerWorld *self, s8 parity);

/* vtable slot 0x80 (byte offset 0x200) of a TriggerWorld-shaped object:
 * takes only `self`, returns a value compared against a caller value.
 * Distinct arity/slot from TriggerWorldFn above -- same object family
 * (per D_8008AC00, the only TriggerWorld-typed global known so far),
 * different vtable entry. Used by func_8005CD58 and func_8005C930. */
typedef s32 (*TriggerWorldFn80)(TriggerWorld *self);

extern bool func_8005CBC8(s32 value, TriggerRecord *record);
/* `out` is a 4-word (0x10-byte) caller stack scratch buffer, reused across
 * every call in func_8005CAB4's loop. Its LAST word is pre-populated by the
 * caller with the return value of the TriggerWorld vtable-0x88 call before
 * the loop starts (`scratch[3] = (s32)callResult;` in func_8005CAB4) --
 * confirmed load-bearing: the match was 19/69 without it, 69/69 with it, no
 * other change. func_8005CDF8 is still INCLUDE_ASM (gp-relative-blocked,
 * see docs/research/gp-relative-blocker.md), so its own use of that word is
 * not derived here. */
extern bool func_8005CDF8(u8 kind, void *out, void *ctx, u8 entry);
extern void func_8005C714(s32 triggerType);
extern bool func_8005630C(void);
extern bool func_8005CD58(s32 idx);
extern bool func_8005CDA8(s32 a0, s32 a1);

#endif
