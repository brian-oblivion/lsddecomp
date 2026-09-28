#include "common.h"
#include <libcd.h>
#include "CdDriver.h"

/*
 * code_179d8_r -- tail slice of the code_179d8 monolith, carved round 45.
 * Named (track 3), round 53 -- runner alpha. All 10 functions were already
 * matched C when this pass started (fresh carve, round 45/46, runner
 * foxtrot); nothing here changes a single byte, only names.
 *
 * This slice and the adjacent code_179d8_q (runner echo, the CD-ROM read
 * driver's module-level half) share callees LockCd (lock: gCdLock = 1) and
 * UnlockCd (unlock: gCdLock = 0), both defined in code_179d8_q.c. Declared
 * locally here, never in a shared header -- see CLAUDE.md on per-unit local
 * views.
 *
 * The slice implements the driver's small CD-read state machine plus its
 * two support structures:
 *   - gCdState holds the current phase: 1 = issue a CdlSetloc seek, 2 = poll
 *     CdSync for it, 7 = issue CdRead, 8 = poll CdReadSync; gCdTimeoutCounter
 *     is the busy-wait counter both tick functions bump while polling,
 *     cleared on every phase change by SetCdState.
 *   - TickCdStateMachine / TickCdLoadFileStateMachine are the two per-tick
 *     steps ServiceCdDriver (code_179d8_q) dispatches on gCdTickStep (1 / 2).
 *     TickCdStateMachine is the default, used by code_179d8_s.c's open,
 *     explicit-seek and straight-read call sites. TickCdLoadFileStateMachine
 *     is used exclusively by that unit's CdDriver__LoadFile (the
 *     CdDriver__RequestLoadFile worker, per its own report): on the
 *     "still busy" signal at phase 2 it proceeds straight into the read
 *     phase instead of resetting, and on a successful read it restores the
 *     caller's saved gCdSeekParam from gCdSavedSeekParam -- both differences
 *     specific to that one combined seek+read operation.
 *   - StartCdOperation / ResetCdStateMachine are the state machine's "start" /
 *     "reset" bookends -- exact mirror images of each other.
 *   - AllocCdRequestNode / FreeCdRequestNode allocate+link / unlink+free a
 *     request-queue node (CdRequestNode, 0x24 bytes), list head
 *     gCdRequestQueue -- the queue code_179d8_q.c's EnqueueCdRequest and
 *     CdDriver__CancelRequests drive from the other end.
 *   - FindCdFileEntry / FindCdFileIndex / GetCdFileEntry are linear-scan /
 *     index helpers over the file table (CdFileEntry, 0x1C bytes each) based
 *     at gFileTable, count gFileTableCount. That type, CdRequestNode and the
 *     module globals the three CD units share are declared once in
 *     include/CdDriver.h (track 4b, round 85).
 */

/* lock/unlock, defined in code_179d8_q.c (runner echo's unit). */
extern void LockCd(void);
extern void UnlockCd(void);

/* CdSync / CdReadSync mode: return the current status at once (0 waits).
 * CdReadSync then answers -1 for an error, 0 when the read is done, and
 * otherwise the sectors still to come. */
#define CD_SYNC_POLL 1

/* Polls of CdSync that answer CdlNoIntr before the seek is issued again. */
#define CD_WAIT_TIMEOUT 601

/* Psy-Q pool allocator, BMemPMgr.c. */
extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

/* libc2/strstr.o, linked (see code_179d8_h.c's carve notes). */
extern char *strstr(char *s1, char *s2);

extern s32 gCdTimeoutCounter; /* timeout counter */

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
            if (cur->next != NULL) {
                do {
                    cur = cur->next;
                } while (cur->next != NULL);
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
    CdFileEntry *entry;
    CdFileEntry *table;

    table = gFileTable;
    LockCd();
    entry = &table[index];
    UnlockCd();
    return entry;
}

/* forward decls -- both defined later in this unit; ROM order keeps the
 * definitions below. */
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

void StartCdOperation(s32 op, s32 state) {
    gCdBusy = 1;
    gCdOperation = op;
    gCdState = state;
    gCdIdle = 0;
    gCdRequestQueue->active = 1;
}

void ResetCdStateMachine(void) {
    gCdOperation = 0;
    gCdState = 0;
    gCdTickStep = 0;
    gCdIdle = 1;
    gCdTimeoutCounter = 0;
    gCdBusy = 0;
}

void SetCdState(s32 state) {
    gCdState = state;
    gCdTimeoutCounter = 0;
}
