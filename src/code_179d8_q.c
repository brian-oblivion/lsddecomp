/*
 * The CD driver's module level: the three request methods that are not
 * per-request (requestLoadFile, stopService, cancelRequests), the queue's
 * front end, the driver mode, the file table and the service tick. The
 * class, the queue's types and the shared module state are
 * include/CdDriver.h's; the constructor is in code_179d8_o.c, the other
 * request methods in code_179d8_s.c, the state machines and the queue's
 * nodes in code_179d8_r.c, the blocking file calls in code_179d8_h.c.
 *
 *   - The driver mode. SetCdDriverMode sets gCdAsyncEnabled (requests are
 *     queued and run in the background), gCdSyncQueueMode (queued, but each
 *     run as a blocking spin) and gCdUseVSyncCallback (ServiceCdDriver is
 *     installed with VSyncCallback; otherwise it becomes the DrawSystem
 *     singleton's callback). It refuses while gCdBusy. InitCdDrive puts the
 *     drive in double speed once; the Is/Get functions read the state
 *     machine's flags back.
 *   - The queue's front end. EnqueueCdRequest fills a node
 *     AllocCdRequestNode has linked, counts it in its owner's
 *     pendingRequests and calls StartCdService. CdDriver__CancelRequests
 *     frees every node an owner has queued, first stopping the drive
 *     (CdFlush, ResetCdStateMachine) if that owner's request is running.
 *   - The service tick. ServiceCdDriver steps the state machine gCdTickStep
 *     names and then runs the queue (runRequestQueue) while sCdQueueEnabled;
 *     StartCdService installs it and enables the queue, StopCdServiceIfIdle
 *     removes it once no state machine runs, DisableCdQueue only stops the
 *     queue. The tick does nothing while sCdLock is set (LockCd/UnlockCd)
 *     or while the BMemPMgr allocator is busy.
 *   - The file table. SetFileTable / SetFileTableCount install a
 *     CdFileEntry array; ResolveFileEntries fills each entry's disc position
 *     and size with CdSearchFile, CD_SEARCH_ATTEMPTS tries per name.
 *
 * The game reaches all of it through GameApplicationFileResource.c's
 * wrappers while the active data source is this class's id, DATASOURCE_CD
 * (0x13); the other source is the SPU/VAB driver, DATASOURCE_SPU (0x23).
 */
#include "common.h"
#include <libetc.h>
#include <libcd.h>
#include "CdDriver.h"
#include "DrawSystem.h"

/* This unit's own functions, called before their definitions. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);
void InitCdDrive(void);
void LockCd(void);
void UnlockCd(void);
s32 ServiceCdDriver(void);
void StopCdServiceIfIdle(void);

/* code_179d8_r.c: the request list, the state machines, the file-table lookup. */
extern CdRequestNode *AllocCdRequestNode(void);     /* allocate and link at the tail */
extern void FreeCdRequestNode(CdRequestNode *node); /* unlink and free */
extern void ResetCdStateMachine(void);              /* end the operation, mark idle */
extern void TickCdStateMachine(void);               /* CD_TICK_STATE_MACHINE */
extern void TickCdLoadFileStateMachine(void);       /* CD_TICK_LOAD_FILE */
extern s32 FindCdFileIndex(char *name);             /* name -> gFileTable index */

/* code_179d8_h.c */
extern char *BuildCdFilePath(char *dest, char *name); /* "\\<data directory><name>;1" */

/* TmdRenderer.c */
extern s32 GetBMemPMgrBusy(void);

extern void printf(const char *fmt, ...);

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */

/* This unit's module state; code_179d8_r/_s see it only through the
 * functions below. */
extern s32 sCdDriveInited;       /* InitCdDrive has set the drive's mode */
extern s32 sCdLock;              /* LockCd / UnlockCd */
extern s32 sCdCallbackInstalled; /* StartCdService has run, StopCdServiceIfIdle not since */
extern s32 sCdQueueEnabled;      /* ServiceCdDriver runs the request queue */

void CdDriver__RequestLoadFile(CdDriver *self, char *name) {
    s32 *unassigned; /* never assigned */
    s32 fileIndex;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            /* MATCHING: retail's bug, a store through the caller's $s2 */
            unassigned[1] = 1;
            fileIndex = FindCdFileIndex(name);
            EnqueueCdRequest(self, fileIndex, CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= CD_FLAG_NONE_PENDING;
            }
        }
    }

    UnlockCd();
}

void CdDriver__StopService(void) {
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}

void CdDriver__CancelRequests(CdDriver *self) {
    CdRequestNode *head;
    CdRequestNode *node;
    CdRequestNode *next;
    CdFileEntry *saved;

    LockCd();

    head = gCdRequestQueue;

    if (head != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (head->owner == self && head->active != 0 && gCdIdle == 0) {
            CdFlush();
            ResetCdStateMachine();
            saved = gCdSavedSeekParam;
            gCdSavedSeekParam = NULL;
            gCdSeekParam = saved;
        }

        for (node = gCdRequestQueue; node != NULL; node = next) {
            next = node->next;
            if (node->owner == self) {
                FreeCdRequestNode(node);
                self->pendingRequests--;
            }
        }
    }

    UnlockCd();
}

CdDriverMethods *GetCdDriverMethods(void) {
    return &gCdDriverMethods;
}

void InitCdDrive(void) {
    u8 mode;

    if (sCdDriveInited != 0) {
        return;
    }

    CdSetDebug(0);
    mode = CdlModeSpeed;
    while (CdControlB(CdlSetmode, &mode, NULL) == 0) {
    }
    sCdDriveInited = 1;
}

s32 IsCdBusy(void) {
    return gCdBusy;
}

s32 IsCdIdle(void) {
    return gCdIdle;
}

s32 GetCdOperation(void) {
    return gCdOperation;
}

s32 GetCdState(void) {
    return gCdState;
}

/* gCdSyncQueueMode is written only by SetCdDriverMode's second argument.
 * With it set and gCdAsyncEnabled clear, the request methods
 * (code_179d8_s.c) still queue each request but run it as a blocking spin
 * when runRequestQueue dispatches it; with both clear they skip the queue. */

s32 GetCdDriverMode(s32 *outSyncQueueMode) {
    if (outSyncQueueMode != NULL) {
        *outSyncQueueMode = gCdSyncQueueMode;
    }
    return gCdAsyncEnabled;
}

s32 SetCdDriverMode(s32 async, s32 syncQueueMode, s32 useVSyncCallback) {
    DrawSystem *drawSystem;

    if (gCdBusy == 0) {
        if (useVSyncCallback == 0) {
            drawSystem = GetDrawSystem();

            if (gCdAsyncEnabled == 0) {
                if (async != 0) {
                    drawSystem->methods->setCallback(drawSystem, (void (*)(void))ServiceCdDriver);
                }
            } else {
                if (async == 0) {
                    drawSystem->methods->setCallback(drawSystem, NULL);
                }
            }
        }

        gCdUseVSyncCallback = useVSyncCallback;
        gCdAsyncEnabled = async;
        gCdSyncQueueMode = syncQueueMode;

        return 1;
    }

    return 0;
}

void SetFileTable(CdFileEntry *table) {
    gFileTable = table;
}

void SetFileTableCount(s32 count) {
    gFileTableCount = count;
}

s32 GetFileTableCount(void) {
    return gFileTableCount;
}

s32 ResolveFileEntries(CdFileEntry *entries, s32 count) {
    CdFileEntry *end;
    char path[CD_PATH_SIZE];
    CdlFILE info;
    s32 tries;

    end = entries + count;

    InitCdDrive();

    for (; entries < end; entries++) {
        BuildCdFilePath(path, entries->name);

        for (tries = 0; tries < CD_SEARCH_ATTEMPTS; tries++) {
            if (CdSearchFile(&info, path) != 0) {
                goto found;
            }
        }

        printf(sFileNotFoundMsg, path);

    found:
        entries->pos = info.pos;
        entries->size = info.size;
    }

    return 1;
}

/* The driver's re-entrancy lock, and its one reader is ServiceCdDriver
 * below: the service tick returns immediately while sCdLock is set, so every
 * public entry point in this unit and in code_179d8_r/_s brackets its body
 * with LockCd()/UnlockCd() to keep the VSync-driven tick out of a
 * half-updated queue. Not a mutex -- nothing spins or blocks on it. */

void LockCd(void) {
    sCdLock = 1;
}

void UnlockCd(void) {
    sCdLock = 0;
}

s32 ServiceCdDriver(void) {
    if (sCdLock != 0) {
        return 0;
    }

    if (GetBMemPMgrBusy() != 0) {
        return 0;
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback(NULL);
    }

    if (gCdTickStep == CD_TICK_STATE_MACHINE) {
        TickCdStateMachine();
    } else if (gCdTickStep == CD_TICK_LOAD_FILE) {
        TickCdLoadFileStateMachine();
    }

    if (sCdQueueEnabled != 0) {
        GetCdDriverMethods()->runRequestQueue();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}

void StartCdService(void) {
    LockCd();

    if (sCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        sCdCallbackInstalled = 1;
    }

    sCdQueueEnabled = 1;
    UnlockCd();
}

void StopCdServiceIfIdle(void) {
    LockCd();

    if (gCdTickStep == CD_TICK_NONE && sCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(NULL);
        }
        sCdCallbackInstalled = 0;
        sCdQueueEnabled = 0;
    }

    UnlockCd();
}

void DisableCdQueue(void) {
    LockCd();
    sCdQueueEnabled = 0;
    UnlockCd();
}

/* Fills a node AllocCdRequestNode has already linked onto gCdRequestQueue,
 * counts it against its owner and starts the service tick. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1) {
    CdRequestNode *node = AllocCdRequestNode();

    /* MATCHING: retail's store order; cc1 keeps statement order here */
    node->op = op;
    node->param0 = param0;
    node->owner = owner;
    node->fileIndex = fileIndex;
    node->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}
