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
 *   - D_8008A8A0 is a timeout counter, reset by func_80028888.
 *   - func_8002858C / func_800286E4 are near-identical per-tick state
 *     machine steps (driven from ServiceCdDriver in code_179d8_q via
 *     D_8008A898 == 1 / == 2), differing only in their state==2 and
 *     state==8-success handling.
 *   - func_80028844 / func_80028864 are the "start" / "reset" bookends of
 *     that state machine.
 *   - AllocCdRequestNode / FreeCdRequestNode are a generic doubly-linked-list
 *     append/remove+free pair over 0x24-byte nodes, list head D_8008A894.
 *   - func_80028448 / func_800284C4 / func_80028540 are linear-scan /
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

/* Psy-Q pool allocator, code_8220.c. */
extern void *func_80017B34(s32 size);
extern void *func_80017CFC(void *ptr);

/* libc2/strstr.o, linked (see code_179d8_h.c's carve notes). */
extern char *strstr(char *s1, char *s2);

/* generic doubly-linked-list node, 0x24 bytes; offsets 0x0/0x4/0x1C/0x20 are
 * the only ones this unit's two list functions touch. */
typedef struct Node8008A894 {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 pad8[0x14];
    /* 0x1C */ struct Node8008A894 *prev;
    /* 0x20 */ struct Node8008A894 *next;
} Node8008A894; /* size 0x24 */

extern Node8008A894 *D_8008A894; /* list head */

extern s32 gCdBusy; /* "busy" flag, 0/1 */
extern char *gFileTable; /* base of a table of 0x1C-byte string records */
extern s32 gFileTableCount;   /* record count */
extern s32 gCdIdle;   /* "idle"/"ready" flag, 0/1 */
extern s32 gCdOperation;   /* context value stashed by func_80028844 */
extern s32 gCdState;   /* CD state-machine phase */
extern void *D_8008A87C; /* CdControlF param pointer */
extern s32 D_8008A880;   /* CdRead sector count */
extern void *D_8008A884; /* CdRead target buffer */
extern void *D_8008A888; /* secondary pointer, used only by func_800286E4 */
extern s32 D_8008A898;   /* which state-machine step to tick, 1 or 2 */
extern s32 D_8008A8A0;   /* timeout counter */

Node8008A894 *AllocCdRequestNode(void)
{
    Node8008A894 *node;
    Node8008A894 *head;
    Node8008A894 *cur;

    LockCd();
    node = func_80017B34(0x24);
    if (node != NULL) {
        head = D_8008A894;
        node->prev = NULL;
        node->next = NULL;
        node->unk0 = 0;
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
            D_8008A894 = node;
        }
    }
    UnlockCd();
    return node;
}

void FreeCdRequestNode(Node8008A894 *node)
{
    Node8008A894 *prev;
    Node8008A894 *next;

    LockCd();
    if (node != NULL) {
        prev = node->prev;
        if (prev != NULL) {
            prev->next = node->next;
        } else {
            D_8008A894 = node->next;
        }
        next = node->next;
        if (next != NULL) {
            next->prev = node->prev;
        }
        func_80017CFC(node);
    }
    UnlockCd();
}

void *func_80028448(char *arg0)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur, arg0) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur += 0x1C;
    } while (i < gFileTableCount);
    return NULL;
}

s32 func_800284C4(char *arg0)
{
    char *cur = gFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur, arg0) == NULL) {
        i++;
        if (i >= gFileTableCount) {
            return -1;
        }
        cur += 0x1C;
    }
    UnlockCd();
    return i;
}

void *func_80028540(s32 index)
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
extern void func_80028864(void);
extern void func_80028888(s32 arg0);

void func_8002858C(void)
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
    if (CdControlF(2, (u8 *)D_8008A87C + 0x14) == 0)
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
    D_8008A8A0++;
    if (D_8008A8A0 < 0x259)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(D_8008A880, D_8008A884, 0x80) == 0)
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
    func_80028864();
    goto L_end;

L_pending:
    CdFlush();
    newstate = 1;

L_set:
    func_80028888(newstate);

L_end:
    UnlockCd();
}

void func_800286E4(void)
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
    if (CdControlF(2, (u8 *)D_8008A87C + 0x14) == 0)
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
    D_8008A8A0++;
    if (D_8008A8A0 < 0x259)
        goto L_end;
    newstate = 1;
    goto L_set;

L_state7:
    if (CdRead(D_8008A880, D_8008A884, 0x80) == 0)
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
    func_80028864();
    tmp = D_8008A888;
    D_8008A888 = NULL;
    D_8008A87C = tmp;
    goto L_end;

L_set:
    func_80028888(newstate);

L_end:
    UnlockCd();
}

void func_80028844(s32 arg0, s32 arg1)
{
    gCdBusy = 1;
    gCdOperation = arg0;
    gCdState = arg1;
    gCdIdle = 0;
    D_8008A894->unk0 = 1;
}

void func_80028864(void)
{
    gCdOperation = 0;
    gCdState = 0;
    D_8008A898 = 0;
    gCdIdle = 1;
    D_8008A8A0 = 0;
    gCdBusy = 0;
}

void func_80028888(s32 arg0)
{
    gCdState = arg0;
    D_8008A8A0 = 0;
}
