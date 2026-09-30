/*
 * LbdFile's methods (include/lbd_file.h: the loader for one stage map
 * chunk, STGnn\Mnnn.LBD), in ROM order: New_LbdFile to
 * LbdFile__SetAutoLoadData, then its getter GetLbdFileMethods; its method
 * table closes the file. The getters over the game's table of file names,
 * which follow it in ROM, are in game_files.c.
 */
#include "common.h"
#include "lbd_file.h"
#include "bmem_pmgr.h"
#include "data_source.h"
#include <stdio.h>

/* allocator: a new map-chunk loader from the pool */
LbdFile *New_LbdFile(void) {
    LbdFile *obj = BMemPMgrAlloc(sizeof(LbdFile));
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
    self->buffer = BMemPMgrAlloc(LBDFILE_HEADER_BLOCK_SIZE);
    if (self->buffer != NULL) {
        self->bufferSize = LBDFILE_HEADER_BLOCK_SIZE;
    }
}

/* slot +0x00C of gLbdFileMethods (finalize) */
void LbdFile__Finalize(LbdFile *self) {
    self->methods->releaseDataBlock(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* slot +0x064 of gLbdFileMethods (onRequestDone) */
void LbdFile__AdvanceLoadState(LbdFile *self) {
    if (self->loadState == LBDFILE_LOAD_HEADER) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->loadState = LBDFILE_LOAD_IDLE;
            self->headerReady = 1;
            if (self->autoLoadData != 0) {
                self->methods->loadDataBlock(self);
            }
        }
    } else if (self->loadState == LBDFILE_LOAD_DATA) {
        if (self->flags & CD_FLAG_READ_DONE) {
            self->dataReady = 1;
            self->loadState = LBDFILE_LOAD_IDLE;
        }
    }
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
}

/* slot +0x074 of gLbdFileMethods (cancelRequests) */
void LbdFile__CancelRequests(LbdFile *self) {
    GetActiveDataSourceMethods()->cancelRequests((FileResource *)self);
    self->headerReady = 0;
    self->dataReady = 0;
    self->loadState = LBDFILE_LOAD_IDLE;
}

/* slot +0x078 of gLbdFileMethods (processBuffer): read the chunk file's header
 * block into the fixed buffer, cancelling any load in progress */
void LbdFile__LoadHeader(LbdFile *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->loadState == LBDFILE_LOAD_IDLE) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->loadState = LBDFILE_LOAD_HEADER;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, LBDFILE_HEADER_BLOCK_SIZE);
    }
}

void LbdFile__ReleaseHeader(LbdFile *self) {
    self->methods->freeBuffer(self);
    self->headerReady = 0;
    self->chunkIndex = -1;
}

/* slot +0x080 of gLbdFileMethods: load the data block the header describes */
/* MATCHING: both gates in one condition around the body. Two early returns
 * reload the loader into the first argument before releaseDataBlock. */
s32 LbdFile__LoadDataBlock(LbdFile *self) {
    s32 size;

    if (((LbdFileHeader *)self->buffer)->hasData != 0 && self->loadState == LBDFILE_LOAD_IDLE) {
        self->methods->releaseDataBlock(self);
        size = ((LbdFileHeader *)self->buffer)->dataSize;
        self->dataBuffer = BMemPMgrAlloc(size);
        if (self->dataBuffer == NULL) {
            return 0;
        }
        self->loadState = LBDFILE_LOAD_DATA;
        self->methods->seek(self, ((LbdFileHeader *)self->buffer)->dataOffset, SEEK_SET);
        self->methods->read(self, self->dataBuffer, size);
        return 1;
    }
    return 0;
}

/* slot +0x084 of gLbdFileMethods */
void LbdFile__ReleaseDataBlock(LbdFile *self) {
    self->dataReady = 0;
    if (self->dataBuffer != NULL) {
        self->dataBuffer = BMemPMgrFree(self->dataBuffer);
    }
}

/* slot +0x088 of gLbdFileMethods */
void LbdFile__SetAutoLoadData(LbdFile *self, s32 value) {
    self->autoLoadData = value;
}

LbdFileMethods *GetLbdFileMethods(void) {
    return &gLbdFileMethods;
}

/* LbdFile's method table (include/lbd_file.h). A (void *) entry is a
 * method declared on another class's type than its slot's. */

/* LbdFile: FileResource's table with the ctor, finalize, onRequestDone and
 * cancelRequests, loadHeader as processBuffer, then the data-block slots. */
LbdFileMethods gLbdFileMethods = {
    /* +0x000 header */ LBDFILE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ LbdFile__LbdFile,
    /* +0x00C finalize */ LbdFile__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ LbdFile__AdvanceLoadState,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ LbdFile__CancelRequests,
    /* +0x078 processBuffer */ LbdFile__LoadHeader,
    /* +0x07C releaseHeader */ LbdFile__ReleaseHeader,
    /* +0x080 loadDataBlock */ LbdFile__LoadDataBlock,
    /* +0x084 releaseDataBlock */ LbdFile__ReleaseDataBlock,
    /* +0x088 setAutoLoadData */ LbdFile__SetAutoLoadData,
};
