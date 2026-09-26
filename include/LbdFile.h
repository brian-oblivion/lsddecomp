#ifndef LBDFILE_H
#define LBDFILE_H

#include "FileResource.h"

/*
 * LbdFile -- a FileResource data source (class id 0x903, method table
 * gLbdFileMethods at 0x80081940) that streams one file in two stages: a
 * header block into its own 0xB358-byte `buffer`, then, if the header says
 * there is one, a data block into a second allocation. Methods in
 * src/code_39094.c. No classes derive from it. Named for its table address:
 * the old local view was `DataSrc39094`, and without its unit suffix that is
 * what every FileResource subclass is.
 *
 * PARENT BY CTOR CHAIN: LbdFile__LbdFile's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize, setFlag and
 * cancelRequests forward to the active driver's, as Class6D940, TimBlockSrc
 * and VabStreamObj do. GetLbdFileMethods is gFileResourceMethods's
 * gDataSourceClientGetters entry at +0x0A0, so SetActiveDataSource rebinds
 * this table's interface slots (+0x040..+0x058, +0x068..+0x074; +0x058 is
 * NULL in the static table).
 *
 * Loading. loadHeader (+0x078) closes, opens `name` and reads 0xB358 bytes
 * into `buffer`, with the load state (`unk2A`) at 9. The driver calls setFlag
 * (+0x064, LbdFile__AdvanceLoadState) when a request completes; with
 * CD_FLAG_READ_DONE (0x80, src/code_179d8_s.c) in `flags`, state 9 sets
 * `headerReady` and, unless `autoLoadData` is 0, runs loadDataBlock
 * (+0x080), which seeks to the header's dataOffset and reads dataSize bytes
 * into `dataBuffer` in state 10; state 10 then sets `dataReady`.
 *
 * Its one user is the grid manager (Class866E8): Class866E8__Class866E8 makes
 * one per grid element with New_LbdFile, sets freeGuard = (buffer != NULL)
 * and ownerKey = the element index, and passes its own arg2 to
 * setAutoLoadData. Class866E8__ApplyRateEntries stores the setup entry's
 * rate in `ownerRate` and calls loadHeader with the entry's name;
 * Class866E8__OnNotifyTag1 sets headerReady to 2 once it has consumed the
 * header and clears dataReady; Class866E8__LoadElementResources points the
 * element's Class6D940 and LinkResource into the loaded header block
 * (LbdFileHeader); ObjM__CheckAuxTrigger passes `dataBuffer` on.
 *
 * Three calls do not match their occupant's parameter list and cast through
 * a typedef below (no code; FINISHING-PLAN track 4 step 6):
 *  - +0x078 is FileResource's `void *slot78`; Class866E8__ApplyRateEntries
 *    calls it as LbdFileLoadHeaderFn.
 *  - LbdFile__AdvanceLoadState calls +0x080 and LbdFile__LoadDataBlock
 *    calls +0x084 with NO argument: retail sets no $a0 for either jalr ($a0
 *    still holds self by accident). LbdFileLoadDataBlockNoArgFn and
 *    LbdFileReleaseDataBlockNoArgFn.
 *  - Class866E8__ResetElementCells calls +0x07C with a second argument, the
 *    element, which LbdFile__ReleaseHeader never reads.
 *    LbdFileReleaseHeaderElemFn.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x903 (`typeviews.py --tree`).
 */

typedef struct LbdFile LbdFile;
typedef struct LbdFileMethods LbdFileMethods;

/* The start of the header block loadHeader reads into `buffer`. */
typedef struct LbdFileHeader {
    /* +0x00 */ u16 unk0;
    /* +0x02 */ u16 hasData; /* zero: loadDataBlock returns 0 */
    /* +0x04 */ s32 gridOffset; /* Class866E8__LoadElementResources: the element's Class6D940 buffer is header + gridOffset */
    /* +0x08 */ s32 gridSize; /* ... and its LinkResource's data header + gridOffset + gridSize */
    /* +0x0C */ u8 padC[0x10 - 0xC];
    /* +0x10 */ u32 dataOffset; /* file offset of the data block (seek mode 0) */
    /* +0x14 */ s32 dataSize;   /* the data block's size: dataBuffer's allocation */
} LbdFileHeader;

struct LbdFileMethods {
    FILERESOURCE_SLOTS(LbdFile, (LbdFile * self));
    /* +0x078 is FileResource's slot78; this table's occupant is
     * LbdFile__LoadHeader (LbdFileLoadHeaderFn). */
    /* +0x07C */ void (*releaseHeader)(LbdFile *self); /* LbdFile__ReleaseHeader */
    /* +0x080 */ s32 (*loadDataBlock)(LbdFile *self); /* LbdFile__LoadDataBlock: 1 when a read was started */
    /* +0x084 */ void (*releaseDataBlock)(LbdFile *self); /* LbdFile__ReleaseDataBlock */
    /* +0x088 */ void (*setAutoLoadData)(LbdFile *self, s32 value); /* LbdFile__SetAutoLoadData */
}; /* 34 slots, 0x8C bytes */

struct LbdFile {
    FILERESOURCE_FIELDS(LbdFileMethods); /* buffer: the 0xB358 header block (LbdFileHeader); unk2A is the load state: 0 idle, 9 header, 10 data block */
    /* +0x02C */ s16 headerReady; /* 1 when the header is read; Class866E8__OnNotifyTag1 sets 2 once consumed */
    /* +0x02E */ s16 dataReady; /* 1 when the data block is read; cleared by Class866E8__OnNotifyTag1 */
    /* +0x030 */ s16 ownerRate; /* the owner's: Class866E8__ApplyRateEntries' setup-entry rate; -1 from the ctor and ReleaseHeader */
    /* +0x032 */ s16 ownerKey;  /* the owner's: Class866E8's element index; zeroed by the ctor */
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
