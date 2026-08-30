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
 * result back into the same slot) plus 0x10 bytes not yet accessed by any
 * function in this unit. Stride is 0x14 (confirmed: func_8005C650, off-limits
 * here per the gp-relative blocker, walks D_80088D28 with the same stride
 * and also writes a second field at +0x4 with a `New_Entity` result). */
typedef struct DreamAuxSlot {
    void *obj;
    u8 unk4[0x10];
} DreamAuxSlot;

extern DreamAuxSlot D_80088D28[14];
extern DreamAuxSlot D_80088D2C[14];

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
    u8 unk0;
    u8 unk1;
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

#endif
