/*
 * code_39094 -- GAME code carved from psyq_39094 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x39094..0x39C80 (vram 0x80048894..0x80049480). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the methods of D_80081940
 * and helpers called only from Class6D3C8, Obj865C8 and ObjM code (stream
 * tasks, the intro logo sequence).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "Class6D430.h"

/* The D_80081940 object: a Class6D430 data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
typedef struct D_80081940Obj {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u16 unk2C;
    /* +0x02E */ u16 unk2E;
    /* +0x030 */ u8 pad30[4];
    /* +0x034 */ void *unk34;   /* BMemPMgr allocation, released by slot +0x084 */
    /* +0x038 */ s32 unk38;
} D_80081940Obj;

/* One 0x1C-byte record of the table func_80048D48 returns (D_80081A04,
 * 0x230 records); only its size is known here. */
typedef struct Rec1C {
    u8 data[0x1C];
} Rec1C;

extern int rand(void);
extern void srand(unsigned int seed);
extern void *BMemPMgrFree(void *ptr);
extern Class6D430Methods *GetActiveDataSourceMethods(void);

INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048894);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800488E4);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048960);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800489B4);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048A68);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048AAC);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048B78);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048BC0);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048C98);
extern u8 D_80081940[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 D_8008A964;
extern s32 D_8008A968;
extern u8 D_800819CC[];
extern u8 D_80081A04[];
extern char *D_8008A96C;  /* -> "SND\\SE" */
extern const char D_800113DC[];
extern s16 D_80086170[];

/* slot +0x088 of D_80081940 */
void func_80048CD8(D_80081940Obj *self, s32 value) {
    self->unk38 = value;
}
void *func_80048CE0(void) {
    return D_80081940;
}
s32 func_80048CF0(void) {
    return D_8008A960;
}
s32 func_80048CFC(s32 seed) {
    if (seed != 0) {
        srand(seed);
    }
    return rand();
}
void func_80048D28(s32 a, s32 b) {
    if (a >= 0) {
        D_8008A964 = a;
    }
    if (b >= 0) {
        D_8008A968 = b;
    }
}
void *func_80048D48(s32 *out) {
    if (out != NULL) {
        *out = 0x230;
    }
    return D_80081A04;
}
void *func_80048D64(void) {
    return D_800819CC;
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048D74);
char **func_80048DF8(void) {
    return &D_8008A96C;
}
char *func_80048E08(void) {
    return *func_80048DF8();
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048E2C);
Rec1C *func_80048E2C(s32 index);

Rec1C *func_80048E80(s32 index) {
    return func_80048E2C(index);
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048EA0);
Rec1C *func_80048F60(s32 index) {
    return &func_80048E2C(index)[4];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048F84);
Rec1C *func_8004903C(s32 index) {
    return &func_80048E2C(index)[9];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049060);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049098);
const char *func_800490F4(s32 *typeCodeOut) {
    if (typeCodeOut != NULL) {
        *typeCodeOut = 0x31;
    }
    return D_800113DC;
}
Rec1C *func_80049110(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 0;
    }
    return &((Rec1C *)func_80048D48(NULL))[0x230];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_8004913C);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800491CC);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800491FC);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049240);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049270);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800492D0);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049334);
s32 func_800493C8(s32 index) {
    return D_80086170[index];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800493E4);
