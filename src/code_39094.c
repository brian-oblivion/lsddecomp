/*
 * code_39094 -- GAME code carved from psyq_39094 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x39094..0x39C80 (vram 0x80048894..0x80049480). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the methods of D_80081940
 * and helpers called only from Class6D3C8, Obj865C8 and ObjM code (stream
 * tasks, the intro logo sequence).
 *
 * Round 82 matched all 28: the allocator, ctor/finalize and methods of
 * D_80081940 (a file-streaming state machine: state 9 = header load, state
 * 10 = data block load, both completed in the setFlag override), and the
 * record accessors and random pickers over func_80048D48's table.
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
    /* +0x080 */ s32 (*slot80)();  /* DataSrc39094__LoadDataBlock(self); unprototyped: DataSrc39094__SetFlag calls it with no argument */
    /* +0x084 */ void (*releaseAlloc)();  /* DataSrc39094__ReleaseDataBlock(self); unprototyped: DataSrc39094__LoadDataBlock calls it with no argument */
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
void *GetDataSrc39094Methods(void);

/* allocator: new D_80081940 object */
D_80081940Obj *New_DataSrc39094(void) {
    D_80081940Obj *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        ((Class6D430Methods *)GetDataSrc39094Methods())->ctor((Class6D430 *)obj);
        return obj;
    }
    return NULL;
}
/* slot +0x008 of D_80081940 (ctor) */
void DataSrc39094__DataSrc39094(D_80081940Obj *self) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetDataSrc39094Methods();
    self->unk30 = -1;
    self->unk2C = 0;
    self->unk2E = 0;
    self->unk32 = 0;
    self->unk34 = NULL;
    self->unk38 = 1;
    self->buffer = BMemPMgrAlloc(0xB358);
    if (self->buffer != NULL) {
        self->bufferSize = 0xB358;
    }
}
/* slot +0x00C of D_80081940 (finalize) */
void DataSrc39094__Finalize(D_80081940Obj *self) {
    self->methods->releaseAlloc(self);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* slot +0x064 of D_80081940 (setFlag) */
void DataSrc39094__SetFlag(D_80081940Obj *self) {
    if (self->unk2A == 9) {
        if (self->flags & 0x80) {
            self->unk2A = 0;
            self->unk2C = 1;
            if (self->unk38 != 0) {
                self->methods->slot80();
            }
        }
    } else if (self->unk2A == 10) {
        if (self->flags & 0x80) {
            self->unk2E = 1;
            self->unk2A = 0;
        }
    }
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
}
/* slot +0x074 of D_80081940 (cancelRequests) */
void DataSrc39094__CancelRequests(D_80081940Obj *self) {
    GetActiveDataSourceMethods()->cancelRequests((Class6D430 *)self);
    self->unk2C = 0;
    self->unk2E = 0;
    self->unk2A = 0;
}
/* slot +0x078 of D_80081940: start streaming a file into the buffer */
void DataSrc39094__LoadHeader(D_80081940Obj *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->unk2A == 0) {
            self->unk2C = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->unk2A = 9;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, 0xB358);
    }
}
void DataSrc39094__ReleaseHeader(D_80081940Obj *self) {
    self->methods->freeBuffer(self);
    self->unk2C = 0;
    self->unk30 = -1;
}
/* The header at the start of D_80081940's 0xB358 buffer (local view). */
typedef struct StreamHdr {
    /* +0x00 */ u16 unk0;
    /* +0x02 */ u16 hasData;
    /* +0x04 */ u8 pad4[0xC];
    /* +0x10 */ u32 dataOffset;
    /* +0x14 */ s32 dataSize;
} StreamHdr;

/* slot +0x080 of D_80081940: load the data block the header describes */
s32 DataSrc39094__LoadDataBlock(D_80081940Obj *self) {
    s32 size;
    if (((StreamHdr *)self->buffer)->hasData == 0) {
        return 0;
    }
    if (self->unk2A != 0) {
        return 0;
    }
    self->methods->releaseAlloc();
    size = ((StreamHdr *)self->buffer)->dataSize;
    self->unk34 = BMemPMgrAlloc(size);
    if (self->unk34 == NULL) {
        return 0;
    }
    self->unk2A = 10;
    self->methods->seek(self, ((StreamHdr *)self->buffer)->dataOffset, 0);
    self->methods->read(self, self->unk34, size);
    return 1;
}
/* slot +0x084 of D_80081940 */
void DataSrc39094__ReleaseDataBlock(D_80081940Obj *self) {
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
void DataSrc39094__SetAutoLoadData(D_80081940Obj *self, s32 value) {
    self->unk38 = value;
}
void *GetDataSrc39094Methods(void) {
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
s32 func_80048D74(s32 arg) {
    u32 r = (u32)func_80048CFC(0, arg) % 7;
    s32 *table = func_80048D64();
    s32 *entry;
    s32 index;
    if (D_8008A964 != 0) {
        index = D_8008A964 - 1;
        entry = &table[index];
    } else {
        entry = &table[r];
    }
    return *entry;
}
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
Rec1C *func_80048EA0(s32 index, s32 arg1, s32 day) {
    s32 n = ((day - 1) % 40) / 10 + 1;
    s32 r = func_80048CFC(0, arg1) % n;
    return &func_80048E80(index)[r];
}
Rec1C *func_80048F60(s32 index) {
    return &func_80048E2C(index)[4];
}
Rec1C *func_80048F84(s32 index) {
    s32 unused;
    u32 r = (u32)func_80048CFC(0, unused) % 5;
    Rec1C *rec;
    if (index == 9) {
        if (r == 2) {
            r = 3;
        }
        if (D_8008A968 == 3) {
            D_8008A968 = 4;
        }
    }
    rec = func_80048F60(index);
    return &rec[D_8008A968 != 0 ? D_8008A968 - 1 : r];
}
Rec1C *func_8004903C(s32 index) {
    return &func_80048E2C(index)[9];
}
Rec1C *func_80049060(s32 index, s32 sub) {
    return &func_8004903C(index)[sub];
}
Rec1C *func_80049098(s32 index, s32 x, s32 y) {
    return func_80049060(index, x + GetStageGridDimensions(index)->columns * y);
}
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
Rec1C *func_8004913C(s32 *countOut) {
    s32 unused;
    u32 r = (u32)func_80048CFC(0, unused) % 7;
    s32 count;
    Rec1C *rec = func_80049110(&count);
    if (countOut != NULL) {
        *countOut = r + count;
    }
    return &rec[r];
}
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
Rec1C *func_80049270(s32 *countOut, s32 sub) {
    s32 count;
    Rec1C *rec = func_80049240(&count);
    if (countOut != NULL) {
        *countOut = sub + count;
    }
    return &rec[sub];
}
Rec1C *func_800492D0(s32 *countOut, s32 n) {
    Rec1C *rec = &((Rec1C *)func_80048D48(NULL))[0x23E];
    if (countOut != NULL) {
        *countOut = n * 2 + 0xE;
    }
    return &rec[n * 6];
}
/* two s16 halves passed by value in one register */
typedef struct RecPick {
    s16 group;
    s16 sub;
} RecPick;

Rec1C *func_80049334(s32 *countOut, RecPick pick) {
    s32 count;
    Rec1C *rec;
    if (pick.group >= 0) {
        rec = func_800492D0(&count, pick.group);
        if (countOut != NULL) {
            *countOut = ((u16)pick.sub < 2) ? pick.sub + count : -1;
        }
        return &rec[pick.sub];
    }
    return func_80049270(countOut, pick.sub);
}
s32 func_800493C8(s32 index) {
    return D_80086170[index];
}
Rec1C *func_800493E4(s32 *total, s32 n, s32 len) {
    s32 count;
    s32 i;
    s32 start;
    Rec1C *rec = func_800492D0(&count, n);
    len *= 2;
    *total = 0;
    start = count;
    len += start;
    for (i = start; i < len; i++) {
        *total += D_80086170[i] + 10;
    }
    *total -= 10;
    return rec;
}
