# AllocCdRequestNode

> Renamed from `func_8002832C` on 2026-09-18 (tools/rename.py). Address 0x8002832c.

**Unit:** CdDriver · **Size:** 38 words · **Status:** MATCHED (38/38 words) · **Round 45**

## What it does

Allocates a 0x24-byte doubly-linked-list node (`BMemPMgrAlloc(0x24)`) and
appends it to the tail of the list rooted at `gCdRequestQueue`. New-node fields
at offset 0x0 and 0x4 are zeroed; 0x1C/0x20 are the prev/next links. If the
list is empty, the new node simply becomes the head; otherwise the function
walks `next` pointers to the current tail and links the new node on.
Never-attempted cold ground (fresh carve, round 45).

## The C

```c
typedef struct Node8008A894 {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ u8 pad8[0x14];
    /* 0x1C */ struct Node8008A894 *prev;
    /* 0x20 */ struct Node8008A894 *next;
} Node8008A894; /* size 0x24 */

extern Node8008A894 *gCdRequestQueue; /* list head */

Node8008A894 *AllocCdRequestNode(void)
{
    Node8008A894 *node;
    Node8008A894 *head;
    Node8008A894 *cur;

    LockCd();
    node = BMemPMgrAlloc(0x24);
    if (node != NULL) {
        head = gCdRequestQueue;
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
            gCdRequestQueue = node;
        }
    }
    UnlockCd();
    return node;
}
```

Closed on the first attempt, once the exact zero-init order was read
straight off the asm (all four fields zeroed unconditionally before the
list-empty branch, including `unk4` which sits in a branch delay slot and
is easy to misread as conditional).

### Proposed learning

`LockCd`/`UnlockCd` (defined in the sibling unit
`CdDriver.c`, runner echo) are a plain lock/unlock pair over a global
flag `sCdLock`. Every function in this slice's small CD-state-machine
and list-management group brackets its body with them; declare them
locally as `extern void LockCd(void); extern void
UnlockCd(void);` rather than sharing a header with the adjacent unit.

## Naming

**Tier A.** Allocates a 0x24-byte queue node (`BMemPMgrAlloc(0x24)`) and
appends it to the tail of `gCdRequestQueue`, clearing `active` (offset 0x00)
and `unk4` (offset 0x04). Named for exactly this mechanics -- a pure
alloc+link leaf, tier A by the "mechanics ARE its purpose" rule. Corroborated
independently by CdDriver.c's own comment on this function (written
before this rename, referring to it by address): "func_8002832C allocates
one and links it onto D_8008A894" / "func_8002832C clears it [active] at
allocation".

## Round 101 (track 7 polish)

- Allocates `sizeof(CdRequestNode)` (0x24) instead of the literal.
- The tail walk is a plain `while (cur->next != NULL)`: the `if` +
  `do`/`while` it was written as is the loop inversion GCC does itself,
  and the bytes do not move.
- `unk4` is still `unk4`: its only other accessor is
  `CdDriver__RunRequestQueue` (CdDriver.c), which ORs bit 0 into the
  owner's `flags` when it is nonzero, and nothing in the image writes it
  nonzero. Left for the head (see the round's summary).

## History (from code_179d8_r.c's comments, moved round 101)

The unit banner this pass replaced read, in substance:

- code_179d8_r was the tail slice of the code_179d8 monolith, carved round
  45; all ten functions were matched C from that carve (rounds 45/46,
  runner foxtrot), and it was named in track 3, round 53, by runner alpha,
  without changing a byte.
- LockCd (`gCdLock = 1`) and UnlockCd (`gCdLock = 0`) are defined in
  code_179d8_q.c (runner echo's unit then) and were declared locally here
  on purpose, never in a shared header, per CLAUDE.md's per-unit local
  views.
- CdFileEntry, CdRequestNode and the module globals the three CD units
  share were moved into include/CdDriver.h in track 4b, round 85.
- The comment on BMemPMgrAlloc/BMemPMgrFree called them "Psy-Q pool
  allocator, BMemPMgr.c"; they are the game's own, src/BMemPMgr.c.
- strstr was declared locally as "libc2/strstr.o, linked (see
  code_179d8_h.c's carve notes)"; round 101 takes it from Sony's
  `<strings.h>`.
