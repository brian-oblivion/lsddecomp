/*
 * code_fa50 -- GAME code carved from psyq_fa50 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0xFA50..0x10D48 (vram 0x8001F250..0x80020548). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the class of method table
 * D_8006BEA0 (new_class_6bea0 allocates it with BMemPMgrAlloc and chains to
 * Get_vtable_BasicClass) and helpers called only from Class6B5CC/BaseObjO
 * code. Owns jtbl_80010354 (attached rodata sub-slot 0xB54).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"

/* The four words the class copies in through its slot +0x040. */
typedef struct Quad_fa50 {
    s32 w[4];
} Quad_fa50;

/* One 28-byte record of the model data slot +0x048 indexes. */
typedef struct Rec28_fa50 {
    u8 pad[0x1C];
} Rec28_fa50;

typedef struct ModelData_fa50 {
    u32 head[3];            /* +0x000; GsMapModelingData gets &head[1] */
    Rec28_fa50 recs[1];     /* +0x00C */
} ModelData_fa50;

typedef struct Class6BEA0 {
    void *vtable;           /* +0x000 */
    u8 pad4[0x8];           /* +0x004 */
    ModelData_fa50 *data;   /* +0x00C */
    void *unk10;            /* +0x010 */
    Quad_fa50 quad;         /* +0x014 */
} Class6BEA0;

typedef struct Target_fa50 {
    u8 pad0[0x6];
    s16 unk6;               /* +0x006 */
} Target_fa50;

typedef struct Inner_fa50 {
    u8 pad0[0x10];
    Target_fa50 *unk10;     /* +0x010 */
} Inner_fa50;

typedef struct Outer_fa50 {
    u8 pad0[0x10];
    Inner_fa50 *unk10;      /* +0x010 */
} Outer_fa50;

extern void GsMapModelingData(unsigned long *p);
extern s32 func_8001F3B0(void *self, void *buf);
extern s32 D_8008AC4C;
extern s32 D_8008B21C[];
extern s32 D_8006BEA0[];

INCLUDE_ASM("asm/nonmatchings/code_fa50", new_class_6bea0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F2B0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F314);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F33C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F360);
void func_8001F37C(void) {
}
void *func_8001F384(void) {
    return D_8006BEA0;
}
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F394);
s32 func_8001F3A4(void *self) {
    return D_8008AC4C;
}
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F3B0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F4E4);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F50C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F51C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F66C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F8B8);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020050);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_800204D0);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020510);
