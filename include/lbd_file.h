#ifndef LBD_FILE_H
#define LBD_FILE_H

#include "file_resource.h"

/**
 * @file lbd_file.h
 * @brief LbdFile, the loader for one stage map chunk (STGnn\\Mnnn.LBD), and
 * the header block it reads.
 */

typedef struct LbdFile LbdFile;
typedef struct LbdFileMethods LbdFileMethods;

/* `headerReady` once StageMap__OnDrawSystemEvent has linked the header's
 * placements into the slot's cells (1 is "read, not yet consumed"). */
#define LBDFILE_HEADER_CONSUMED 2

/** @brief LbdFile's steps in FileResource's loadState. */
enum LbdFileLoadState {
    LBDFILE_LOAD_IDLE = 0,   /**< Nothing pending. */
    LBDFILE_LOAD_HEADER = 9, /**< loadHeader's read into `buffer` is pending. */
    LBDFILE_LOAD_DATA = 10   /**< loadDataBlock's read into `dataBuffer` is pending. */
};

/* The header block: loadHeader's read size and the ctor's `buffer`
 * allocation (0xB358). */
#define LBDFILE_HEADER_BLOCK_SIZE 45912

/** @brief The start of the header block loadHeader reads into `buffer`. */
typedef struct LbdFileHeader {
    /* +0x00 */ u8 pad0[0x2];
    /* +0x02 */ u16 hasData; /**< Zero: there is no data block, and loadDataBlock returns 0. */
    /* +0x04 */ s32 placementsOffset; /**< The placements section's offset: the element's PlacementGrid buffer is header + placementsOffset. */
    /* +0x08 */ s32 placementsSize; /**< Its size: the models the LinkResource covers start at placementsOffset + placementsSize. */
    /* +0x0C */ u8 padC[0x10 - 0xC];
    /* +0x10 */ u32 dataOffset; /**< File offset of the data block. */
    /* +0x14 */ s32 dataSize;   /**< The data block's size, dataBuffer's allocation. */
} LbdFileHeader;

/**
 * @brief LbdFile's method table: FileResource's slots, then four of its own.
 * +0x078 (processBuffer) is LbdFile__LoadHeader, called through
 * LbdFileLoadHeaderFn.
 */
struct LbdFileMethods {
    FILERESOURCE_SLOTS(LbdFile, (LbdFile * self));
    /* +0x07C */ void (*releaseHeader)(LbdFile *self);    /**< @see LbdFile__ReleaseHeader */
    /* +0x080 */ s32 (*loadDataBlock)(LbdFile *self);     /**< @see LbdFile__LoadDataBlock */
    /* +0x084 */ void (*releaseDataBlock)(LbdFile *self); /**< @see LbdFile__ReleaseDataBlock */
    /* +0x088 */ void (*setAutoLoadData)(LbdFile *self, s32 value); /**< @see LbdFile__SetAutoLoadData */
}; /* 34 slots, 0x8C bytes */

/**
 * @brief One of the stage's map-chunk files, STGnn\\Mnnn.LBD, loaded for one
 * element of the grid manager (class id 0x903). A FileResource subclass
 * with no subclasses; methods in src/cd/game_files.c.
 *
 * The files it is handed are the record table entries
 * GetStageMapChunkRecord(stage, chunk) returns: StageMap__ComputeChunkLoadEntry
 * takes each one from the grid's callback, ObjM__GetGridRecord, and an
 * entry's first bytes are its path ("STG00\\M000.LBD" is record 9 of stage
 * 0's group).
 *
 * The file is read in two stages. loadHeader opens the file and reads its
 * first LBDFILE_HEADER_BLOCK_SIZE bytes into the ctor's fixed `buffer`
 * (LBDFILE_LOAD_HEADER); that block starts with an LbdFileHeader. When the
 * driver reports the read done (onRequestDone, LbdFile__AdvanceLoadState,
 * CD_FLAG_READ_DONE in `flags`), `headerReady` is set and, unless
 * setAutoLoadData turned it off, loadDataBlock reads the optional data block
 * the header locates into a second allocation, `dataBuffer`
 * (LBDFILE_LOAD_DATA), setting `dataReady` when that read completes.
 * releaseHeader and releaseDataBlock free the two halves.
 *
 * Like every FileResource client it runs on the active driver: the ctor,
 * finalize, onRequestDone and cancelRequests chain to
 * GetActiveDataSourceMethods()'s, and GetLbdFileMethods is in
 * sDataSourceClientGetters, so SetActiveDataSource rebinds this table's
 * file-I/O slots.
 *
 * Lifecycle: its one user is the grid manager (StageMap), which makes one per
 * grid element (New_LbdFile in StageMap__StageMap, with `freeGuard` set so
 * freeBuffer keeps the fixed buffer), loads each element's chunk through it
 * (StageMap__ApplyChunkLoads), links the header block's placements and
 * models into the element's cells (StageMap__PopulateSlotCells), then marks
 * the header consumed (LBDFILE_HEADER_CONSUMED). ObjM__CheckAuxTrigger hands
 * the data block to TryDreamAuxTrigger and releases it when that returns 0.
 */
struct LbdFile {
    FILERESOURCE_FIELDS(LbdFileMethods);
    /* +0x02C */ s16 headerReady; /**< 1 when the header is read; StageMap__OnDrawSystemEvent sets LBDFILE_HEADER_CONSUMED. */
    /* +0x02E */ s16 dataReady; /**< 1 when the data block is read; cleared by StageMap__OnDrawSystemEvent. */
    /* +0x030 */ s16 chunkIndex; /**< The loaded chunk's record index in its stage (StageMap__ApplyChunkLoads); -1 when none. */
    /* +0x032 */ s16 elemKey; /**< The owner's element key: StageMap__StageMap's index, then StageMap__UpdateFootprintTracking's. */
    /* +0x034 */ void *dataBuffer; /**< The data block, BMemPMgrAlloc(dataSize); freed by ReleaseDataBlock. */
    /* +0x038 */ s32 autoLoadData; /**< Nonzero (the ctor's 1): the header's completion starts loadDataBlock. */
}; /* 0x3C bytes: New_LbdFile. `buffer` is the header block; `loadState` an LbdFileLoadState. */

/**
 * @brief The type the processBuffer slot (+0x078) is called through:
 * FileResource declares that slot untyped, and this class's occupant is
 * LbdFile__LoadHeader. StageMap__ApplyChunkLoads calls it.
 */
typedef void (*LbdFileLoadHeaderFn)(LbdFile *self, char *name);

/**
 * @brief A no-argument view of loadDataBlock (+0x080), the type
 * LbdFile__AdvanceLoadState calls it through; the occupant still receives
 * the caller's `self`.
 */
typedef s32 (*LbdFileLoadDataBlockNoArgFn)(void);

/**
 * @brief A no-argument view of releaseDataBlock (+0x084), the type
 * LbdFile__LoadDataBlock calls it through; the occupant still receives the
 * caller's `self`.
 */
typedef void (*LbdFileReleaseDataBlockNoArgFn)(void);

/**
 * @brief releaseHeader (+0x07C) with the extra argument
 * StageMap__ClearSlotCells passes, the grid element, which
 * LbdFile__ReleaseHeader never reads.
 */
typedef void (*LbdFileReleaseHeaderElemFn)(LbdFile *self, void *elem);

/** @brief LbdFile's method table (see LbdFileMethods). */
extern LbdFileMethods gLbdFileMethods;

/**
 * @brief Returns LbdFile's method table.
 * @return &gLbdFileMethods.
 */
extern LbdFileMethods *GetLbdFileMethods(void);

/**
 * @brief Allocates an LbdFile from the BMemPMgr pool and constructs it.
 * @return The new loader, or NULL when the pool is exhausted.
 */
LbdFile *New_LbdFile(void);

/**
 * @brief Constructor (slot +0x008): the active driver's ctor, no chunk, the
 * flags and data block cleared, autoLoadData on, and the fixed header-block
 * `buffer` allocated.
 * @param self The object being constructed.
 */
void LbdFile__LbdFile(LbdFile *self);

/**
 * @brief Finalizer (slot +0x00C): releases the data block, then the active
 * driver's finalize.
 * @param self The object being finalized.
 */
void LbdFile__Finalize(LbdFile *self);

/**
 * @brief Slot +0x064, onRequestDone: after the header read, sets
 * `headerReady` and starts loadDataBlock when autoLoadData is on; after the
 * data read, sets `dataReady`. Then the active driver's onRequestDone.
 * @param self The loader whose request completed.
 */
void LbdFile__AdvanceLoadState(LbdFile *self);

/**
 * @brief Slot +0x074, cancelRequests: the active driver's, then the ready
 * flags and the load state cleared.
 * @param self The loader.
 */
void LbdFile__CancelRequests(LbdFile *self);

/**
 * @brief Slot +0x078 (processBuffer): starts reading the header block of
 * file `name` into `buffer`, cancelling any load in progress.
 * @param self The loader.
 * @param name The chunk file's path; NULL does nothing.
 */
void LbdFile__LoadHeader(LbdFile *self, char *name);

/**
 * @brief Slot +0x07C: frees the buffer (the fixed one survives while
 * `freeGuard` is set), clears `headerReady` and forgets the chunk.
 * @param self The loader.
 */
void LbdFile__ReleaseHeader(LbdFile *self);

/**
 * @brief Slot +0x080: when the header says there is a data block and no load
 * is pending, releases the old block, allocates `dataSize` bytes and reads
 * the block from `dataOffset`.
 * @param self The loader.
 * @return 1 when a read was started, else 0.
 */
s32 LbdFile__LoadDataBlock(LbdFile *self);

/**
 * @brief Slot +0x084: clears `dataReady` and frees the data block.
 * @param self The loader.
 */
void LbdFile__ReleaseDataBlock(LbdFile *self);

/**
 * @brief Slot +0x088: sets whether the header's completion starts
 * loadDataBlock.
 * @param self  The loader.
 * @param value Nonzero to load the data block automatically.
 */
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value);

#endif
