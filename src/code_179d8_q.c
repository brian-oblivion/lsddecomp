/*
 * code_179d8_q -- the CD-ROM read driver.
 *
 * This unit is the module-level half of the class whose method table is
 * gCdDriverMethods (header word 0x13; the object itself and its read/seek/close
 * methods are code_179d8_s, its constructor code_179d8_o). It owns four
 * things, all of them singleton state in .sdata:
 *
 *   - the file table: an array of 0x1C-byte CdFileEntry records (name, disc
 *     position, size) at gFileTable/gFileTableCount, whose positions
 *     ResolveFileEntries fills in with CdSearchFile;
 *   - the driver mode: gCdAsyncEnabled and gCdUseVSyncCallback, set through
 *     SetCdDriverMode, which decide whether a request is queued and serviced
 *     in the background or performed by a blocking CdSync spin;
 *   - the request queue's front door: EnqueueCdRequest appends a node to the
 *     gCdRequestQueue list (code_179d8_r owns the list itself) and starts the
 *     service;
 *   - the service pump: ServiceCdDriver, installed as a VSyncCallback (or as
 *     the DrawSystem singleton's callback, its setCallback slot +0x084), which ticks
 *     code_179d8_r's CD state machine and drains the request queue, plus
 *     LockCd/UnlockCd, the latch that keeps that tick out of a half-updated
 *     queue.
 *
 * GameApplicationFileResource.c reaches all of this through wrappers gated on
 * `gActiveDataSource == 0x13`, this class's header word; the other value that gate
 * takes, 0x23, selects the SPU/VAB streamer in PlacementGridVabSound.c. So the two
 * are interchangeable data sources behind one small dispatch layer.
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
    s32 *unassigned; /* never assigned: see the store below */
    s32 fileIndex;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            /* MATCHING: retail stores 1 through an unassigned callee-saved
             * register (whatever the caller left in $s2), a bug in the
             * original; which object it meant to reach is unknowable. */
            unassigned[1] = 1;
            fileIndex = FindCdFileIndex(name);
            EnqueueCdRequest(self, fileIndex, CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= 4;
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

/* The class's own table getter (include/CdDriver.h) -- an address-of, not
 * gp_rel: gCdDriverMethods lives in .data, not .sdata. */
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
    while (CdControlB(CdlSetmode, &mode, 0) == 0) {
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
                    drawSystem->methods->setCallback(drawSystem, 0);
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
    char path[0x40];
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
        VSyncCallback(0);
    }

    if (gCdTickStep == 1) {
        TickCdStateMachine();
    } else if (gCdTickStep == 2) {
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

    if (gCdTickStep == 0 && sCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(0);
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
 * counts it against its owner and starts the service tick.
 * MATCHING: the stores are in retail's order (+0x08, +0x14, +0x0C, +0x10,
 * +0x18); this compiler keeps statement order. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1) {
    CdRequestNode *node = AllocCdRequestNode();

    node->op = op;
    node->param0 = param0;
    node->owner = owner;
    node->fileIndex = fileIndex;
    node->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}
