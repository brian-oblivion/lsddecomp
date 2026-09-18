#include "common.h"

/*
 * code_179d8_r -- tail slice of the code_179d8 monolith, carved round 45.
 * Runner foxtrot. Cold ground: none of these 10 functions has ever been
 * attempted before this round.
 *
 * This slice and the adjacent code_179d8_q (runner echo) share callees
 * LockCd (lock: gCdLock = 1) and UnlockCd (unlock:
 * gCdLock = 0), both defined in code_179d8_q.c. Declared locally here,
 * never in a shared header -- see CLAUDE.md on per-unit local views.
 *
 * The slice implements a small CD-read state machine:
 *   - gCdState holds the current state/phase.
 *   - gCdTimeoutCounter is a timeout counter, reset by SetCdState.
 *   - TickCdStateMachine / TickCdLoadFileStateMachine are near-identical per-tick state
 *     machine steps (driven from ServiceCdDriver in code_179d8_q via
 *     gCdTickStep == 1 / == 2), differing only in their state==2 and
 *     state==8-success handling.
 *   - StartCdOperation / ResetCdStateMachine are the "start" / "reset" bookends of
 *     that state machine.
 *   - AllocCdRequestNode / FreeCdRequestNode are a generic doubly-linked-list
 *     append/remove+free pair over 0x24-byte nodes, list head gCdRequestQueue.
 *   - FindCdFileEntry / FindCdFileIndex / GetCdFileEntry are linear-scan /
 *     index helpers over a flat table of 0x1C-byte string records based at
 *     gFileTable, count gFileTableCount.
 */

/* lock/unlock, defined in code_179d8_q.c (runner echo's unit). */
extern void LockCd(void);
extern void UnlockCd(void);

/* Psy-Q CD-ROM library, linked from lib/libcd/sys.o. Locally typed the same
 * way code_179d8_h.c already types this unit's own call sites. */
extern s32 CdControlF(s32 com, void *param);
extern s32 CdSync(s32 mode, void *result);
extern s32 CdRead(s32 sectors, void *buf, s32 mode);
extern s32 CdReadSync(s32 mode, s32 result);
extern void CdFlush(void);

/* Psy-Q's own command code (include/psyq/LIBCD.H documents the command list
 * but not the numeric values; this one is confirmed the same way
 * code_179d8_q.c's CD_CMD_SETMODE is, by the sibling unit's independently
 * derived 0x0E == CdlSetmode matching the well-known Psy-Q CdlCommand
 * enumeration -- CdlSetloc is that enumeration's 3rd member, value 2).
 * Spelled locally rather than by including LIBCD.H, same rationale as
 * code_179d8_q.c: this unit's libcd declarations are deliberately
 * per-call-site. */
#define CD_CMD_SETLOC 2

/* Both state==2 branches below busy-wait this many ticks (~601 service-pump
 * calls) before giving up and retrying the command from state 1. */
#define CD_WAIT_TIMEOUT 0x259

/* Psy-Q pool allocator, code_8220.c. */
extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);

/* libc2/strstr.o, linked (see code_179d8_h.c's carve notes). */
extern char *strstr(char *s1, char *s2);

/* the CD request-queue node, 0x24 bytes; offsets 0x0/0x4/0x1C/0x20 are the
 * only ones this unit's two list functions touch. `active` matches the name
 * the sibling unit code_179d8_q.c's own independent view of this same
 * struct (CdRequest_D70) already gives this field: StartCdOperation sets it
 * on the head node when it starts an operation on it, AllocCdRequestNode
 * clears it at allocation. This is a unit-local typedef, so the rename is
 * local to this file (CLAUDE.md, multiple-independent-local-views); `unk4`
 * has no evidence anywhere and keeps its placeholder name. */
typedef struct CdRequestNode {
    /* 0x00 */ s32 active;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 pad8[0x14];
    /* 0x1C */ struct CdRequestNode *prev;
    /* 0x20 */ struct CdRequestNode *next;
} CdRequestNode; /* size 0x24 */

extern CdRequestNode *gCdRequestQueue; /* list head */

extern s32 gCdBusy; /* "busy" flag, 0/1 */
extern char *gFileTable; /* base of a table of 0x1C-byte string records */
extern s32 gFileTableCount;   /* record count */
extern s32 gCdIdle;   /* "idle"/"ready" flag, 0/1 */
extern s32 gCdOperation;   /* context value stashed by StartCdOperation */
extern s32 gCdState;   /* CD state-machine phase */
extern void *gCdSeekParam; /* CdControlF param pointer */
extern s32 gCdReadSectorCount;   /* CdRead sector count */
extern void *gCdReadBuffer; /* CdRead target buffer */
extern void *gCdSavedSeekParam; /* secondary pointer, used only by TickCdLoadFileStateMachine */
extern s32 gCdTickStep;   /* which state-machine step to tick, 1 or 2 */
extern s32 gCdTimeoutCounter;   /* timeout counter */

CdRequestNode *AllocCdRequestNode(void)
{
    CdRequestNode *node;
    CdRequestNode *head;
    CdRequestNode *cur;

    LockCd();
    node = func_80017B34(0x24);
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

void FreeCdRequestNode(CdRequestNode *node)
{
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
        func_80017CFC(node);
    }
    UnlockCd();
}

void *FindCdFileEntry(char *name)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur, name) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur += 0x1C;
    } while (i < gFileTableCount);
    return NULL;
}

s32 FindCdFileIndex(char *name)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur, name) == NULL) {
        i++;
        if (i >= gFileTableCount) {
            return -1;
        }
        cur += 0x1C;
    }
    UnlockCd();
    return i;
}

void *GetCdFileEntry(s32 index)
{
    void *result;
    char *base;

    base = gFileTable;
    LockCd();
    result = base + index * 0x1C;
    UnlockCd();
    return result;
}

/* forward decls -- both defined later in this unit; ROM order keeps the
 * definitions below. */
extern void ResetCdStateMachine(void);
extern void SetCdState(s32 state);

void TickCdStateMachine(void)
{
    s32 state;
    s32 v1;
    s32 newstate;

    LockCd();
    state = gCdState;

    if (state == 2)
        goto L_state2;
    if (state < 3) {
        if (state == 1)
            goto L_state1;
        goto L_end;
    }
    if (state == 7)
        goto L_state7;
    if (state == 8)
        goto L_state8;
    goto L_end;

L_state1:
    if (CdControlF(CD_CMD_SETLOC, (u8 *)gCdSeekParam + 0x14) == 0)
        goto L_end;
    newstate = 2;
    goto L_set;

L_state2:
    v1 = CdSync(1, NULL);
    if (v1 == state)
        goto L_reset;
    if (v1 < 3) {
        if (v1 == 0)
            goto L_count;
        goto L_end;
    }
    if (v1 != 5)
        goto L_end;
    newstate = 1;
    goto L_set;

L_count:
    gCdTimeoutCounter++;
    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(gCdReadSectorCount, gCdReadBuffer, 0x80) == 0)
        goto L_end;
    newstate = 8;
    goto L_set;

L_state8:
    v1 = CdReadSync(1, 0);
    if (v1 == -1)
        goto L_pending;
    if (v1 != 0)
        goto L_end;

L_reset:
    ResetCdStateMachine();
    goto L_end;

L_pending:
    CdFlush();
    newstate = 1;

L_set:
    SetCdState(newstate);

L_end:
    UnlockCd();
}

void TickCdLoadFileStateMachine(void)
{
    s32 state;
    s32 v1;
    s32 newstate;
    void *tmp;

    LockCd();
    state = gCdState;

    if (state == 2)
        goto L_state2;
    if (state < 3) {
        if (state == 1)
            goto L_state1;
        goto L_end;
    }
    if (state == 7)
        goto L_state7;
    if (state == 8)
        goto L_state8;
    goto L_end;

L_state1:
    if (CdControlF(CD_CMD_SETLOC, (u8 *)gCdSeekParam + 0x14) == 0)
        goto L_end;
    newstate = 2;
    goto L_set;

L_state2:
    v1 = CdSync(1, NULL);
    if (v1 == state)
        goto L_busy;
    if (v1 < 3) {
        if (v1 == 0)
            goto L_count;
        goto L_end;
    }
    newstate = 1;
    if (v1 == 5)
        goto L_set;
    goto L_end;

L_busy:
    newstate = 7;
    goto L_set;

L_count:
    gCdTimeoutCounter++;
    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(gCdReadSectorCount, gCdReadBuffer, 0x80) == 0)
        goto L_end;
    newstate = 8;
    goto L_set;

L_state8:
    v1 = CdReadSync(1, 0);
    if (v1 == -1) {
        newstate = 1;
        goto L_set;
    }
    if (v1 != 0)
        goto L_end;
    ResetCdStateMachine();
    tmp = gCdSavedSeekParam;
    gCdSavedSeekParam = NULL;
    gCdSeekParam = tmp;
    goto L_end;

L_set:
    SetCdState(newstate);

L_end:
    UnlockCd();
}

void StartCdOperation(s32 op, s32 state)
{
    gCdBusy = 1;
    gCdOperation = op;
    gCdState = state;
    gCdIdle = 0;
    gCdRequestQueue->active = 1;
}

void ResetCdStateMachine(void)
{
    gCdOperation = 0;
    gCdState = 0;
    gCdTickStep = 0;
    gCdIdle = 1;
    gCdTimeoutCounter = 0;
    gCdBusy = 0;
}

void SetCdState(s32 state)
{
    gCdState = state;
    gCdTimeoutCounter = 0;
}
