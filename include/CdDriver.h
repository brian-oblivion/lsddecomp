#ifndef CDDRIVER_H
#define CDDRIVER_H

#include "common.h"

/*
 * CdDriver -- the game's CD-ROM file driver: its request queue, its file
 * table and the module state its three units share (track 4b, round 85).
 *
 *   src/code_179d8_q.c  queue front end (EnqueueCdRequest, CancelRequests),
 *                       the file table's setters, ResolveFileEntries, the
 *                       lock and the VSync service tick (ServiceCdDriver)
 *   src/code_179d8_r.c  the read state machine, AllocCdRequestNode /
 *                       FreeCdRequestNode, the file-table lookups
 *   src/code_179d8_s.c  Class6D4E8's methods, which enqueue and run requests
 *
 * Every global here was declared in two or more of those units, with up to
 * three different types; the types below are the ones their accessors need.
 * A global only one unit touches stays a local extern in that unit.
 * Class6D4E8 itself is track 4a's and is referred to by tag only.
 */

struct Class6D4E8;

/* A disc position in the shape of Psy-Q's CdlLOC, declared as two s16 so the
 * struct is 2-aligned and a whole-struct copy compiles to lwl/lwr + swl/swr
 * (the idiom CLAUDE.md documents). The halves are never read apart. */
typedef struct CdLoc16 {
    s16 unk0;
    s16 unk2;
} CdLoc16;

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
    /* +0x00 */ s32 active;               /* set by StartCdOperation when the op starts */
    /* +0x04 */ s32 unk4;                 /* zeroed at allocation; nonzero ORs flags bit 0 */
    /* +0x08 */ s32 op;                   /* CD_OP_* */
    /* +0x0C */ struct Class6D4E8 *owner; /* the requesting object */
    /* +0x10 */ s32 fileIndex;            /* FindCdFileIndex's index, 0 if none */
    /* +0x14 */ s32 param0;
    /* +0x18 */ s32 param1;
    /* +0x1C */ struct CdRequestNode *prev;
    /* +0x20 */ struct CdRequestNode *next;
} CdRequestNode; /* size 0x24 */

/* A queue node's `op`: EnqueueCdRequest's third argument, dispatched back by
 * CdDriver__RunRequestQueue to the owner's slot of the same name. */
#define CD_OP_OPEN      2
#define CD_OP_CLOSE     3
#define CD_OP_SEEK      4
#define CD_OP_READ      5
#define CD_OP_LOAD_FILE 7

/* gCdTickStep: which of code_179d8_r.c's two state machines ServiceCdDriver
 * ticks. */
#define CD_TICK_STATE_MACHINE 1 /* TickCdStateMachine */
#define CD_TICK_LOAD_FILE     2 /* TickCdLoadFileStateMachine */

extern s32 gCdAsyncEnabled;
extern s32 D_8008A860;
extern s32 gCdBusy;                     /* 0/1 */
extern CdFileEntry *gFileTable;         /* SetFileTable */
extern s32 gFileTableCount;             /* SetFileTableCount */
extern s32 gCdIdle;                     /* 0/1 */
extern s32 gCdOperation;                /* StartCdOperation's op, GetCdOperation's result */
extern s32 gCdState;                    /* the state machine's phase */
extern CdFileEntry *gCdSeekParam;       /* the state machines seek to &gCdSeekParam->pos */
extern s32 gCdReadSectorCount;          /* CdRead sector count */
extern void *gCdReadBuffer;             /* CdRead target buffer */
extern CdFileEntry *gCdSavedSeekParam;  /* LoadFile's saved gCdSeekParam */
extern CdRequestNode *gCdRequestQueue;  /* list head */
extern s32 gCdTickStep;                 /* CD_TICK_* */
extern s32 gCdUseVSyncCallback;

#endif /* CDDRIVER_H */
