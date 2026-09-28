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
 * code_1677c.c reaches all of this through wrappers gated on
 * `gActiveDataSource == 0x13`, this class's header word; the other value that gate
 * takes, 0x23, selects the SPU/VAB streamer in PlacementGridVabSound.c. So the two
 * are interchangeable data sources behind one small dispatch layer.
 */
#include "common.h"
#include <libetc.h>
#include <libcd.h>
#include "CdDriver.h"
#include "DrawSystem.h"

extern void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);
extern s32 FindCdFileIndex(char *name); /* code_179d8_r: name -> table index */

void CdDriver__RequestLoadFile(CdDriver *self, char *name) {
    s32 *unassigned; /* never assigned: see the store below */
    s32 idx;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            /* MATCHING: retail stores 1 through an unassigned callee-saved
             * register (whatever the caller left in $s2), a bug in the
             * original; which object it meant to reach is unknowable. */
            unassigned[1] = 1;
            idx = FindCdFileIndex(name);
            EnqueueCdRequest(self, idx, CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= 4;
            }
        }
    }

    UnlockCd();
}

extern void LockCd(void);
extern void UnlockCd(void);
extern void StopCdServiceIfIdle(void);

void CdDriver__StopService(void) {
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}

extern void ResetCdStateMachine(void);             /* code_179d8_r: reset the state machine */
extern void FreeCdRequestNode(CdRequestNode *req); /* code_179d8_r: unlink+free */

void CdDriver__CancelRequests(CdDriver *self) {
    CdRequestNode *entry;
    CdRequestNode *node;
    CdRequestNode *next;
    CdFileEntry *saved;

    LockCd();

    entry = gCdRequestQueue;

    if (entry != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (entry->owner == self && entry->active != 0 && gCdIdle == 0) {
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

extern s32 sCdDriveInited;

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

s32 GetCdDriverMode(s32 *outMode2) {
    if (outMode2 != NULL) {
        *outMode2 = gCdSyncQueueMode;
    }
    return gCdAsyncEnabled;
}

extern s32 ServiceCdDriver(void);

s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback) {
    DrawSystem *obj;

    if (gCdBusy == 0) {
        if (useVSyncCallback == 0) {
            obj = GetDrawSystem();

            if (gCdAsyncEnabled == 0) {
                if (async != 0) {
                    obj->methods->setCallback(obj, (void (*)(void))ServiceCdDriver);
                }
            } else {
                if (async == 0) {
                    obj->methods->setCallback(obj, 0);
                }
            }
        }

        gCdUseVSyncCallback = useVSyncCallback;
        gCdAsyncEnabled = async;
        gCdSyncQueueMode = mode2;

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

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */
extern void printf(const char *fmt, void *arg1);
extern char *BuildCdFilePath(char *dest, char *suffix); /* code_179d8_r */
extern void InitCdDrive(void);

#define CD_SEARCH_RETRIES 0x65

s32 ResolveFileEntries(CdFileEntry *entries, s32 count) {
    CdFileEntry *end;
    char path[0x40];
    CdlFILE info;
    s32 tries;

    end = entries + count;

    InitCdDrive();

    for (; entries < end; entries++) {
        BuildCdFilePath(path, entries->name);

        for (tries = 0; tries < CD_SEARCH_RETRIES; tries++) {
            if (CdSearchFile(&info, path) != 0) {
                goto found;
            }
        }

        printf(sFileNotFoundMsg, path);

    found:
        /* CdLoc16 is the project's spelling of CdlLOC's four bytes (FileResource.h). */
        entries->pos = *(CdLoc16 *)&info.pos;
        entries->size = info.size;
    }

    return 1;
}

/* The driver's re-entrancy lock, and its one reader is ServiceCdDriver
 * below: the service tick returns immediately while gCdLock is set, so every
 * public entry point in this unit and in code_179d8_r/_s brackets its body
 * with LockCd()/UnlockCd() to keep the VSync-driven tick out of a
 * half-updated queue. Not a mutex -- nothing spins or blocks on it. */
extern s32 gCdLock;

void LockCd(void) {
    gCdLock = 1;
}

extern s32 gCdLock;

void UnlockCd(void) {
    gCdLock = 0;
}

extern s32 GetBMemPMgrBusy(void);             /* TmdRenderer */
extern void TickCdStateMachine(void);         /* code_179d8_r: state-machine step 1 */
extern void TickCdLoadFileStateMachine(void); /* code_179d8_r: state-machine step 2 */
extern s32 gCdQueueEnabled;

s32 ServiceCdDriver(void) {
    if (gCdLock != 0) {
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

    if (gCdQueueEnabled != 0) {
        GetCdDriverMethods()->runRequestQueue();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}

extern s32 gCdCallbackInstalled;

void StartCdService(void) {
    LockCd();

    if (gCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        gCdCallbackInstalled = 1;
    }

    gCdQueueEnabled = 1;
    UnlockCd();
}

extern s32 gCdCallbackInstalled;
extern s32 gCdQueueEnabled;

void StopCdServiceIfIdle(void) {
    LockCd();

    if (gCdTickStep == 0 && gCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(0);
        }
        gCdCallbackInstalled = 0;
        gCdQueueEnabled = 0;
    }

    UnlockCd();
}

extern s32 gCdQueueEnabled;

void DisableCdQueue(void) {
    LockCd();
    gCdQueueEnabled = 0;
    UnlockCd();
}

extern CdRequestNode *AllocCdRequestNode(void); /* code_179d8_r: alloc + link */

/* Fills a node AllocCdRequestNode has already linked onto gCdRequestQueue,
 * counts it against its owner and starts the service tick.
 * MATCHING: the stores are in retail's order (+0x08, +0x14, +0x0C, +0x10,
 * +0x18); this compiler keeps statement order. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1) {
    CdRequestNode *entry = AllocCdRequestNode();

    entry->op = op;
    entry->param0 = param0;
    entry->owner = owner;
    entry->fileIndex = fileIndex;
    entry->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}
