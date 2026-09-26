# CdDriver__CancelRequests — MATCHED (62/62 words)

> Renamed from `Class6D4E8__CancelRequests` on 2026-09-26 (tools/rename.py). Address 0x80027d70.

> Renamed from `func_80027D70` on 2026-09-17 (tools/rename.py). Address 0x80027d70.

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. This class's
own method-table slot **+0x074** (see the class-map comment above
`GetCdDriverMethods` in the `.c`).

## Result

Byte-exact, third attempt (two intermediate near-misses, see below).

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
/* The queued-request node AllocCdRequestNode (code_179d8_r) allocates and
 * FreeCdRequestNode (code_179d8_r) unlinks and frees -- only the fields this
 * call site itself reads are typed here. `active` is the flag StartCdOperation
 * (code_179d8_r) sets on the head node when it starts an operation on it;
 * AllocCdRequestNode clears it at allocation. The list head is gCdRequestQueue. */
typedef struct CdRequest_D70 CdRequest_D70;
struct CdRequest_D70 {
    /* +0x00 */ s32 active;
    u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 owner;
    u8 pad10[0x20 - 0x10];
    /* +0x20 */ CdRequest_D70 *next;
};

typedef struct Obj6D4E8_D70 Obj6D4E8_D70;
struct Obj6D4E8_D70 {
    u8 pad00[0x22];
    /* +0x22 */ u16 pendingRequests;
    /* +0x24 */ s32 flags;
};

extern s32 gCdRequestQueue;
extern s32 gCdIdle;
extern s32 gCdSavedSeekParam;
extern s32 gCdSeekParam;
extern void CdFlush(void);
extern void ResetCdStateMachine(void); /* code_179d8_r: reset the state machine */
extern void FreeCdRequestNode(CdRequest_D70 *req); /* code_179d8_r: unlink+free */

void CdDriver__CancelRequests(Obj6D4E8_D70 *self)
{
    CdRequest_D70 *entry;
    CdRequest_D70 *node;
    CdRequest_D70 *next;
    s32 saved;

    LockCd();

    entry = (CdRequest_D70 *)gCdRequestQueue;

    if (entry != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (entry->owner == (s32)self && entry->active != 0 && gCdIdle == 0) {
            CdFlush();
            ResetCdStateMachine();
            saved = gCdSavedSeekParam;
            gCdSavedSeekParam = 0;
            gCdSeekParam = saved;
        }

        for (node = (CdRequest_D70 *)gCdRequestQueue; node != NULL; node = next) {
            next = node->next;
            if (node->owner == (s32)self) {
                FreeCdRequestNode(node);
                self->pendingRequests--;
            }
        }
    }

    UnlockCd();
}
```

## Derivation

Reads as "flush a pending CD queue's entries belonging to `self`, then
unlink and free them." Two guards gate the ENTIRE body (not just the
`CdFlush` block): the pending-list head (`gCdRequestQueue`) must be non-NULL and
`self->pendingRequests` (spelled `unk22` when this was written) must be non-zero — if either fails, the
function does nothing but the latch dance. Inside that, an inner
three-condition guard (head node's owner is `self`, head node's `active` flag (`unk00` when this was written)
flag is set, and `gCdIdle == 0`) triggers `CdFlush()` +
`ResetCdStateMachine()` (both cross-unit — `ResetCdStateMachine` from foxtrot's
`code_179d8_r`) and a load-clear-store handoff between `gCdSavedSeekParam` and
`gCdSeekParam` (needs an explicit temp: the store order is `gCdSavedSeekParam`
cleared BEFORE the old value lands in `gCdSeekParam`, not the natural-looking
`gCdSeekParam = gCdSavedSeekParam; gCdSavedSeekParam = 0;`, which would store in the
opposite order). Then, regardless of that inner guard, a loop walks the
whole list unlinking every node whose `owner == self` via
`FreeCdRequestNode` (also `code_179d8_r`) and decrementing `self->pendingRequests` per
node removed.

Two register-allocation traps, both giving clean length-matching near-misses
that still diffed:

1. **The list-walk loop must pre-fetch `next` before the removal call, as
   its own local, not `node = node->next` at the `for` update clause.**
   Writing the update as `node = node->next` reads `node->next` AFTER
   `FreeCdRequestNode(node)` may have freed/unlinked `node` — GCC still compiled
   it (nothing catches use-after-free at compile time), but it forced a
   `move $a0,$s0` to preserve `node` across the call for the (now
   mis-timed) later read, one extra word retail doesn't have. Retail
   snapshots `next = node->next;` as the loop's FIRST statement, before the
   owner check, letting `node` stay live in `$a0` — the same register the
   call needs it in — with no copy.
2. **The initial guard's list-head read and the loop's OWN list-head reread
   are different C locals, not the same variable reused.** Both statements
   are literally `X = gCdRequestQueue;`, and reusing a single `node` variable for
   both is semantically identical — but it made GCC keep the value in `$a0`
   for BOTH sites, while retail's actual register choice is `$v1` for the
   first (guard) read and `$a0` for the second (loop-entry) read. A second,
   separately-named local (`entry`) for the guard fixed it: with two
   independently-scoped locals, the allocator was free to pick the register
   assignment retail's compiler picked, rather than being pinned to one
   choice by variable identity.

### Proposed learning

**Two loads of the same global, in two different statements that never
share live range with each other, are not necessarily "the same variable"
even when semantically interchangeable — they can get different registers,
and forcing them into one C local can pin the allocator to the wrong one.**
This is the struct/global analogue of the do-while-loop-scratch idiom
already on file (round 44's `SceneNode__UpdateRotation`): when a near-miss is
length-correct but register-swapped only at sites that don't overlap in
lifetime, try splitting a reused local into two independently-named ones
before suspecting anything structural.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027D70` | `CdDriver__CancelRequests` | A |

**Evidence.** Slot `+0x074` of `gCdDriverMethods`. It walks the `gCdRequestQueue`
request list and, for every node whose `owner` is this object, calls
`FreeCdRequestNode` (code_179d8_r: unlink + free) and decrements the object's own
pending count. Before that, if the HEAD node is this object's and is already
`active` and the drive is not idle, it aborts the transfer in flight
(`CdFlush`, `ResetCdStateMachine` = reset the state machine) and restores the saved
position pointer. Cancelling this owner's outstanding requests is the whole
function; the `Cancel` is not an inference about purpose but a description of
`unlink + free + abort the one in flight`.

**Field names established here** (applied in this unit, which owns its views):

- `CdRequest_D70.active` (`+0x00`): `StartCdOperation` (code_179d8_r) sets the
  head node's `+0x00` to 1 when it starts an operation on it, and
  `AllocCdRequestNode` clears it at allocation. Tier B -- "an operation has been
  started on this node" is what the two writers show; whether it also means
  anything to the state machine's later steps is not established.
- `CdRequest_D70.owner` (`+0x0C`): `EnqueueCdRequest` stores the requesting
  object there. Tier A. (Already named `owner` in round 45; confirmed.)
- `Obj6D4E8_D70.pendingRequests` / `.flags`: see
  `CdDriver__RequestLoadFile.md`.

## Proposed field names

**APPLIED by the head at merge, round 52** -- all four fields, both types
and all five vtable slots below are now in the tree, each one applied
separately with `./build-and-verify.sh` green and byte-exact after it. One
mis-hit had to be resolved by receiver type: `src/code_179d8_h.c:143`
accesses `pendingGeneration` on a `Class6D430 *self`, while the same file's
lines 97/174/175/182 are its OWN `ObjA34_179D8H::unk0C` and were left alone.
The compiler named that mis-hit (`structure has no member named 'unk0C'`),
which is the procedure working in the direction where it can work.

| unit | type | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- | --- |
| code_179d8_r | `Node8008A894` | `unk0` | `active` | B | set by `StartCdOperation` on the head node at operation start, cleared at allocation |
| code_179d8_s | `Node8008A894` | `unkC` | `owner` | A | written by `EnqueueCdRequest` with the requesting object |

## Track 4b (2026-09-25, round 85)

The CD driver's shared globals and records are now declared once, in
`include/CdDriver.h`, and this body uses that one reading: the node is `CdRequestNode` (was the local `CdRequest_D70` view), `owner` is compared as a `struct Class6D4E8 *`, and the saved seek target is a `CdFileEntry *`. The
global's type comes from its accessors (`gFileTable` is walked at the 0x1C
`CdFileEntry` stride; `gCdSeekParam` is read for `->size` and sought to at
`+0x14`, i.e. `pos`). Byte-identical; no new `-Wall` warning.


Track 4, 2026-09-26 (round 88). The class of gCdDriverMethods (was D_8006D4E8, id 0x13 = DATASOURCE_CD) is CdDriver, in include/CdDriver.h: its ctor calls InitCdDrive, its slots enqueue CD_OP_* requests and drive the CD read state machine, and it is VabDriver's sibling. The object views this function was typed against are replaced by CdDriver, whose fields are all Class6D430's (the driver runs on its clients' objects; Class6D430's +0x018/+0x01C were named pos/size for it). Byte-identical. `Class6D4E8__CancelRequests` -> `CdDriver__CancelRequests` by rename.py.
