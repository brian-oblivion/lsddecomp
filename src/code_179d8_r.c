#include "common.h"
#include <libcd.h>
#include <strings.h>
#include "CdDriver.h"

/*
 * The CD driver's read state machine, its request-queue nodes and its
 * file-table lookups (the class, the queue and the table are
 * include/CdDriver.h's).
 *
 * The state machine takes one step each time ServiceCdDriver
 * (code_179d8_q.c, a VSync callback) runs: it calls TickCdStateMachine or
 * TickCdLoadFileStateMachine as gCdTickStep says, after a CdDriver method
 * (code_179d8_s.c) has started an operation with StartCdOperation and set
 * gCdSeekParam, gCdReadSectorCount and gCdReadBuffer. gCdState walks
 * CD_STATE_SETLOC (CdControlF(CdlSetloc) to gCdSeekParam->pos),
 * CD_STATE_SETLOC_WAIT (poll CdSync), CD_STATE_READ (CdRead) and
 * CD_STATE_READ_WAIT (poll CdReadSync); SetCdState moves it and
 * ResetCdStateMachine ends the operation and marks the driver idle. A seek
 * that errors or stays unanswered for CD_WAIT_TIMEOUT polls, and a read that
 * errors, go back to CD_STATE_SETLOC.
 *
 * The two tick functions differ in three places. TickCdStateMachine, which
 * Open and Seek use (Read starts it at CD_STATE_READ), ends the operation
 * when the seek completes and CdFlushes after a read error;
 * TickCdLoadFileStateMachine, LoadFile's, goes on to read after the seek,
 * and when the read is done restores the gCdSeekParam LoadFile saved in
 * gCdSavedSeekParam.
 *
 * AllocCdRequestNode appends a zeroed node to gCdRequestQueue and
 * FreeCdRequestNode unlinks one; EnqueueCdRequest (code_179d8_q.c) fills
 * them and CdDriver__RunRequestQueue (code_179d8_s.c) consumes them from the
 * head. FindCdFileEntry and FindCdFileIndex look a name up in gFileTable by
 * substring, GetCdFileEntry indexes it. Every function but the three
 * state setters brackets its body with LockCd / UnlockCd, which makes
 * ServiceCdDriver skip its tick in between.
 */

/* Defined in code_179d8_q.c. */
extern void LockCd(void);
extern void UnlockCd(void);

/* CdSync / CdReadSync mode: return the current status at once (0 waits).
 * CdReadSync then answers -1 for an error, 0 when the read is done, and
 * otherwise the sectors still to come. */
#define CD_SYNC_POLL 1

/* Polls of CdSync that answer CdlNoIntr before the seek is issued again. */
#define CD_WAIT_TIMEOUT 601

/* The game's pool allocator, src/BMemPMgr.c. */
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

extern s32 gCdTimeoutCounter; /* CD_STATE_SETLOC_WAIT's polls; SetCdState clears it */

CdRequestNode *AllocCdRequestNode(void) {
    CdRequestNode *node;
    CdRequestNode *head;
    CdRequestNode *cur;

    LockCd();
    node = BMemPMgrAlloc(sizeof(CdRequestNode));
    if (node != NULL) {
        head = gCdRequestQueue;
        node->prev = NULL;
        node->next = NULL;
        node->active = 0;
        node->unk4 = 0;
        if (head != NULL) {
            cur = head;
            while (cur->next != NULL) {
                cur = cur->next;
            }
            cur->next = node;
            node->prev = cur;
        } else {
            gCdRequestQueue = node;
        }
    }
    UnlockCd();
    return node;
}

void FreeCdRequestNode(CdRequestNode *node) {
    CdRequestNode *prev;
    CdRequestNode *next;

    LockCd();
    if (node != NULL) {
        prev = node->prev;
        if (prev != NULL) {
            prev->next = node->next;
        } else {
            gCdRequestQueue = node->next;
        }
        next = node->next;
        if (next != NULL) {
            next->prev = node->prev;
        }
        BMemPMgrFree(node);
    }
    UnlockCd();
}

/* The first entry whose name contains `name`, or NULL. The not-found return
 * skips UnlockCd, as FindCdFileIndex's does, so ServiceCdDriver stays off
 * until the next UnlockCd anywhere. */
void *FindCdFileEntry(char *name) {
    CdFileEntry *cur = gFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur->name, name) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur++;
    } while (i < gFileTableCount);
    return NULL;
}

/* The index of the first entry whose name contains `name`, or -1. */
s32 FindCdFileIndex(char *name) {
    CdFileEntry *cur = gFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur->name, name) == NULL) {
        i++;
        if (i >= gFileTableCount) {
            return -1;
        }
        cur++;
    }
    UnlockCd();
    return i;
}

void *GetCdFileEntry(s32 index) {
    CdFileEntry *table = gFileTable;
    CdFileEntry *entry;

    LockCd();
    entry = &table[index];
    UnlockCd();
    return entry;
}

/* Defined below. */
extern void ResetCdStateMachine(void);
extern void SetCdState(s32 state);

void TickCdStateMachine(void) {
    s32 result;
    s32 newState;

    LockCd();
    switch (gCdState) {
        case CD_STATE_SETLOC:
            if (CdControlF(CdlSetloc, (u_char *)&gCdSeekParam->pos) == 0) {
                goto unlock;
            }
            newState = CD_STATE_SETLOC_WAIT;
            break;
        case CD_STATE_SETLOC_WAIT:
            switch (CdSync(CD_SYNC_POLL, NULL)) {
                /* MATCHING: CdlDiskError's case first; last, its test flips polarity */
                case CdlDiskError:
                    newState = CD_STATE_SETLOC;
                    break;
                case CdlComplete:
                    ResetCdStateMachine();
                    goto unlock;
                case CdlNoIntr:
                    gCdTimeoutCounter++;
                    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT) {
                        goto unlock;
                    }
                    newState = CD_STATE_SETLOC;
                    break;
                default:
                    goto unlock;
            }
            break;
        case CD_STATE_READ:
            if (CdRead(gCdReadSectorCount, gCdReadBuffer, CdlModeSpeed) == 0) {
                goto unlock;
            }
            newState = CD_STATE_READ_WAIT;
            break;
        case CD_STATE_READ_WAIT:
            result = CdReadSync(CD_SYNC_POLL, NULL);
            if (result == -1) {
                CdFlush();
                newState = CD_STATE_SETLOC;
                break;
            }
            if (result == 0) {
                ResetCdStateMachine();
            }
            goto unlock;
        default:
            goto unlock;
    }
    SetCdState(newState);
unlock:
    UnlockCd();
}

void TickCdLoadFileStateMachine(void) {
    s32 result;
    s32 newState;

    LockCd();
    switch (gCdState) {
        case CD_STATE_SETLOC:
            if (CdControlF(CdlSetloc, (u_char *)&gCdSeekParam->pos) == 0) {
                goto unlock;
            }
            newState = CD_STATE_SETLOC_WAIT;
            break;
        case CD_STATE_SETLOC_WAIT:
            switch (CdSync(CD_SYNC_POLL, NULL)) {
                case CdlComplete:
                    newState = CD_STATE_READ;
                    break;
                case CdlNoIntr:
                    gCdTimeoutCounter++;
                    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT) {
                        goto unlock;
                    }
                    newState = CD_STATE_SETLOC;
                    break;
                case CdlDiskError:
                    newState = CD_STATE_SETLOC;
                    break;
                default:
                    goto unlock;
            }
            break;
        case CD_STATE_READ:
            if (CdRead(gCdReadSectorCount, gCdReadBuffer, CdlModeSpeed) == 0) {
                goto unlock;
            }
            newState = CD_STATE_READ_WAIT;
            break;
        case CD_STATE_READ_WAIT:
            result = CdReadSync(CD_SYNC_POLL, NULL);
            if (result == -1) {
                newState = CD_STATE_SETLOC;
                break;
            }
            if (result == 0) {
                ResetCdStateMachine();
                gCdSeekParam = gCdSavedSeekParam;
                gCdSavedSeekParam = NULL;
            }
            goto unlock;
        default:
            goto unlock;
    }
    SetCdState(newState);
unlock:
    UnlockCd();
}

/* op is a CD_OPERATION_* and state the first CD_STATE_*; the operation is the
 * queue's head request, which is marked active. */
void StartCdOperation(s32 op, s32 state) {
    gCdBusy = 1;
    gCdOperation = op;
    gCdState = state;
    gCdIdle = 0;
    gCdRequestQueue->active = 1;
}

/* No operation, no tick step: the driver is idle. */
void ResetCdStateMachine(void) {
    gCdOperation = 0;
    gCdState = CD_STATE_IDLE;
    gCdTickStep = CD_TICK_NONE;
    gCdIdle = 1;
    gCdTimeoutCounter = 0;
    gCdBusy = 0;
}

void SetCdState(s32 state) {
    gCdState = state;
    gCdTimeoutCounter = 0;
}
