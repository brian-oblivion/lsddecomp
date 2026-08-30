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

#endif
