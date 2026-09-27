/*
 * code_39094 -- two independent groups of game code:
 *
 * 1. LbdFile (include/LbdFile.h): the loader for one stage map chunk,
 *    STGnn\Mnnn.LBD. LbdFile__LoadHeader streams the file's first 0xB358
 *    bytes into the object's fixed buffer; when that read completes
 *    (LbdFile__AdvanceLoadState) LbdFile__LoadDataBlock reads the optional
 *    data block the header locates into a second allocation, unless
 *    LbdFile__SetAutoLoadData turned that off.
 * 2. Free functions over gRecordTable, a table of 0x230+ fixed 0x1C-byte
 *    records (Rec1C) whose first bytes are a file path (the sound banks
 *    SND\*.VH/VB; then per stage its TEXx.TIX, BGx.SEQ and Mnnn.LBD files;
 *    then the FILM .STR and IMG .TIM files): random-or-forced
 *    pickers (SeedAndRandom, SetPickOverrides/gForcedWeeklyGroup/
 *    gForcedVariant), per-stage record-group accessors indexed by
 *    gRecordIndexTable and, for GetGridRecordXY, by StageGrid.h's cell
 *    columns, and a family of "stream channel" lookups (GetIntroStreamName,
 *    PickWeeklyStreamChannel, GetStreamChannelInit, ResolveCinematicChannel,
 *    GetGraphRoomStreamChannel) whose shapes match their call sites in
 *    code_1677c.c one for one. The records' other fields and the channels'
 *    in-game meaning are not established.
 */
#include "common.h"
#include "LbdFile.h"
#include "StageGrid.h"

/* One 0x1C-byte record of the table GetRecordTable returns (gRecordTable,
 * 0x230 records); only its size is known here. */
typedef struct Rec1C {
    u8 data[0x1C];
} Rec1C;

extern int rand(void);
extern void srand(unsigned int seed);
extern void *BMemPMgrFree(void *ptr);
extern FileResourceMethods *GetActiveDataSourceMethods(void);
extern void *BMemPMgrAlloc(s32 size);

/* allocator: new LbdFile object */
LbdFile *New_LbdFile(void) {
    LbdFile *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        GetLbdFileMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* slot +0x008 of gLbdFileMethods (ctor) */
void LbdFile__LbdFile(LbdFile *self) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLbdFileMethods();
    self->chunkIndex = -1;
    self->headerReady = 0;
    self->dataReady = 0;
    self->elemKey = 0;
    self->dataBuffer = NULL;
    self->autoLoadData = 1;
    self->buffer = BMemPMgrAlloc(0xB358);
    if (self->buffer != NULL) {
        self->bufferSize = 0xB358;
    }
}

/* slot +0x00C of gLbdFileMethods (finalize) */
void LbdFile__Finalize(LbdFile *self) {
    self->methods->releaseDataBlock(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* slot +0x064 of gLbdFileMethods (setFlag) */
void LbdFile__AdvanceLoadState(LbdFile *self) {
    if (self->loadState == 9) {
        if (self->flags & 0x80) {
            self->loadState = 0;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                ((LbdFileLoadDataBlockNoArgFn)self->methods->loadDataBlock)(); /* retail passes no argument */
            }
        }
    } else if (self->loadState == 10) {
        if (self->flags & 0x80) {
            self->dataReady = 1;
            self->loadState = 0;
        }
    }
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
}

/* slot +0x074 of gLbdFileMethods (cancelRequests) */
void LbdFile__CancelRequests(LbdFile *self) {
    GetActiveDataSourceMethods()->cancelRequests((FileResource *)self);
    self->headerReady = 0;
    self->dataReady = 0;
    self->loadState = 0;
}

/* slot +0x078 of gLbdFileMethods: start streaming a file into the buffer */
void LbdFile__LoadHeader(LbdFile *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->loadState == 0) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->loadState = 9;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, 0xB358);
    }
}

void LbdFile__ReleaseHeader(LbdFile *self) {
    self->methods->freeBuffer(self);
    self->headerReady = 0;
    self->chunkIndex = -1;
}

/* slot +0x080 of gLbdFileMethods: load the data block the header describes */
s32 LbdFile__LoadDataBlock(LbdFile *self) {
    s32 size;
    if (((LbdFileHeader *)self->buffer)->hasData == 0) {
        return 0;
    }
    if (self->loadState != 0) {
        return 0;
    }
    ((LbdFileReleaseDataBlockNoArgFn)self->methods->releaseDataBlock)(); /* retail passes no argument */
    size = ((LbdFileHeader *)self->buffer)->dataSize;
    self->dataBuffer = BMemPMgrAlloc(size);
    if (self->dataBuffer == NULL) {
        return 0;
    }
    self->loadState = 10;
    self->methods->seek(self, ((LbdFileHeader *)self->buffer)->dataOffset, 0);
    self->methods->read(self, self->dataBuffer, size);
    return 1;
}

/* slot +0x084 of gLbdFileMethods */
void LbdFile__ReleaseDataBlock(LbdFile *self) {
    self->dataReady = 0;
    if (self->dataBuffer != NULL) {
        self->dataBuffer = BMemPMgrFree(self->dataBuffer);
    }
}

extern s32 D_8008A960;
extern s32 gForcedWeeklyGroup;
extern s32 gForcedVariant;
extern u8 gWeeklyGroupTable[];
extern u8 gRecordTable[];
extern char *gSoundEffectDirPtr; /* -> "SND\\SE" */
extern const char sAsmkStreamPath[];
extern s16 gStreamTypeToGroupTable[];
extern s16 gRecordIndexTable[];

/* slot +0x088 of gLbdFileMethods */
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value) {
    self->autoLoadData = value;
}

LbdFileMethods *GetLbdFileMethods(void) {
    return &gLbdFileMethods;
}

s32 GetDefaultDataDirectory(void) {
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

Rec1C *PickVariant(s32 index, s32 arg1) {
    u32 r = (u32)SeedAndRandom(0, arg1) % 5; /* arg1 only forwarded, like PickDailyVariant's */
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

Rec1C *PickWeeklyStreamChannel(s32 *countOut, s32 arg1) {
    u32 r = (u32)SeedAndRandom(0, arg1) % 7; /* arg1 only forwarded, like PickDailyVariant's */
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
