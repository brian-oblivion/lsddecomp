# TaskCore__OnPadPrev — MATCH (27/27 words)

> Renamed from `Obj86B60__func_8003C944` on 2026-09-25 (tools/rename.py). Address 0x8003c944.

> Renamed from `func_8003C944` on 2026-09-24 (tools/rename.py). Address 0x8003c944.

**Unit:** task · **Size:** 27 instructions

## What it does

```c
void TaskCore__OnPadPrev(Obj86B60 *self, s32 a1)
{
    void (*handler)(Obj86B60 *self);

    if (self->unk4C == NULL) {
        return;
    }
    if (self->unk3C == 1) {
        handler = self->methods->slotEC;
    } else if (self->unk3C == 2) {
        handler = self->methods->slot118;
    } else {
        return;
    }
    handler(self);
}
```

Same gate family, but now `self->unk3C` selects between TWO different
handler slots (`slotEC`/`slot118`, both external) rather than a reason-code
literal, and retail's own control flow funnels both into ONE shared call
site (`jalr` at a single address, fed by whichever branch ran).

## Residue and fix (1 wasted attempt, then matched)

**First attempt (`if (unk4C) { if (unk3C==1) slotEC(); else if (unk3C==2)
slot118(); }`, two separate `jalr` call sites inlined at each branch) scored
11/27 with `WARNING: differs OUTSIDE this range (243005 bytes)`** -- i.e. it
compiled one word LARGER than retail and shifted everything after it. Retail
does not duplicate the call; it computes a function-pointer VALUE in one of
two branches (or returns early on neither), then falls through to a SINGLE
shared `jalr`. **Fix: introduce an explicit local function-pointer variable
(`void (*handler)(Obj86B60*)`), assign it in each branch, `return` early on
the "neither" case, and call it once after the if-chain.** This is the same
"write the literal jump graph" family of lesson as
DECOMPILATION_LEARNINGS' cross-jump/tail-merge entry, but the direction here
is the opposite: the SOURCE already merges the calls (one call site), and
inlining the call into each branch is what desyncs it from retail, not a
merge retail is doing that the C fails to express.

## Struct knowledge established

- `Obj86B60Methods::slotEC` (+0x0EC, external `TaskCore__FindPrevFreeSlot`) and
  `::slot118` (+0x118, external `TaskCore__RetreatSlotCursor`) -- both `void (*)(Obj86B60*)`.

  **CORRECTED, round 12.** This line originally named `TaskCore__GetActiveItemCursor` as the
  occupant of `+0x118`. It is not; `TaskCore__GetActiveItemCursor` sits at `+0x120`. Resolved
  against the table bytes in the executable, twice independently (runner alpha
  while matching `TaskCore__GetActiveItemCursor`, then the head):

  ```sh
  python3 tools/classtable.py gTitleMenuMethods
  #   +0x118  0x8003DE30 TaskCore__RetreatSlotCursor
  #   +0x120  0x8003DFA0 TaskCore__GetActiveItemCursor
  ```

  `slotEC`'s attribution in the same line re-checked and is correct.

  **Why the error was invisible here.** `TaskCore__OnPadPrev` only ever *calls*
  through the slot and discards the result, so nothing about its own match
  depends on which function the slot holds — the match was byte-exact with the
  wrong name written down, and stayed byte-exact after the correction. A
  "struct knowledge established" entry naming a slot's OCCUPANT is a claim
  about DATA, and the compiled offset that the match actually verifies is a
  different claim. Only the second one gets checked by matching.

  Generalises to: **resolve a slot with `tools/classtable.py` before reusing
  another report's attribution**, exactly as CLAUDE.md says to do instead of
  counting. This is the first recorded instance of the failure it warns about,
  and it propagated one round before being caught.

### Proposed learning

When retail's disassembly shows a value (here, a vtable slot's function
pointer) computed on TWO OR MORE branches that all converge on one shared
call/use site, write that exact shape in C -- a local variable assigned per
branch, used once after the merge -- rather than inlining the call/use into
each branch even when the two forms are behaviorally identical. Inlining
cost a whole extra instruction (the duplicated `jalr`/setup) and triggered
address drift on the very first attempt here.

## Provenance

round 2026-09-02, runner echo, unit task. 2 attempts.

## Naming (round 78, delta)

**Tier C.** `func_8003C944` -> `Obj86B60__func_8003C944`. Message-0x12
handler (slot80, corrected occupant). Body: when `self->unk4C` is set, picks
`self->methods->slotEC` (if `unk3C==1`) or `self->methods->slot118` (if
`unk3C==2`) and calls it with no further arguments -- a mode-gated indirect
forward, exact same shape as `TaskCore__OnPadNext` below (differing only
in which slot pair it forwards to). No independent evidence of what either
target represents, so tier C.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__func_8003C944 (tools/rename.py). Occupant of +0x080 (`onPadPrev`, 0x12): findPrevFreeSlot (inputMode 1) or retreatSlotCursor (inputMode 2). The retreatSlotCursor slot is now void like its occupant, which removed this function's baseline "assignment from incompatible pointer type" warning. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

inputMode tests read TASKCORE_INPUT_CHOOSING_SLOT / TASKCORE_INPUT_SCROLLING; the single `handler` call site (the residue above) carries a MATCHING line. Byte-identical.
