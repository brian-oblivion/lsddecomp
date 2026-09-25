#ifndef CLASS_16334_H
#define CLASS_16334_H

#include "common.h"
#include "BasicClass.h"

/* Class table at gPadMethods (see tools/classtable.py gPadMethods). 21 slots:
 * header + 14 slots inherited verbatim from BASICCLASS_METHODS (D_8006B58C,
 * returned by Get_vtable_BasicClass) + 7 slots this class adds/overrides (+0x08
 * ctor, +0x0C dtor, +0x40.. new virtuals).
 *
 * A game-side wrapper around the Psy-Q Pad library, confirmed (not just
 * hypothesized) by the calls this unit's own matched bodies make: Pad__Pad
 * and Pad__Destroy call PadInit/PadStop (include/psyq/LIBETC.H) on the
 * first/last live instance, and Pad__UpdateMasks calls PadRead directly.
 * Pad__LoadButtonTable copies the SDK's own default 16-entry digital-button
 * mask table (D_80010764, inside the `psyq_15d04` rodata blob) into this
 * unit's runtime copy (`sButtonMasks`). Pad__UpdateMasks turns two
 * consecutive raw reads into held/pressed/released edge masks, and
 * Pad__DispatchEvents (a priority-ordered, reverse-order per-bit scan) is
 * the event fan-out this class's owner installs its own `onButtonEvent`
 * (+0x30) handler into. Round 77 (naming pass): all 8 non-stub functions
 * are tier A; the two vtable-only no-op slots (+0x4C/+0x54,
 * Pad__func_80025E14/Pad__func_80025E94) stay tier C -- both compile to
 * `jr $ra; nop` in retail, and neither slot is ever invoked anywhere in the
 * decompiled tree (src/main.c's only Pad call is `New_Pad(0, 0)`), so there
 * is nothing to derive their purpose from.
 */

typedef struct Pad Pad;
typedef struct PadMethods PadMethods;

struct PadMethods {
    /* +0x00 */ s32 header;   /* class-id/flags word; meaning open project-wide (see DECOMPILATION_LEARNINGS.md) */
    /* +0x04 */ void *unk04;  /* == BasicClass__Release, inherited, unused by this unit */
    /* +0x08 */ void (*ctor)(Pad *self, void *arg1, s32 port);
    /* +0x0C */ void (*dtor)(Pad *self);
    /* +0x10 */ void *unk10;
    /* +0x14 */ void *unk14;
    /* +0x18 */ void *unk18;
    /* +0x1C */ void *unk1C;
    /* +0x20 */ void *unk20;
    /* +0x24 */ void *unk24;
    /* +0x28 */ void *unk28;
    /* +0x2C */ void *unk2C;
    /* +0x30 */ void (*onButtonEvent)(Pad *self, s32 event); /* per-instance overridable; base impl lives outside this unit */
    /* +0x34 */ void *unk34;
    /* +0x38 */ void *unk38;  /* == BasicClass__OnNotify, inherited, unused by this unit */
    /* +0x3C */ void *unk3C;  /* null slot in the base table */
    /* +0x40 */ void (*init)(Pad *self, s32 port);
    /* +0x44 */ u32 (*updateMasks)(Pad *self);
    /* +0x48 */ void (*dispatchEvents)(Pad *self);
    /* +0x4C */ void (*func4C)(void);
    /* +0x50 */ void (*loadButtonTable)(void);
    /* +0x54 */ void (*func54)(void);
};

struct Pad {
    /* +0x00 */ PadMethods *methods;
    /* +0x04 */ u8 unk04[8];   /* BasicClass instance fields; owned by whatever unit decompiles BasicClass itself */
    /* +0x0C */ u16 port;      /* boolified in the ctor: forced to 0 or 1 */
    /* +0x0E */ u8 unk0E[2];
    /* +0x10 */ u32 heldMask;
    /* +0x14 */ u32 releasedMask;
    /* +0x18 */ u32 pressedMask;
    /* +0x1C */ u8 unk1C[4];
};

extern void *BMemPMgrAlloc(s32 size);

/* Psy-Q Pad library helpers (asm/psyq_PadInit.s, uncarved). */
extern void PadInit(void *arg1);
extern u32 PadRead(s32 port);
extern void PadStop(void);

extern PadMethods gPadMethods;
extern s32 sPadRefCount;          /* live-instance counter; the first ctor registers the Pad ISR, the last dtor tears it down */
extern u32 sButtonMasks[16];      /* runtime copy of the button-mask table, filled by Pad__LoadButtonTable */

/* A 0x40-byte block, copied as a whole (GCC's inlined block-move codegen for
 * a struct assignment, not a word loop) rather than word-indexed. */
typedef struct { u32 w[16]; } Block64;
extern Block64 D_80010764;      /* Psy-Q's own default button-mask table (psyq_15d04 rodata) */

PadMethods *Get_vtable_Pad(void);
Pad *New_Pad(void *arg1, s32 port);
void Pad__Pad(Pad *self, void *arg1, s32 port);
void Pad__Destroy(Pad *self);
void Pad__Init(Pad *self, s32 port);
u32 Pad__UpdateMasks(Pad *self);
void Pad__DispatchEvents(Pad *self);
void Pad__LoadButtonTable(void);

#endif
