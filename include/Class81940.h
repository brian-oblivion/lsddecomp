#ifndef CLASS81940_H
#define CLASS81940_H

#include "Class6D430.h"

/*
 * Class81940 -- a Class6D430 data source (class id 0x903, method table
 * gClass81940Methods at 0x80081940) that streams one file in two stages: a
 * header block into its own 0xB358-byte `buffer`, then, if the header says
 * there is one, a data block into a second allocation. Methods in
 * src/code_39094.c. No classes derive from it. Named for its table address:
 * the old local view was `DataSrc39094`, and without its unit suffix that is
 * what every Class6D430 subclass is.
 *
 * PARENT BY CTOR CHAIN: Class81940__Class81940's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize, setFlag and
 * cancelRequests forward to the active driver's, as Class6D940, TimBlockSrc
 * and VabStreamObj do. GetClass81940Methods is D_8006D430's
 * gDataSourceClientGetters entry at +0x0A0, so SetActiveDataSource rebinds
 * this table's interface slots (+0x040..+0x058, +0x068..+0x074; +0x058 is
 * NULL in the static table).
 *
 * Loading. loadHeader (+0x078) closes, opens `name` and reads 0xB358 bytes
 * into `buffer`, with the load state (`unk2A`) at 9. The driver calls setFlag
 * (+0x064, Class81940__AdvanceLoadState) when a request completes; with
 * CD_FLAG_READ_DONE (0x80, src/code_179d8_s.c) in `flags`, state 9 sets
 * `headerReady` and, unless `autoLoadData` is 0, runs loadDataBlock
 * (+0x080), which seeks to the header's dataOffset and reads dataSize bytes
 * into `dataBuffer` in state 10; state 10 then sets `dataReady`.
 *
 * Its one user is the grid manager (Class866E8): Class866E8__Class866E8 makes
 * one per grid element with New_Class81940, sets freeGuard = (buffer != NULL)
 * and ownerKey = the element index, and passes its own arg2 to
 * setAutoLoadData. Class866E8__ApplyRateEntries stores the setup entry's
 * rate in `ownerRate` and calls loadHeader with the entry's name;
 * Class866E8__OnNotifyTag1 sets headerReady to 2 once it has consumed the
 * header and clears dataReady; Class866E8__LoadElementResources points the
 * element's Class6D940 and LinkResource into the loaded header block
 * (Class81940Header); ObjM__CheckAuxTrigger passes `dataBuffer` on.
 *
 * Three calls do not match their occupant's parameter list and cast through
 * a typedef below (no code; FINISHING-PLAN track 4 step 6):
 *  - +0x078 is Class6D430's `void *slot78`; Class866E8__ApplyRateEntries
 *    calls it as Class81940LoadHeaderFn.
 *  - Class81940__AdvanceLoadState calls +0x080 and Class81940__LoadDataBlock
 *    calls +0x084 with NO argument: retail sets no $a0 for either jalr ($a0
 *    still holds self by accident). Class81940LoadDataBlockNoArgFn and
 *    Class81940ReleaseDataBlockNoArgFn.
 *  - Class866E8__ResetElementCells calls +0x07C with a second argument, the
 *    element, which Class81940__ReleaseHeader never reads.
 *    Class81940ReleaseHeaderElemFn.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x903 (`typeviews.py --tree`).
 */

typedef struct Class81940 Class81940;
typedef struct Class81940Methods Class81940Methods;

/* The start of the header block loadHeader reads into `buffer`. */
typedef struct Class81940Header {
    /* +0x00 */ u16 unk0;
    /* +0x02 */ u16 hasData; /* zero: loadDataBlock returns 0 */
    /* +0x04 */ s32 gridOffset; /* Class866E8__LoadElementResources: the element's Class6D940 buffer is header + gridOffset */
    /* +0x08 */ s32 gridSize; /* ... and its LinkResource's data header + gridOffset + gridSize */
    /* +0x0C */ u8 padC[0x10 - 0xC];
    /* +0x10 */ u32 dataOffset; /* file offset of the data block (seek mode 0) */
    /* +0x14 */ s32 dataSize;   /* the data block's size: dataBuffer's allocation */
} Class81940Header;

struct Class81940Methods {
    CLASS6D430_SLOTS(Class81940, (Class81940 * self));
    /* +0x078 is Class6D430's slot78; this table's occupant is
     * Class81940__LoadHeader (Class81940LoadHeaderFn). */
    /* +0x07C */ void (*releaseHeader)(Class81940 *self); /* Class81940__ReleaseHeader */
    /* +0x080 */ s32 (*loadDataBlock)(Class81940 *self); /* Class81940__LoadDataBlock: 1 when a read was started */
    /* +0x084 */ void (*releaseDataBlock)(Class81940 *self); /* Class81940__ReleaseDataBlock */
    /* +0x088 */ void (*setAutoLoadData)(Class81940 *self, s32 value); /* Class81940__SetAutoLoadData */
}; /* 34 slots, 0x8C bytes */

struct Class81940 {
    CLASS6D430_FIELDS(Class81940Methods); /* buffer: the 0xB358 header block (Class81940Header); unk2A is the load state: 0 idle, 9 header, 10 data block */
    /* +0x02C */ s16 headerReady; /* 1 when the header is read; Class866E8__OnNotifyTag1 sets 2 once consumed */
    /* +0x02E */ s16 dataReady; /* 1 when the data block is read; cleared by Class866E8__OnNotifyTag1 */
    /* +0x030 */ s16 ownerRate; /* the owner's: Class866E8__ApplyRateEntries' setup-entry rate; -1 from the ctor and ReleaseHeader */
    /* +0x032 */ s16 ownerKey;  /* the owner's: Class866E8's element index; zeroed by the ctor */
    /* +0x034 */ void *dataBuffer; /* the data block, BMemPMgrAlloc(dataSize); freed by ReleaseDataBlock */
    /* +0x038 */ s32 autoLoadData; /* nonzero (the ctor's 1): the header's completion starts loadDataBlock */
}; /* 0x3C bytes: New_Class81940 */

typedef void (*Class81940LoadHeaderFn)(Class81940 *self, char *name);
typedef s32 (*Class81940LoadDataBlockNoArgFn)(void);
typedef void (*Class81940ReleaseDataBlockNoArgFn)(void);
typedef void (*Class81940ReleaseHeaderElemFn)(Class81940 *self, void *elem);

extern Class81940Methods gClass81940Methods;
extern Class81940Methods *GetClass81940Methods(void);

Class81940 *New_Class81940(void);
void Class81940__Class81940(Class81940 *self);
void Class81940__Finalize(Class81940 *self);
void Class81940__AdvanceLoadState(Class81940 *self);
void Class81940__CancelRequests(Class81940 *self);
void Class81940__LoadHeader(Class81940 *self, char *name);
void Class81940__ReleaseHeader(Class81940 *self);
s32 Class81940__LoadDataBlock(Class81940 *self);
void Class81940__ReleaseDataBlock(Class81940 *self);
void Class81940__SetAutoLoadData(Class81940 *self, s32 value);

#endif
