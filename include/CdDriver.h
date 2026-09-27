#ifndef CDDRIVER_H
#define CDDRIVER_H

#include "FileResource.h"

/*
 * CdDriver -- the CD-ROM data-source driver (class id 0x13 = DATASOURCE_CD,
 * method table gCdDriverMethods), a FileResource subclass and VabDriver's
 * (gVabDriverMethods, 0x23) sibling; and, below the class, the request
 * queue, the file table and the module state its units share (track 4b,
 * round 85).
 *
 *   src/code_179d8_o.c  New_CdDriver, the ctor (FileResource's ctor, then
 *                       InitCdDrive), Finalize, NoOpSlot40
 *   src/code_179d8_s.c  Open, Close, Seek, NoOpSlot50, Read, LoadFile,
 *                       RunRequestQueue: each enqueues a CD_OP_* request, or
 *                       starts it when RunRequestQueue dispatches it back
 *   src/code_179d8_q.c  RequestLoadFile, StopService, CancelRequests, the
 *                       getter; the queue front end (EnqueueCdRequest), the
 *                       file table's setters, ResolveFileEntries, the lock
 *                       and the VSync service tick (ServiceCdDriver)
 *   src/code_179d8_r.c  the read state machine, AllocCdRequestNode /
 *                       FreeCdRequestNode, the file-table lookups
 *   src/code_179d8_h.c  the synchronous OpenCdFile / CloseCdFile /
 *                       GetCdFileSize / ReadCdFile the methods call when
 *                       the driver is not in async mode
 *
 * THE DRIVER RUNS ON OTHER CLASSES' OBJECTS. Nothing calls New_CdDriver.
 * SetActiveDataSource (src/code_171e0.c) copies this table's eleven
 * interface slots (+0x040..+0x058, +0x068..+0x074) into FileResource's table
 * and every client table, so `self` in Open/Read/... is whatever
 * FileResource object called its own `open`/`read` (a TimImage, a TodSet,
 * ...), and a queued request's `owner` is that object. That is why the
 * fields the methods use -- isOpen, buffer, bufferSize, pos, size,
 * freeGuard, pendingRequests, flags, inQueueDispatch -- are all FileResource's
 * (pos and size, +0x018/+0x01C, were its pad18 until round 88), and the
 * object has no own fields: New_CdDriver allocates 0x2C bytes, FileResource's
 * size.
 *
 * Every own method is named for its slot (`classtable.py gCdDriverMethods
 * --vs gFileResourceMethods`); the slot names are FileResource's, which were named for
 * these occupants. Slot types are FileResource's too. Where the old local
 * views disagreed (open/close/loadFile/setFlag/stopService returning s32,
 * read's buf as s32), the occupants return void / take void * and the
 * bytes did not move when the views were replaced.
 *
 * NO FIELDS/SLOTS MACROS: no class lies below 0x13 (`typeviews.py --tree`).
 */

typedef struct CdDriver CdDriver;
typedef struct CdDriverMethods CdDriverMethods;

struct CdDriverMethods {
    FILERESOURCE_SLOTS(CdDriver, (CdDriver * self));
    /* The table is 29 slots and ends after +0x074 (FileResource's slot78 is
     * the next table's header). */
};

struct CdDriver {
    FILERESOURCE_FIELDS(CdDriverMethods);
}; /* 0x2C bytes: New_CdDriver */

extern CdDriverMethods gCdDriverMethods;
extern CdDriverMethods *GetCdDriverMethods(void);

CdDriver *New_CdDriver(void);
void CdDriver__CdDriver(CdDriver *self); /* +0x008 ctor */
void CdDriver__Finalize(CdDriver *self); /* +0x00C finalize: cancelRequests, freeBuffer */
void CdDriver__NoOpSlot40(void);         /* +0x040 slot40 */
void CdDriver__Open(CdDriver *self, char *name, s32 arg2, s32 arg3); /* +0x044 open */
void CdDriver__Close(CdDriver *self);                                /* +0x048 close */
s32 CdDriver__Seek(CdDriver *self, u32 offset, s32 mode);            /* +0x04C seek */
void CdDriver__NoOpSlot50(void);                                     /* +0x050 slot50 */
s32 CdDriver__Read(CdDriver *self, void *buf, u32 size);             /* +0x054 read: returns 0 */
void CdDriver__LoadFile(CdDriver *self, char *name);                 /* +0x058 loadFile */
void CdDriver__RunRequestQueue(void);                                /* +0x068 runRequestQueue */
void CdDriver__RequestLoadFile(CdDriver *self, char *name);          /* +0x06C requestLoadFile */
void CdDriver__StopService(void);                                    /* +0x070 stopService */
void CdDriver__CancelRequests(CdDriver *self);                       /* +0x074 cancelRequests */

/* One record of the file table: a name resolved once by ResolveFileEntries
 * (CdSearchFile on BuildCdFilePath(name)) and then reused as a seek target.
 * FindCdFileEntry / FindCdFileIndex / GetCdFileEntry walk it at this 0x1C
 * stride; the state machines seek to `pos` of the entry gCdSeekParam holds. */
typedef struct CdFileEntry {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ CdLoc16 pos;
    /* +0x18 */ u32 size;
} CdFileEntry; /* size 0x1C */

/* One queued request: AllocCdRequestNode allocates and links it at the tail
 * of gCdRequestQueue, EnqueueCdRequest fills it, StartCdOperation sets
 * `active` on the head node, CdDriver__RunRequestQueue dispatches `op` back
 * to `owner`'s slot of the same name, FreeCdRequestNode unlinks and frees. */
typedef struct CdRequestNode {
    /* +0x00 */ s32 active;      /* set by StartCdOperation when the op starts */
    /* +0x04 */ s32 unk4;        /* zeroed at allocation; nonzero ORs flags bit 0 */
    /* +0x08 */ s32 op;          /* CD_OP_* */
    /* +0x0C */ CdDriver *owner; /* the requesting object (any FileResource client) */
    /* +0x10 */ s32 fileIndex;   /* FindCdFileIndex's index, 0 if none */
    /* +0x14 */ s32 param0;
    /* +0x18 */ s32 param1;
    /* +0x1C */ struct CdRequestNode *prev;
    /* +0x20 */ struct CdRequestNode *next;
} CdRequestNode; /* size 0x24 */

/* A queue node's `op`: EnqueueCdRequest's third argument, dispatched back by
 * CdDriver__RunRequestQueue to the owner's slot of the same name. */
#define CD_OP_OPEN 2
#define CD_OP_CLOSE 3
#define CD_OP_SEEK 4
#define CD_OP_READ 5
#define CD_OP_LOAD_FILE 7

/* gCdTickStep: which of code_179d8_r.c's two state machines ServiceCdDriver
 * ticks. */
#define CD_TICK_STATE_MACHINE 1 /* TickCdStateMachine */
#define CD_TICK_LOAD_FILE 2     /* TickCdLoadFileStateMachine */

/* A CD-ROM data sector's user data (2048 bytes; <libcd.h>'s CdlModeSize0/1
 * clear). CdRead counts sectors, so byte sizes and offsets are shifted by
 * CD_SECTOR_SHIFT (ReadCdFile, CdDriver__Seek), and GetCdFileSize reports a
 * file's size in whole sectors. CD_SECTOR_SIZE is spelled exactly as
 * GraphicsResources.c's own definition, so the two may meet. */
#define CD_SECTOR_SIZE 2048
#define CD_SECTOR_SHIFT 11

/* A path buffer for BuildCdFilePath's "\\<data directory><name>;1". */
#define CD_PATH_SIZE 64

/* The file lookups call CdSearchFile this many times before they print
 * "File not found" and give up (OpenCdFile, ResolveFileEntries). */
#define CD_SEARCH_ATTEMPTS 101

/* The module state. Every global here was declared in two or more of the
 * units above, with up to three different types; the types below are the
 * ones their accessors need. A global only one unit touches stays a local
 * extern in that unit (track 4b, round 85). */
extern s32 gCdAsyncEnabled;
extern s32 gCdSyncQueueMode; /* nonzero with gCdAsyncEnabled 0: requests queue, then run blocking */
extern s32 gCdBusy;          /* 0/1 */
extern CdFileEntry *gFileTable;        /* SetFileTable */
extern s32 gFileTableCount;            /* SetFileTableCount */
extern s32 gCdIdle;                    /* 0/1 */
extern s32 gCdOperation;               /* StartCdOperation's op, GetCdOperation's result */
extern s32 gCdState;                   /* the state machine's phase */
extern CdFileEntry *gCdSeekParam;      /* the state machines seek to &gCdSeekParam->pos */
extern s32 gCdReadSectorCount;         /* CdRead sector count */
extern void *gCdReadBuffer;            /* CdRead target buffer */
extern CdFileEntry *gCdSavedSeekParam; /* LoadFile's saved gCdSeekParam */
extern CdRequestNode *gCdRequestQueue; /* list head */
extern s32 gCdTickStep;                /* CD_TICK_* */
extern s32 gCdUseVSyncCallback;

#endif /* CDDRIVER_H */
