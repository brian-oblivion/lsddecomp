#ifndef LBDFILE_H
#define LBDFILE_H

#include "FileResource.h"

/*
 * LbdFile -- one of the stage's map-chunk files, STGnn\Mnnn.LBD, loaded for
 * one element of the grid manager (class id 0x903, method table
 * gLbdFileMethods, parent FileResource; methods in src/code_39094.c; no
 * subclasses). The files it is handed are the gRecordTable records
 * GetGridRecordAt(stage, chunk) returns: StageMap__ComputeChunkLoadEntry takes
 * each entry's name from the grid's callback, ObjM__OnRegistrantEvent, whose
 * tail call leaves that record in $v0, and the record's first bytes are the
 * path ("STG00\M000.LBD" is record 9 of stage 0's group).
 *
 * The file is streamed in two stages. loadHeader (+0x078, FileResource's
 * processBuffer slot) opens the file and reads its first 0xB358 bytes into
 * the ctor's fixed `buffer` (loadState 9); that block starts with an
 * LbdFileHeader. When the CD driver reports the read done (setFlag, +0x064,
 * LbdFile__AdvanceLoadState, bit 0x80 of `flags`), `headerReady` is set and,
 * unless setAutoLoadData turned it off, loadDataBlock (+0x080) reads the
 * optional data block the header locates into a second allocation,
 * `dataBuffer` (loadState 10), setting `dataReady` when that read completes.
 * releaseHeader and releaseDataBlock free the two halves.
 *
 * Like every FileResource client it runs on the active driver: the ctor,
 * finalize, setFlag and cancelRequests chain to GetActiveDataSourceMethods()'s
 * first, and GetLbdFileMethods is in gDataSourceClientGetters, so
 * SetActiveDataSource rebinds this table's file-I/O slots.
 *
 * Its one user is the grid manager (StageMap), which makes one per grid
 * element (New_LbdFile in StageMap__StageMap, with freeGuard set so
 * freeBuffer keeps the fixed buffer), loads each element's chunk through it
 * (StageMap__ApplyChunkLoads), links the header block's placements and
 * models into the element's cells (StageMap__PopulateSlotCells), then
 * marks the header consumed (headerReady 2). ObjM__CheckAuxTrigger hands
 * the data block to TryDreamAuxTrigger and releases it when that returns 0.
 *
 * Three callers do not use their occupant's parameter list and cast through
 * a typedef below (no code):
 *  - +0x078 is FileResource's untyped processBuffer; ApplyRateEntries calls
 *    it as LbdFileLoadHeaderFn.
 *  - AdvanceLoadState calls +0x080 and LoadDataBlock calls +0x084 with no
 *    argument (retail sets no $a0; it still holds self):
 *    LbdFileLoadDataBlockNoArgFn and LbdFileReleaseDataBlockNoArgFn.
 *  - StageMap__ClearSlotCells passes +0x07C a second argument, the
 *    element, that LbdFile__ReleaseHeader never reads:
 *    LbdFileReleaseHeaderElemFn.
 */

typedef struct LbdFile LbdFile;
typedef struct LbdFileMethods LbdFileMethods;

/* The start of the header block loadHeader reads into `buffer`. */
typedef struct LbdFileHeader {
    /* +0x00 */ u8 pad0[0x2];
    /* +0x02 */ u16 hasData; /* zero: loadDataBlock returns 0 */
    /* +0x04 */ s32 placementsOffset; /* StageMap__PopulateSlotCells: the element's PlacementGrid buffer is header + placementsOffset */
    /* +0x08 */ s32 placementsSize; /* ... and its LinkResource's data header + placementsOffset + placementsSize */
    /* +0x0C */ u8 padC[0x10 - 0xC];
    /* +0x10 */ u32 dataOffset; /* file offset of the data block (seek mode 0) */
    /* +0x14 */ s32 dataSize;   /* the data block's size: dataBuffer's allocation */
} LbdFileHeader;

struct LbdFileMethods {
    FILERESOURCE_SLOTS(LbdFile, (LbdFile * self));
    /* +0x078 is FileResource's processBuffer; this table's occupant is
     * LbdFile__LoadHeader (LbdFileLoadHeaderFn). */
    /* +0x07C */ void (*releaseHeader)(LbdFile *self); /* LbdFile__ReleaseHeader */
    /* +0x080 */ s32 (*loadDataBlock)(LbdFile *self); /* LbdFile__LoadDataBlock: 1 when a read was started */
    /* +0x084 */ void (*releaseDataBlock)(LbdFile *self);           /* LbdFile__ReleaseDataBlock */
    /* +0x088 */ void (*setAutoLoadData)(LbdFile *self, s32 value); /* LbdFile__SetAutoLoadData */
}; /* 34 slots, 0x8C bytes */

struct LbdFile {
    FILERESOURCE_FIELDS(LbdFileMethods); /* buffer: the 0xB358 header block (LbdFileHeader); loadState: 0 idle, 9 header, 10 data block */
    /* +0x02C */ s16 headerReady; /* 1 when the header is read; StageMap__OnNotifyTag1 sets 2 once consumed */
    /* +0x02E */ s16 dataReady; /* 1 when the data block is read; cleared by StageMap__OnNotifyTag1 */
    /* +0x030 */ s16 chunkIndex; /* the loaded chunk's record index in its stage (ApplyRateEntries); -1 when none (ctor, ReleaseHeader) */
    /* +0x032 */ s16 elemKey; /* the owner's element key: StageMap's ctor (the index) and BuildRateEntries; zeroed by the ctor */
    /* +0x034 */ void *dataBuffer; /* the data block, BMemPMgrAlloc(dataSize); freed by ReleaseDataBlock */
    /* +0x038 */ s32 autoLoadData; /* nonzero (the ctor's 1): the header's completion starts loadDataBlock */
}; /* 0x3C bytes: New_LbdFile */

typedef void (*LbdFileLoadHeaderFn)(LbdFile *self, char *name);
typedef s32 (*LbdFileLoadDataBlockNoArgFn)(void);
typedef void (*LbdFileReleaseDataBlockNoArgFn)(void);
typedef void (*LbdFileReleaseHeaderElemFn)(LbdFile *self, void *elem);

extern LbdFileMethods gLbdFileMethods;
extern LbdFileMethods *GetLbdFileMethods(void);

LbdFile *New_LbdFile(void);
void LbdFile__LbdFile(LbdFile *self);
void LbdFile__Finalize(LbdFile *self);
void LbdFile__AdvanceLoadState(LbdFile *self);
void LbdFile__CancelRequests(LbdFile *self);
void LbdFile__LoadHeader(LbdFile *self, char *name);
void LbdFile__ReleaseHeader(LbdFile *self);
s32 LbdFile__LoadDataBlock(LbdFile *self);
void LbdFile__ReleaseDataBlock(LbdFile *self);
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value);

#endif
