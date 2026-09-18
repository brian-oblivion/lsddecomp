> Renamed from `func_80027D70` on 2026-09-17 (tools/rename.py). Address 0x80027d70.

# Class6D4E8__CancelRequests — MATCHED (62/62 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. This class's
own method-table slot **+0x074** (see the class-map comment above
`GetClass6D4E8Methods` in the `.c`).

## Result

Byte-exact, third attempt (two intermediate near-misses, see below).

The body below is the round-51 source, after track-3 naming. The
derivation notes that follow were written in round 45 against the same
code under its `unk` names; only names changed, the image is
byte-identical, and the `## Naming` section at the end of this report
carries the evidence for each one.

```c
/* The queued-request node func_8002832C (code_179d8_r) allocates and
 * func_800283C4 (code_179d8_r) unlinks and frees -- only the fields this
 * call site itself reads are typed here. `active` is the flag func_80028844
 * (code_179d8_r) sets on the head node when it starts an operation on it;
 * func_8002832C clears it at allocation. The list head is D_8008A894. */
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

extern s32 D_8008A894;
extern s32 gCdIdle;
extern s32 D_8008A888;
extern s32 D_8008A87C;
extern void CdFlush(void);
extern void func_80028864(void); /* code_179d8_r: reset the state machine */
extern void func_800283C4(CdRequest_D70 *req); /* code_179d8_r: unlink+free */

void Class6D4E8__CancelRequests(Obj6D4E8_D70 *self)
{
    CdRequest_D70 *entry;
    CdRequest_D70 *node;
    CdRequest_D70 *next;
    s32 saved;

    LockCd();

    entry = (CdRequest_D70 *)D_8008A894;

    if (entry != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (entry->owner == (s32)self && entry->active != 0 && gCdIdle == 0) {
            CdFlush();
            func_80028864();
            saved = D_8008A888;
            D_8008A888 = 0;
            D_8008A87C = saved;
        }

        for (node = (CdRequest_D70 *)D_8008A894; node != NULL; node = next) {
            next = node->next;
            if (node->owner == (s32)self) {
                func_800283C4(node);
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
`CdFlush` block): the pending-list head (`D_8008A894`) must be non-NULL and
`self->pendingRequests` (spelled `unk22` when this was written) must be non-zero — if either fails, the
function does nothing but the latch dance. Inside that, an inner
three-condition guard (head node's owner is `self`, head node's `active` flag (`unk00` when this was written)
flag is set, and `gCdIdle == 0`) triggers `CdFlush()` +
`func_80028864()` (both cross-unit — `func_80028864` from foxtrot's
`code_179d8_r`) and a load-clear-store handoff between `D_8008A888` and
`D_8008A87C` (needs an explicit temp: the store order is `D_8008A888`
cleared BEFORE the old value lands in `D_8008A87C`, not the natural-looking
`D_8008A87C = D_8008A888; D_8008A888 = 0;`, which would store in the
opposite order). Then, regardless of that inner guard, a loop walks the
whole list unlinking every node whose `owner == self` via
`func_800283C4` (also `code_179d8_r`) and decrementing `self->pendingRequests` per
node removed.

Two register-allocation traps, both giving clean length-matching near-misses
that still diffed:

1. **The list-walk loop must pre-fetch `next` before the removal call, as
   its own local, not `node = node->next` at the `for` update clause.**
   Writing the update as `node = node->next` reads `node->next` AFTER
   `func_800283C4(node)` may have freed/unlinked `node` — GCC still compiled
   it (nothing catches use-after-free at compile time), but it forced a
   `move $a0,$s0` to preserve `node` across the call for the (now
   mis-timed) later read, one extra word retail doesn't have. Retail
   snapshots `next = node->next;` as the loop's FIRST statement, before the
   owner check, letting `node` stay live in `$a0` — the same register the
   call needs it in — with no copy.
2. **The initial guard's list-head read and the loop's OWN list-head reread
   are different C locals, not the same variable reused.** Both statements
   are literally `X = D_8008A894;`, and reusing a single `node` variable for
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
already on file (round 44's `func_8001CEB4`): when a near-miss is
length-correct but register-swapped only at sites that don't overlap in
lifetime, try splitting a reused local into two independently-named ones
before suspecting anything structural.

## Naming

Round 51 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027D70` | `Class6D4E8__CancelRequests` | A |

**Evidence.** Slot `+0x074` of `D_8006D4E8`. It walks the `D_8008A894`
request list and, for every node whose `owner` is this object, calls
`func_800283C4` (code_179d8_r: unlink + free) and decrements the object's own
pending count. Before that, if the HEAD node is this object's and is already
`active` and the drive is not idle, it aborts the transfer in flight
(`CdFlush`, `func_80028864` = reset the state machine) and restores the saved
position pointer. Cancelling this owner's outstanding requests is the whole
function; the `Cancel` is not an inference about purpose but a description of
`unlink + free + abort the one in flight`.

**Field names established here** (applied in this unit, which owns its views):

- `CdRequest_D70.active` (`+0x00`): `func_80028844` (code_179d8_r) sets the
  head node's `+0x00` to 1 when it starts an operation on it, and
  `func_8002832C` clears it at allocation. Tier B -- "an operation has been
  started on this node" is what the two writers show; whether it also means
  anything to the state machine's later steps is not established.
- `CdRequest_D70.owner` (`+0x0C`): `EnqueueCdRequest` stores the requesting
  object there. Tier A. (Already named `owner` in round 45; confirmed.)
- `Obj6D4E8_D70.pendingRequests` / `.flags`: see
  `Class6D4E8__RequestLoadFile.md`.

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
| code_179d8_r | `Node8008A894` | `unk0` | `active` | B | set by `func_80028844` on the head node at operation start, cleared at allocation |
| code_179d8_s | `Node8008A894` | `unkC` | `owner` | A | written by `EnqueueCdRequest` with the requesting object |
