#ifndef CDDRIVER_H
#define CDDRIVER_H

#include "FileResource.h"

/*
 * CdDriver -- the CD-ROM data-source driver (class id 0x13 = DATASOURCE_CD,
 * method table gCdDriverMethods), a FileResource subclass and NullDriver's
 * (gNullDriverMethods, 0x23) sibling; and, below the class, the request
 * queue, the file table and the module state its units share.
 *
 *   src/cd/CdDriver.c      the whole class, in five parts:
 *                       1. New_CdDriver, the ctor (FileResource's ctor, then
 *                       InitCdDrive), Finalize, NoOpSlot40
 *                       2. Open, Close, Seek, NoOpSlot50, Read, LoadFile,
 *                       RunRequestQueue: each enqueues a CD_OP_* request, or
 *                       starts it when RunRequestQueue dispatches it back
 *                       3. RequestLoadFile, StopService, CancelRequests, the
 *                       getter; the queue front end (EnqueueCdRequest), the
 *                       file table's setters, ResolveFileEntries, the lock
 *                       and the VSync service tick (ServiceCdDriver)
 *                       4. the read state machine, AllocCdRequestNode /
 *                       FreeCdRequestNode, the file-table lookups
 *                       5. the synchronous OpenCdFile / CloseCdFile /
 *                       GetCdFileSize / ReadCdFile the methods call when
 *                       the driver is not in async mode
 *
 * THE DRIVER RUNS ON OTHER CLASSES' OBJECTS. Nothing calls New_CdDriver.
 * SetActiveDataSource (src/app/GameApplicationFileResource.c) copies this table's eleven
 * interface slots (+0x040..+0x058, +0x068..+0x074) into FileResource's table
 * and every client table, so `self` in Open/Read/... is whatever
 * FileResource object called its own `open`/`read` (a TimImage, a TodSet,
 * ...), and a queued request's `owner` is that object. That is why the
 * fields the methods use -- isOpen, buffer, bufferSize, pos, size,
 * freeGuard, pendingRequests, flags, inQueueDispatch -- are all FileResource's,
 * and the object has no own fields: New_CdDriver allocates 0x2C bytes,
 * FileResource's size.
 *
 * Every own method is named for its slot (`classtable.py gCdDriverMethods
 * --vs gFileResourceMethods`); the slot names and types are FileResource's:
 * open/close/loadFile/onRequestDone/stopService return void and read's buf is
 * void *.
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
void CdDriver__Open(CdDriver *self, char *name, s32 param0, s32 param1); /* +0x044 open */
void CdDriver__Close(CdDriver *self);                                    /* +0x048 close */
s32 CdDriver__Seek(CdDriver *self, u32 offset, s32 mode);                /* +0x04C seek */
void CdDriver__NoOpSlot50(void);                                         /* +0x050 slot50 */
s32 CdDriver__Read(CdDriver *self, void *buf, u32 size);    /* +0x054 read: returns 0 */
void CdDriver__LoadFile(CdDriver *self, char *name);        /* +0x058 loadFile */
void CdDriver__RunRequestQueue(void);                       /* +0x068 runRequestQueue */
void CdDriver__RequestLoadFile(CdDriver *self, char *name); /* +0x06C requestLoadFile */
void CdDriver__StopService(void);                           /* +0x070 stopService */
void CdDriver__CancelRequests(CdDriver *self);              /* +0x074 cancelRequests */

/* One record of the file table: a name resolved once by ResolveFileEntries
 * (CdSearchFile on BuildCdFilePath(name)) and then reused as a seek target.
 * FindCdFileEntry / FindCdFileIndex / GetCdFileEntry walk it at this 0x1C
 * stride; the state machines seek to `pos` of the entry sCdSeekParam holds. */
typedef struct CdFileEntry {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ CdlLOC pos;
    /* +0x18 */ u32 size;
} CdFileEntry; /* size 0x1C */

/* One queued request: AllocCdRequestNode allocates and links it at the tail
 * of sCdRequestQueue, EnqueueCdRequest fills it, StartCdOperation sets
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

/* sCdTickStep: which of CdDriver.c's two state machines ServiceCdDriver
 * ticks. */
#define CD_TICK_NONE 0          /* neither: ResetCdStateMachine's value */
#define CD_TICK_STATE_MACHINE 1 /* TickCdStateMachine */
#define CD_TICK_LOAD_FILE 2     /* TickCdLoadFileStateMachine */

/* sCdOperation: which method's request the state machine is running,
 * StartCdOperation's first argument and GetCdOperation's (and so
 * GetActiveDataSourceOperation's) result. Each value is passed by exactly one
 * of CdDriver.c's methods. 0 is also what ResetCdStateMachine leaves
 * when nothing runs: Close resets straight after starting. */
#define CD_OPERATION_CLOSE 0
#define CD_OPERATION_OPEN 1
#define CD_OPERATION_SEEK 2
#define CD_OPERATION_READ 3
#define CD_OPERATION_LOAD_FILE 4

/* sCdState: the state machines' phase, StartCdOperation's second argument
 * (CdDriver.c's banner; that unit still spells them as literals). */
#define CD_STATE_IDLE 0        /* ResetCdStateMachine's value */
#define CD_STATE_SETLOC 1      /* issue CdControl(CdlSetloc) */
#define CD_STATE_SETLOC_WAIT 2 /* poll CdSync for it */
#define CD_STATE_READ 7        /* issue CdRead */
#define CD_STATE_READ_WAIT 8   /* poll CdReadSync */

/* A CD-ROM data sector's user data (2048 bytes; <libcd.h>'s CdlModeSize0/1
 * clear). CdRead counts sectors, so byte sizes and offsets are shifted by
 * CD_SECTOR_SHIFT (ReadCdFile, CdDriver__Seek), and GetCdFileSize reports a
 * file's size in whole sectors. */
#define CD_SECTOR_SIZE 2048
#define CD_SECTOR_SHIFT 11

/* A path buffer for BuildCdFilePath's "\\<data directory><name>;1". */
#define CD_PATH_SIZE 64

/* The file lookups call CdSearchFile this many times before they print
 * "File not found" and give up (OpenCdFile, ResolveFileEntries). */
#define CD_SEARCH_ATTEMPTS 101

/* The driver's free functions, in CdDriver.c's order. */

/* The drive and its state, as GameApplicationFileResource.c's data-source
 * wrappers read it. */
extern void InitCdDrive(void);
extern s32 IsCdBusy(void);
extern s32 IsCdIdle(void);
extern s32 GetCdOperation(void);
extern s32 GetCdState(void);
extern s32 GetCdDriverMode(s32 *outSyncQueueMode);
extern s32 SetCdDriverMode(s32 async, s32 syncQueueMode, s32 useVSyncCallback);

/* The file table: SetFileTable/SetFileTableCount install a CdFileEntry array,
 * ResolveFileEntries fills each entry's position and size from the disc. */
extern void SetFileTable(CdFileEntry *table);
extern void SetFileTableCount(s32 count);
extern s32 GetFileTableCount(void);
extern s32 ResolveFileEntries(CdFileEntry *entries, s32 count);

/* The lock, the service and the request queue. */
extern void LockCd(void);
extern void UnlockCd(void);
extern s32 ServiceCdDriver(void);
extern void StartCdService(void);
extern void StopCdServiceIfIdle(void);
extern void DisableCdQueue(void);
extern void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);
extern CdRequestNode *AllocCdRequestNode(void);     /* allocate and link at the tail */
extern void FreeCdRequestNode(CdRequestNode *node); /* unlink and free */

/* The file-table lookups. */
extern void *FindCdFileEntry(char *name);
extern s32 FindCdFileIndex(char *name); /* name -> sFileTable index */
extern void *GetCdFileEntry(s32 index); /* the entry is also its name, its first member */

/* The state machines. op is a CD_OPERATION_* and state the first CD_STATE_*. */
extern void TickCdStateMachine(void);         /* CD_TICK_STATE_MACHINE */
extern void TickCdLoadFileStateMachine(void); /* CD_TICK_LOAD_FILE */
extern void StartCdOperation(s32 op, s32 state);
extern void ResetCdStateMachine(void); /* end the operation, mark idle */
extern void SetCdState(s32 state);

/* The blocking file access the read driver's methods use. */
extern void FileResource__InstallCdReadDriver(FileResource *self);
extern void FileResource__DestroyCdReadDriver(FileResource *self);
extern void OpenCdFile(CdDriver *self, char *name);
extern char *BuildCdFilePath(char *dest, char *name); /* "\\<data directory><name>;1" */
extern void CloseCdFile(CdDriver *self);
extern s32 GetCdFileSize(CdDriver *self);
extern s32 ReadCdFile(CdDriver *self, void *buf, s32 size);
extern s32 GetCdUseVSyncCallback(void);

#endif /* CDDRIVER_H */
