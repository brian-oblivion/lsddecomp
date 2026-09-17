> Renamed from `func_80027D70` on 2026-09-17 (tools/rename.py). Address 0x80027d70.

# Class6D4E8__CancelRequests — MATCHED (62/62 words)

Round 45, runner echo (second sitting), `src/code_179d8_q.c`. This class's
own method-table slot **+0x074** (see the class-map comment above
`GetClass6D4E8Methods` in the `.c`).

## Result

Byte-exact, third attempt (two intermediate near-misses, see below).

```c
/* The pending-list node type func_8002832C (code_179d8_r) allocates and
 * func_800283C4 (code_179d8_r) unlinks/frees -- only the fields this call
 * site itself reads are typed here. */
typedef struct QueueEntryD70 QueueEntryD70;
struct QueueEntryD70 {
    /* +0x00 */ s32 unk00;
    u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 owner;
    u8 pad10[0x20 - 0x10];
    /* +0x20 */ QueueEntryD70 *next;
};

typedef struct SelfD70 SelfD70;
struct SelfD70 {
    u8 pad00[0x22];
    /* +0x22 */ u16 unk22;
    /* +0x24 */ s32 unk24;
};

extern s32 D_8008A894;
extern s32 gCdIdle;
extern s32 D_8008A888;
extern s32 D_8008A87C;
extern void CdFlush(void);
extern void func_80028864(void); /* code_179d8_r */
extern void func_800283C4(QueueEntryD70 *arg0); /* code_179d8_r */

void Class6D4E8__CancelRequests(SelfD70 *self)
{
    QueueEntryD70 *entry;
    QueueEntryD70 *node;
    QueueEntryD70 *next;
    s32 saved;

    LockCd();

    entry = (QueueEntryD70 *)D_8008A894;

    if (entry != NULL && self->unk22 != 0) {
        self->unk24 = 0;

        if (entry->owner == (s32)self && entry->unk00 != 0 && gCdIdle == 0) {
            CdFlush();
            func_80028864();
            saved = D_8008A888;
            D_8008A888 = 0;
            D_8008A87C = saved;
        }

        for (node = (QueueEntryD70 *)D_8008A894; node != NULL; node = next) {
            next = node->next;
            if (node->owner == (s32)self) {
                func_800283C4(node);
                self->unk22--;
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
`self->unk22` (a pending-count) must be non-zero — if either fails, the
function does nothing but the latch dance. Inside that, an inner
three-condition guard (head node's owner is `self`, head node's `unk00`
flag is set, and `gCdIdle == 0`) triggers `CdFlush()` +
`func_80028864()` (both cross-unit — `func_80028864` from foxtrot's
`code_179d8_r`) and a load-clear-store handoff between `D_8008A888` and
`D_8008A87C` (needs an explicit temp: the store order is `D_8008A888`
cleared BEFORE the old value lands in `D_8008A87C`, not the natural-looking
`D_8008A87C = D_8008A888; D_8008A888 = 0;`, which would store in the
opposite order). Then, regardless of that inner guard, a loop walks the
whole list unlinking every node whose `owner == self` via
`func_800283C4` (also `code_179d8_r`) and decrementing `self->unk22` per
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
