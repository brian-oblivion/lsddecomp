/*
 * code_39094 -- GAME code carved from psyq_39094 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x39094..0x39C80 (vram 0x80048894..0x80049480). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). All 38 functions matched round 82 (three
 * echo sessions); named round 82 (bravo). Two independent groups of code:
 *
 * 1. DataSrc39094 (method table D_80081940, unrenamed -- only the getter is
 *    named, per this project's convention for a Class6D430 subclass with no
 *    established in-game role): a file-streaming state machine that loads a
 *    header block into its own 0xB358 buffer (state 9, DataSrc39094__LoadHeader),
 *    then, once flagged complete (DataSrc39094__SetFlag), an optional data
 *    block the header describes into a second allocation (state 10,
 *    DataSrc39094__LoadDataBlock/dataBuffer), auto-triggered unless disabled
 *    via DataSrc39094__SetAutoLoadData.
 * 2. Free functions over gRecordTable, a table of 0x230+ fixed 0x1C-byte
 *    records (Rec1C): random-or-forced pickers (SeedAndRandom,
 *    SetPickOverrides/gForcedWeeklyGroup/gForcedVariant), record-group
 *    accessors indexed by gRecordIndexTable and, for GetGridRecordXY, by
 *    StageGrid.h's cell columns, and a family of "stream channel" lookups
 *    (GetIntroStreamName, PickWeeklyStreamChannel, GetStreamChannelInit,
 *    ResolveCinematicChannel, GetGraphRoomStreamChannel) whose shapes match
 *    their exact call sites in code_1677c.c one for one. The records' own
 *    fields and the channels' in-game meaning are not established.
 */
#include "common.h"
#include "Class6D430.h"
#include "StageGrid.h"

typedef struct DataSrc39094 DataSrc39094;

/* D_80081940's own method table (local view): the Class6D430 interface plus
 * this class's extra slots. */
typedef struct DataSrc39094Methods {
    CLASS6D430_SLOTS(DataSrc39094, (DataSrc39094 *self));
    /* +0x07C */ void *slot7C;
    /* +0x080 */ s32 (*loadDataBlock)();  /* DataSrc39094__LoadDataBlock(self); unprototyped: DataSrc39094__SetFlag calls it with no argument */
    /* +0x084 */ void (*releaseAlloc)();  /* DataSrc39094__ReleaseDataBlock(self); unprototyped: DataSrc39094__LoadDataBlock calls it with no argument */
} DataSrc39094Methods;

/* The D_80081940 object: a Class6D430 data source with its own fields from
 * +0x2C (local view; only this unit's methods read them). */
struct DataSrc39094 {
    CLASS6D430_FIELDS(DataSrc39094Methods);
    /* +0x02C */ u16 headerReady;
    /* +0x02E */ u16 dataReady;
    /* +0x030 */ s16 unk30;
    /* +0x032 */ u16 unk32;
    /* +0x034 */ void *dataBuffer;   /* BMemPMgr allocation, released by slot +0x084 */
    /* +0x038 */ s32 autoLoadData;
};

/* One 0x1C-byte record of the table GetRecordTable returns (gRecordTable,
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
DataSrc39094 *New_DataSrc39094(void) {
    DataSrc39094 *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        ((Class6D430Methods *)GetDataSrc39094Methods())->ctor((Class6D430 *)obj);
        return obj;
    }
    return NULL;
}
/* slot +0x008 of D_80081940 (ctor) */
void DataSrc39094__DataSrc39094(DataSrc39094 *self) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetDataSrc39094Methods();
    self->unk30 = -1;
    self->headerReady = 0;
    self->dataReady = 0;
    self->unk32 = 0;
    self->dataBuffer = NULL;
    self->autoLoadData = 1;
    self->buffer = BMemPMgrAlloc(0xB358);
    if (self->buffer != NULL) {
        self->bufferSize = 0xB358;
    }
}
/* slot +0x00C of D_80081940 (finalize) */
void DataSrc39094__Finalize(DataSrc39094 *self) {
    self->methods->releaseAlloc(self);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* slot +0x064 of D_80081940 (setFlag) */
void DataSrc39094__SetFlag(DataSrc39094 *self) {
    if (self->unk2A == 9) {
        if (self->flags & 0x80) {
            self->unk2A = 0;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                self->methods->loadDataBlock();
            }
        }
    } else if (self->unk2A == 10) {
        if (self->flags & 0x80) {
            self->dataReady = 1;
            self->unk2A = 0;
        }
    }
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
}
/* slot +0x074 of D_80081940 (cancelRequests) */
void DataSrc39094__CancelRequests(DataSrc39094 *self) {
    GetActiveDataSourceMethods()->cancelRequests((Class6D430 *)self);
    self->headerReady = 0;
    self->dataReady = 0;
    self->unk2A = 0;
}
/* slot +0x078 of D_80081940: start streaming a file into the buffer */
void DataSrc39094__LoadHeader(DataSrc39094 *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->unk2A == 0) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->unk2A = 9;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, 0xB358);
    }
}
void DataSrc39094__ReleaseHeader(DataSrc39094 *self) {
    self->methods->freeBuffer(self);
    self->headerReady = 0;
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
s32 DataSrc39094__LoadDataBlock(DataSrc39094 *self) {
    s32 size;
    if (((StreamHdr *)self->buffer)->hasData == 0) {
        return 0;
    }
    if (self->unk2A != 0) {
        return 0;
    }
    self->methods->releaseAlloc();
    size = ((StreamHdr *)self->buffer)->dataSize;
    self->dataBuffer = BMemPMgrAlloc(size);
    if (self->dataBuffer == NULL) {
        return 0;
    }
    self->unk2A = 10;
    self->methods->seek(self, ((StreamHdr *)self->buffer)->dataOffset, 0);
    self->methods->read(self, self->dataBuffer, size);
    return 1;
}
/* slot +0x084 of D_80081940 */
void DataSrc39094__ReleaseDataBlock(DataSrc39094 *self) {
    self->dataReady = 0;
    if (self->dataBuffer != NULL) {
        self->dataBuffer = BMemPMgrFree(self->dataBuffer);
    }
}
extern u8 D_80081940[];   /* method table, 34 slots */
extern s32 D_8008A960;
extern s32 gForcedWeeklyGroup;
extern s32 gForcedVariant;
extern u8 gWeeklyGroupTable[];
extern u8 gRecordTable[];
extern char *gSoundEffectDirPtr;  /* -> "SND\\SE" */
extern const char sAsmkStreamPath[];
extern s16 gStreamTypeToGroupTable[];
extern s16 gRecordIndexTable[];

/* slot +0x088 of D_80081940 */
void DataSrc39094__SetAutoLoadData(DataSrc39094 *self, s32 value) {
    self->autoLoadData = value;
}
void *GetDataSrc39094Methods(void) {
    return D_80081940;
}
s32 func_80048CF0(void) {
    return D_8008A960;
}
s32 SeedAndRandom(s32 seed, s32 unused) {
    if (seed != 0) {
        srand(seed);
    }
    return rand();
}
void SetPickOverrides(s32 a, s32 b) {
    if (a >= 0) {
        gForcedWeeklyGroup = a;
    }
    if (b >= 0) {
        gForcedVariant = b;
    }
}
void *GetRecordTable(s32 *out) {
    if (out != NULL) {
        *out = 0x230;
    }
    return gRecordTable;
}
void *GetWeeklyGroupTable(void) {
    return gWeeklyGroupTable;
}
s32 PickWeeklyGroup(s32 arg) {
    u32 r = (u32)SeedAndRandom(0, arg) % 7;
    s32 *table = GetWeeklyGroupTable();
    s32 *entry;
    s32 index;
    if (gForcedWeeklyGroup != 0) {
        index = gForcedWeeklyGroup - 1;
        entry = &table[index];
    } else {
        entry = &table[r];
    }
    return *entry;
}
char **GetSoundEffectDirRef(void) {
    return &gSoundEffectDirPtr;
}
char *GetSoundEffectDir(void) {
    return *GetSoundEffectDirRef();
}
Rec1C *GetRecordGroup(s32 index) {
    return &((Rec1C *)GetRecordTable(NULL))[gRecordIndexTable[index]];
}

Rec1C *GetRecordGroupAlias(s32 index) {
    return GetRecordGroup(index);
}
Rec1C *PickDailyVariant(s32 index, s32 arg1, s32 day) {
    s32 n = ((day - 1) % 40) / 10 + 1;
    s32 r = SeedAndRandom(0, arg1) % n;
    return &GetRecordGroupAlias(index)[r];
}
Rec1C *GetVariantBlock(s32 index) {
    return &GetRecordGroup(index)[4];
}
Rec1C *PickVariant(s32 index) {
    s32 unused;
    u32 r = (u32)SeedAndRandom(0, unused) % 5;
    Rec1C *rec;
    if (index == 9) {
        if (r == 2) {
            r = 3;
        }
        if (gForcedVariant == 3) {
            gForcedVariant = 4;
        }
    }
    rec = GetVariantBlock(index);
    return &rec[gForcedVariant != 0 ? gForcedVariant - 1 : r];
}
Rec1C *GetGridRecordBase(s32 index) {
    return &GetRecordGroup(index)[9];
}
Rec1C *GetGridRecordAt(s32 index, s32 sub) {
    return &GetGridRecordBase(index)[sub];
}
Rec1C *GetGridRecordXY(s32 index, s32 x, s32 y) {
    return GetGridRecordAt(index, x + GetStageGridDimensions(index)->columns * y);
}
const char *GetIntroStreamName(s32 *typeCodeOut) {
    if (typeCodeOut != NULL) {
        *typeCodeOut = 0x31;
    }
    return sAsmkStreamPath;
}
Rec1C *GetWeeklyStreamPool(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 0;
    }
    return &((Rec1C *)GetRecordTable(NULL))[0x230];
}
Rec1C *PickWeeklyStreamChannel(s32 *countOut) {
    s32 unused;
    u32 r = (u32)SeedAndRandom(0, unused) % 7;
    s32 count;
    Rec1C *rec = GetWeeklyStreamPool(&count);
    if (countOut != NULL) {
        *countOut = r + count;
    }
    return &rec[r];
}
Rec1C *GetStreamPool2(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 7;
    }
    return &((Rec1C *)GetRecordTable(NULL))[0x237];
}
Rec1C *GetStreamChannelInit(s32 *countOut) {
    s32 count;
    Rec1C *rec = GetStreamPool2(&count);
    if (countOut != NULL) {
        *countOut = count;
    }
    return rec;
}
Rec1C *GetStreamPool3(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 8;
    }
    return &((Rec1C *)GetRecordTable(NULL))[0x238];
}
Rec1C *GetStreamPool3Channel(s32 *countOut, s32 sub) {
    s32 count;
    Rec1C *rec = GetStreamPool3(&count);
    if (countOut != NULL) {
        *countOut = sub + count;
    }
    return &rec[sub];
}
Rec1C *GetCinematicBank(s32 *countOut, s32 n) {
    Rec1C *rec = &((Rec1C *)GetRecordTable(NULL))[0x23E];
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

Rec1C *ResolveCinematicChannel(s32 *countOut, RecPick pick) {
    s32 count;
    Rec1C *rec;
    if (pick.group >= 0) {
        rec = GetCinematicBank(&count, pick.group);
        if (countOut != NULL) {
            *countOut = ((u16)pick.sub < 2) ? pick.sub + count : -1;
        }
        return &rec[pick.sub];
    }
    return GetStreamPool3Channel(countOut, pick.sub);
}
s32 GetStreamGroupForType(s32 index) {
    return gStreamTypeToGroupTable[index];
}
Rec1C *GetGraphRoomStreamChannel(s32 *total, s32 n, s32 len) {
    s32 count;
    s32 i;
    s32 start;
    Rec1C *rec = GetCinematicBank(&count, n);
    len *= 2;
    *total = 0;
    start = count;
    len += start;
    for (i = start; i < len; i++) {
        *total += gStreamTypeToGroupTable[i] + 10;
    }
    *total -= 10;
    return rec;
}
