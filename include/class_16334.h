#ifndef CLASS_16334_H
#define CLASS_16334_H

#include "common.h"

/* Class table at D_8006D370 (see tools/classtable.py D_8006D370). 21 slots:
 * header + 14 slots inherited verbatim from BASICCLASS_METHODS (D_8006B58C,
 * returned by func_80018390) + 7 slots this class adds/overrides (+0x08
 * ctor, +0x0C dtor, +0x40.. new virtuals).
 *
 * Working hypothesis (see docs/match-reports for the evidence trail): this
 * class is a thin C wrapper around the Psy-Q Pad library. func_80025EAC/
 * func_80025EFC/func_80025F2C -- called from this unit's ctor/dtor/updater --
 * disassemble as part of the `psyq_PadInit` segment (config/splat...yaml,
 * file offset 0x166ac), and func_80025E1C copies its default button-mask
 * table from `D_80010764`, which itself sits inside the `psyq_15d04` rodata
 * blob -- i.e. this class copies a Psy-Q-owned constant. Held/released/
 * pressed edge-detection (func_80025CC4) and a priority-ordered per-bit
 * event dispatch (func_80025D10) are exactly the shape of a game-side Pad
 * wrapper. Names below are chosen on that hypothesis; not yet in
 * config/symbols (out of this unit's scope to rename).
 */

typedef struct Pad Pad;
typedef struct PadMethods PadMethods;

struct PadMethods {
    /* +0x00 */ s32 header;   /* class-id/flags word; meaning open project-wide (see DECOMPILATION_LEARNINGS.md) */
    /* +0x04 */ void *unk04;  /* == BasicClass__func_17eb0, inherited, unused by this unit */
    /* +0x08 */ void (*ctor)(Pad *self, void *arg1, s32 port);
    /* +0x0C */ void *(*dtor)(Pad *self);
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
    /* +0x38 */ void *unk38;  /* == BasicClass__func_18358, inherited, unused by this unit */
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

/* BasicClass's own table (D_8006B58C) -- returned by func_80018390, which
 * lives in the still-uncarved code_8220 segment. Only the two slots this
 * unit calls are typed here. */
typedef struct BasicClassMethods {
    /* +0x00 */ s32 header;
    /* +0x04 */ void *unk04;
    /* +0x08 */ void *(*ctor)(void *self);
    /* +0x0C */ void *(*dtor)(void *self);
} BasicClassMethods;

extern BasicClassMethods *func_80018390(void);
extern void *func_80017B34(s32 size, s32 zone);

/* Psy-Q Pad library helpers (asm/psyq_PadInit.s, uncarved). */
extern void func_80025EAC(void *arg1);
extern u32 func_80025EFC(s32 port);
extern void func_80025F2C(void);

extern PadMethods D_8006D370;
extern s32 D_8008A848;          /* live-instance counter; the first ctor registers the Pad ISR, the last dtor tears it down */
extern u32 D_8008B388[16];      /* runtime copy of the button-mask table, filled by func_80025E1C */

/* A 0x40-byte block, copied as a whole (GCC's inlined block-move codegen for
 * a struct assignment, not a word loop) rather than word-indexed. */
typedef struct { u32 w[16]; } Block64;
extern Block64 D_80010764;      /* Psy-Q's own default button-mask table (psyq_15d04 rodata) */

PadMethods *func_80025E9C(void);
Pad *func_80025B34(void *arg1, s32 port);
void func_80025BA0(Pad *self, void *arg1, s32 port);
void *func_80025C30(Pad *self);
void func_80025C84(Pad *self, s32 port);
u32 func_80025CC4(Pad *self);
void func_80025D10(Pad *self);
void func_80025E1C(void);

#endif
