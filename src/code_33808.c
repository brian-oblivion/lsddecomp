/*
 * code_33808 -- GAME code carved from the head of psyq_33808 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x33808..0x36654 (vram
 * 0x80043008..0x80045E54). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the methods of eleven tables (D_8006F0B8,
 * D_8006F13C, D_8006F1C4, D_8006F240, D_8006F2C4, D_8006F384, D_8006F40C,
 * D_8006F498, D_8006F514, D_8006F590, D_8006F614) and nine slots of
 * D_8006D430 (Class6D430, the data-source interface): from +0x07C that table
 * lists `Get...Methods` getters (4-word `return &table` stubs) and nine of
 * them are here, one per class. The methods call Lock/UnlockActiveDataSource
 * and GetClass6B5CCMethods. libpress starts right after, at DecDCTReset
 * (now psyq_36654).
 *
 * Matching began in round 82: `grep -c INCLUDE_ASM` gives what is left.
 */
#include "common.h"
#include "BasicClass.h"
#include "Class6B5CC.h"
#include "Class6D430.h"

typedef struct DataSrc33808 DataSrc33808;

/* Unit-local view of this unit's Class6D430 data-source subclasses: the
 * interface plus the extra slots their methods call. Unprototyped where a
 * caller passes no argument. */
typedef struct DataSrc33808Methods {
    CLASS6D430_SLOTS(DataSrc33808, (DataSrc33808 *self));
    /* +0x07C */ s32 (*slot7C)();
    /* +0x080 */ void *(*slot80)();
} DataSrc33808Methods;

struct DataSrc33808 {
    CLASS6D430_FIELDS(DataSrc33808Methods);
    /* +0x02C */ s32 unk2C;
    /* +0x030 */ DataSrc33808 *unk30;  /* forwarded to by +0x080/+0x084 of D_8006F384/D_8006F40C */
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 unk38;            /* D_8006F40C: count; D_8006F498: an allocation */
};

/* A buffer that starts with a word, a count, then that many words. */
typedef struct CountedBuf33808 {
    /* +0x00 */ s32 unk0;
    /* +0x04 */ u32 count;
    /* +0x08 */ s32 entries[1];
} CountedBuf33808;

extern Class6D430Methods *GetActiveDataSourceMethods(void);
extern void ReleaseBasicClassArray(BasicClass **array, s32 count);
extern void BMemPMgrFree(void *arg);
void *func_800441A4(void);
void *func_800449FC(void);

INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043008);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043068);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800431A8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043200);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800434DC);
/* D_8006F0B8 +0x078: set entry `index`'s shift, and its mask from it. */
typedef struct Ent6F0B8 {
    /* +0x00 */ u16 shift;
    /* +0x02 */ u16 mask;
    /* +0x04 */ u8 pad4[0xC];
} Ent6F0B8;

typedef struct Obj6F0B8 {
    /* +0x000 */ u8 pad0[0x40];
    /* +0x040 */ Ent6F0B8 entries[1];
} Obj6F0B8;

void func_80043538(Obj6F0B8 *self, s32 index, s32 shift) {
    Ent6F0B8 *e = &self->entries[index];

    e->shift = shift;
    e->mask = 1 << e->shift;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004355C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800435D0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043648);
extern s32 D_8006F0B8[];

void *func_80043830(void) {
    return D_8006F0B8;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043840);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800438B0);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043954);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800439EC);
/* LIBGS.H: void GsMapModelingData(unsigned long *p); */
void GsMapModelingData(u32 *p);

/* D_8006F13C +0x078: map the TMD in the buffer (past its id word). */
void func_80043B18(Class6D430 *self) {
    GsMapModelingData((u32 *)self->buffer + 1);
}
/* D_8006F13C +0x07C: the address of record `index`, 0x1C bytes each,
 * from +0x0C of the buffer. */
typedef struct Rec6F13C {
    u8 data[0x1C];
} Rec6F13C;

typedef struct Buf6F13C {
    /* +0x00 */ u8 pad0[0xC];
    /* +0x0C */ Rec6F13C recs[1];
} Buf6F13C;

Rec6F13C *func_80043B3C(Class6D430 *self, s32 index) {
    return &((Buf6F13C *)self->buffer)->recs[index];
}
/* D_8006F13C +0x080: returns entry `index` of the word array at +0x2C
 * (the first field past the 0x2C-byte Class6D430 base). */
typedef struct Obj6F13C {
    /* +0x000 */ u8 pad0[0x2C];
    /* +0x02C */ s32 *entries;
} Obj6F13C;

s32 func_80043B58(Obj6F13C *self, s32 index) {
    return self->entries[index];
}
void func_80043B70(void) {
}
extern s32 D_8006F13C[];

void *func_80043B78(void) {
    return D_8006F13C;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043B88);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043BE8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043C60);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043CB8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043DFC);
extern s32 D_8006F1C4[];

void *func_80043E74(void) {
    return D_8006F1C4;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043E84);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043EE4);
/* D_8006F240 +0x00C: finalize, straight to the active driver's. */
void func_80043F78(Class6D430 *self) {
    GetActiveDataSourceMethods()->finalize(self);
}
/* D_8006F240 +0x078: slot +0x07C over the buffer past its first two words. */
u8 func_80043FB0(DataSrc33808 *self, s32 arg1, s32 arg2) {
    return self->methods->slot7C(self, arg1, arg2, (u8 *)self->buffer + 8);
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80043FE4);
/* D_8006F240/D_8006F590 +0x080: decode one packet word -- the low byte, then
 * the two nibbles at bits 16 and 20, then the top byte -- and return the
 * pointer past it. */
u32 *func_8004416C(DataSrc33808 *self, u32 *acc, u8 *out0, u8 *out1, u8 *out2, u8 *out3) {
    u32 v = *acc;

    *out0 = v;
    *out1 = (v >> 16) & 0xF;
    *out2 = (v >> 20) & 0xF;
    *out3 = v >> 24;
    return acc + 1;
}
extern s32 D_8006F240[];

void *func_800441A4(void) {
    return D_8006F240;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800441B4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044220);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044294);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044380);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004441C);
/* D_8006F2C4 (a Class6B5CC subclass) +0x0B8: when `enable`, copy a
 * three-byte vector to +0x54. */
typedef struct Vec3S8 {
    s8 x;
    s8 y;
    s8 z;
} Vec3S8;

typedef struct Obj6F2C4 {
    CLASS6B5CC_FIELDS(Class6B5CCMethods);
    /* +0x044 */ u8 pad44[0x10];
    /* +0x054 */ Vec3S8 unk54;
} Obj6F2C4;

void func_8004464C(Obj6F2C4 *self, s32 enable, Vec3S8 *src) {
    if (enable) {
        self->unk54 = *src;
    }
}
void func_80044674(void) {
}
extern s32 D_8006F2C4[];

void *func_8004467C(void) {
    return D_8006F2C4;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004468C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800446FC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800447B4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044808);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044858);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800448F8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004497C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800449B8);
extern s32 D_8006F384[];

void *func_800449FC(void) {
    return D_8006F384;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044A0C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044A7C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044B04);
/* D_8006F40C +0x064: slot +0x078. */
void func_80044B58(DataSrc33808 *self) {
    ((s32 (*)())self->methods->slot78)(self);
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044B88);
/* D_8006F40C +0x07C: release the object array in the buffer (past its first
 * two words), +0x38 entries long, and zero the count. */
void func_80044C58(DataSrc33808 *self) {
    ReleaseBasicClassArray((BasicClass **)((u8 *)self->buffer + 8), self->unk38);
    self->unk38 = 0;
}
/* D_8006F40C +0x088: entry `index` of the buffer's counted word array, 0 when
 * out of range. */
s32 func_80044C90(DataSrc33808 *self, u32 index) {
    CountedBuf33808 *buf = self->buffer;

    if (index < buf->count) {
        return buf->entries[index];
    }
    return 0;
}
extern s32 D_8006F40C[];

void *func_80044CC4(void) {
    return D_8006F40C;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044CD4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044D40);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044DC8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044E10);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044E64);
extern s32 D_8006F498[];

void *func_80044F20(void) {
    return D_8006F498;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044F30);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80044F90);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004500C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045060);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800450B4);
extern s32 D_8006F514[];

void *func_800451A8(void) {
    return D_8006F514;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800451B8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045228);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800452AC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800452FC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800453DC);
extern s32 D_8006F590[];

void *func_80045428(void) {
    return D_8006F590;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045438);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800454C4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800455D4);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004564C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_8004575C);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800457C0);
/* A class with an s32 at +0x50, set to 1 / -1 by the two setters below;
 * the class is not yet identified (neither setter sits in a method table). */
typedef struct Obj33808_50 {
    u8 pad0[0x50];
    s32 unk50;
} Obj33808_50;

void func_800458AC(Obj33808_50 *self) {
    self->unk50 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_800458B8);
void func_8004593C(Obj33808_50 *self) {
    self->unk50 = -1;
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045948);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045A38);
void func_80045AC8(void) {
}
void func_80045AD0(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045AD8);
void func_80045BC0(void) {
}
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045BC8);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045C94);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045CFC);
INCLUDE_ASM("asm/nonmatchings/code_33808", func_80045DE0);
/* Hang until +0x4C is nonzero (it is read once). */
typedef struct Obj45E18 {
    u8 pad0[0x4C];
    s32 unk4C;
} Obj45E18;

void func_80045E18(Obj45E18 *self) {
    while (self->unk4C == 0) {
    }
}
/* D_8006F614 +0x06C: stores its argument at +0x68. */
typedef struct Obj6F614 {
    u8 pad0[0x68];
    s32 unk68;
} Obj6F614;

void func_80045E3C(Obj6F614 *self, s32 value) {
    self->unk68 = value;
}
extern s32 D_8006F614[];

void *func_80045E44(void) {
    return D_8006F614;
}
