/*
 * code_39094 -- GAME code carved from psyq_39094 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x39094..0x39C80 (vram 0x80048894..0x80049480). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the methods of D_80081940
 * and helpers called only from Class6D3C8, Obj865C8 and ObjM code (stream
 * tasks, the intro logo sequence).
 *
 * Round 82 matched twenty: the allocator, ctor/finalize and small methods of
 * D_80081940, and the record accessors. The eight larger bodies left are
 * still INCLUDE_ASM.
 */
#include "common.h"
#include "Class6D430.h"
#include "StageGrid.h"

typedef struct D_80081940Obj D_80081940Obj;

/* D_80081940's own method table (local view): the Class6D430 interface plus
 * this class's extra slots. */
typedef struct D_80081940Methods {
    CLASS6D430_SLOTS(D_80081940Obj, (D_80081940Obj *self));
    /* +0x07C */ void *slot7C;
    /* +0x080 */ void *slot80;
    /* +0x084 */ void (*releaseAlloc)(D_80081940Obj *self);  /* func_80048C98 */
} D_80081940Methods;

/* The D_80081940 object: a Class6D430 data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
struct D_80081940Obj {
    CLASS6D430_FIELDS(D_80081940Methods);
    /* +0x02C */ u16 unk2C;
    /* +0x02E */ u16 unk2E;
    /* +0x030 */ s16 unk30;
    /* +0x032 */ u16 unk32;
    /* +0x034 */ void *unk34;   /* BMemPMgr allocation, released by slot +0x084 */
    /* +0x038 */ s32 unk38;
};

/* One 0x1C-byte record of the table func_80048D48 returns (D_80081A04,
 * 0x230 records); only its size is known here. */
typedef struct Rec1C {
    u8 data[0x1C];
} Rec1C;

extern int rand(void);
extern void srand(unsigned int seed);
extern void *BMemPMgrFree(void *ptr);
extern Class6D430Methods *GetActiveDataSourceMethods(void);
extern void *BMemPMgrAlloc(s32 size);
void *func_80048CE0(void);

/* allocator: new D_80081940 object */
D_80081940Obj *func_80048894(void) {
    D_80081940Obj *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        ((Class6D430Methods *)func_80048CE0())->ctor((Class6D430 *)obj);
        return obj;
    }
    return NULL;
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800488E4);
/* slot +0x00C of D_80081940 (finalize) */
void func_80048960(D_80081940Obj *self) {
    self->methods->releaseAlloc(self);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800489B4);
/* slot +0x074 of D_80081940 (cancelRequests) */
void func_80048A68(D_80081940Obj *self) {
    GetActiveDataSourceMethods()->cancelRequests((Class6D430 *)self);
    self->unk2C = 0;
    self->unk2E = 0;
    self->unk2A = 0;
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048AAC);
void func_80048B78(D_80081940Obj *self) {
    self->methods->freeBuffer(self);
    self->unk2C = 0;
    self->unk30 = -1;
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80048BC0);
/* slot +0x084 of D_80081940 */
void func_80048C98(D_80081940Obj *self) {
    self->unk2E = 0;
    if (self->unk34 != NULL) {
        self->unk34 = BMemPMgrFree(self->unk34);
    }
}
extern u8 D_80081940[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 D_8008A964;
extern s32 D_8008A968;
extern u8 D_800819CC[];
extern u8 D_80081A04[];
extern char *D_8008A96C;  /* -> "SND\\SE" */
extern const char D_800113DC[];
extern s16 D_80086170[];
extern s16 D_800819E8[];

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
s32 func_80048CFC(s32 seed, s32 unused) {
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
Rec1C *func_80048E2C(s32 index) {
    return &((Rec1C *)func_80048D48(NULL))[D_800819E8[index]];
}

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
Rec1C *func_80049060(s32 index, s32 sub) {
    return &func_8004903C(index)[sub];
}
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
Rec1C *func_800491CC(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 7;
    }
    return &((Rec1C *)func_80048D48(NULL))[0x237];
}
Rec1C *func_800491FC(s32 *countOut) {
    s32 count;
    Rec1C *rec = func_800491CC(&count);
    if (countOut != NULL) {
        *countOut = count;
    }
    return rec;
}
Rec1C *func_80049240(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 8;
    }
    return &((Rec1C *)func_80048D48(NULL))[0x238];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049270);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800492D0);
INCLUDE_ASM("asm/nonmatchings/code_39094", func_80049334);
s32 func_800493C8(s32 index) {
    return D_80086170[index];
}
INCLUDE_ASM("asm/nonmatchings/code_39094", func_800493E4);
