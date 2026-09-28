# TaskObjF__CardInfoStatus — MATCH (48/48 words)

> Renamed from `func_8004E7D0` on 2026-09-24 (tools/rename.py). Address 0x8004e7d0.

**Unit:** title_menu (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__CardInfoStatus(Node3bb8cE *self, s32 *p1, s32 *p2)`. Zeroes both
out-parameters, calls `TaskObjF__TestEvents(self)` (a still-INCLUDE_ASM helper in
`title_menu.c`), busy-waits on `func_80050B18(self->unk10)` until it
returns nonzero, then reads a status code from `TaskObjF__WaitForReadyEvent(self)` and
maps it to a return value plus the two out-parameters: `0x100` -> success
already (status 0, no out-param write); `0x8000` -> status 0, `*p1 = 1`;
`0x2000` -> `*p2 = 1` and a follow-up `_card_clear(self->unk10)` call.
Anything else leaves the default `status = 1` (failure) untouched.

## Where it stood, and the fix

Reached 46/48 immediately with the natural transcription
(`*p1 = 0; TaskObjF__TestEvents(self); *p2 = 0;`) — every value, branch target and
call was already right; the only residue was the two zero-stores and the
call being in the wrong relative order (zero address drift, confirmed via
`asm-differ`): retail stores `*p1` (ordinary instruction), THEN calls
`TaskObjF__TestEvents`, using `*p2`'s store as the call's delay-slot filler. My
compile instead hoisted the call one slot earlier, using `*p1`'s store as
the filler, and left `*p2`'s store as an ordinary instruction afterward —
the same two instructions, transposed.

**Neither source-order swap nor a bare `__asm__("")` barrier fixed it**
(the barrier inserted an extra `nop` and cascaded ~176KB of address
drift — this residue was never barrier-shaped). What worked, found via
the permuter (`tools/setup-permuter.sh`, zero score at iteration 80 of a
few thousand): write both zero-stores as ONE chained-assignment
expression **before** the call, in the specific order that matches
retail's own register content (`p2`'s store first in the C, `p1`'s store
second, even though `p1`'s store is the one that ends up physically
FIRST in the compiled output):

```c
s32 TaskObjF__CardInfoStatus(Node3bb8cE *self, s32 *p1, s32 *p2)
{
    s32 status;
    s32 code;

    status = 1;
    *p2 = *p1 = 0;
    TaskObjF__TestEvents(self);
    while (func_80050B18(self->unk10) == 0)
        ;
    code = TaskObjF__WaitForReadyEvent(self);
    if (code == 0x100) {
        status = 0;
    } else if (code == 0x8000) {
        status = 0;
        *p1 = 1;
    } else if (code == 0x2000) {
        *p2 = 1;
        _card_clear(self->unk10);
    }
    return status;
}
```

The permuter's own zero-scoring form was `*p2 = (*p1 = 0);`, textually
identical to the chained-assignment idiom above (C guarantees the same
evaluation order for both spellings); the chained form was used as the
more idiomatic rendering and re-verified independently (48/48, `build
exit=0`).

## What was tried and rejected

- Two separate statements, `*p1 = 0;` before the call and `*p2 = 0;`
  after (either literal order) — always reproduced the SAME 46/48
  residue or worse (swapping the statement order regressed to 45/48 with
  an extra spurious word, not just a reorder). GCC 2.6.3 consistently
  hoisted the call to absorb whichever store came immediately before it
  into the delay slot, rather than leaving a preceding store as an
  ordinary instruction and filling the delay slot from what follows.
- A bare `__asm__("")` immediately after `*p1 = 0;` — regressed hard
  (11/48, ~176KB of outside-range drift): an extra `nop` appeared,
  proving this residue is NOT an order-only case a barrier can fix at
  zero cost.
- Declaring `TaskObjF__TestEvents` as taking no arguments (`void(void)`) versus
  its real signature (it tail-calls `TaskObjF__ForEachEvent` with `self` forwarded
  through an untouched `$a0`, so it genuinely takes `self`) — no effect
  either way, since `$a0` already holds `self` at that point regardless.

### Proposed learning

**When two independent stores flank a void call and only their MERGED
ORDER (not their individual positions) is wrong, try a single chained
assignment expression (`*a = *b = value;`) instead of two statements.**
Two separate statements gave GCC 2.6.3 the freedom to hoist the call
across whichever store came first, absorbing it into the delay slot and
always producing the WRONG one of the two possible orderings regardless
of which statement was written first manually. Folding both stores into
one expression removed that freedom entirely, forcing both to be emitted
consecutively before the call — and the compound expression's own
left-to-right evaluation order (rightmost sub-assignment evaluates
first) decided which store ends up physically first, counter-intuitively
requiring the OUTER write (`*p2 = ...`) to be the one whose VALUE
argument physically stores earlier. Found by the permuter after ~6 failed
manual attempts; confirm via the permuter before manually iterating
further on this residue class.

## Naming (round 78, track 3)

`func_8004E7D0` -> `TaskObjF__CardInfoStatus`. **Tier B.** Private helper called only by `TaskObjF__CardInfoAndLoadStatus`. Polls `_card_info(self->cardHandle)` until ready, waits for the resulting event (`TaskObjF__WaitForReadyEvent`), and decodes the status code into two output flags, clearing the card (`_card_clear`) on code 0x2000. Named for the BIOS call it wraps; the status codes' game-level meaning is not established.

## Constants (round 98, track 7)

The answers `TaskObjF__WaitForReadyEvent` returns are `sCardEventSpecs`
entries, i.e. `<kernel.h>` event specs: 0x100 `EvSpTIMOUT` (no card
answered), 0x8000 `EvSpERROR`, 0x2000 `EvSpNEW` (for `_card_info` a newly
inserted card, which is then `_card_clear`ed; for `_card_load` an
unformatted one). The BIOS calls come from `<kernel.h>` instead of local
externs. Zero bytes changed.
