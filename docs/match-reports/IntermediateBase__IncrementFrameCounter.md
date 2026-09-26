# IntermediateBase__IncrementFrameCounter — MATCH (5/5 words)

> Renamed from `Obj86B60__IncrementFrameCounter` on 2026-09-25 (tools/rename.py). Address 0x8003e4a4.

> Renamed from `func_8003E4A4` on 2026-09-19 (tools/rename.py). Address 0x8003e4a4.

**Unit:** code_2cc8c_c · **Size:** 5 instructions

## What it does

Increments `Obj86B60::unk1C` (already documented). Retail's own delay-slot
fill (`sw` in the `jr $ra` delay slot) happens to leave the incremented
value in `$v0` at return, but that is a side effect of scheduling, not
evidence of a non-`void` return -- no caller was found that reads it, so
this is written as a plain `void` increment. Both a `void` and an
`s32`-returning `return self->unk1C;` form compile identically here (the
value is already live in `$v0` either way); `void` was chosen as the
simpler, unforced reading.

## The C

```c
void IntermediateBase__IncrementFrameCounter(Obj86B60 *self)
{
    self->unk1C++;
}
```

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**IntermediateBase__IncrementFrameCounter** (renamed from `func_8003E4A4`, round 55,
runner alpha). Tier A: a pure leaf increment (`self->unk1C++`) -- tier A by
the same "mechanics ARE the purpose" rule as a getter/clamp/list-push.
"FrameCounter" reuses the already-established cross-function reading of
`unk1C` ("a running count/frame value multiplied against unk84",
`TaskCore__TickColorFade`, code_2cc8c.c) rather than inventing a new one; `unk1C`
itself is PROPOSED for rename to `frameCounter` in this unit's
`## Proposed field names` (shared with code_2cc8c.c).

## Proposed field names

- `Obj86B60::unk1C` -> `frameCounter` (tier B). Mechanics established across
  three independent sources: incremented here unconditionally
  (`IntermediateBase__IncrementFrameCounter`), zeroed on state-reset paths
  (`IntermediateBase__ResetCounters`, `IntermediateBase__OnState2`,
  `IntermediateBase__OnState3`, and `TaskCore__SetState` in `code_2cc8c.c` on
  several message codes), and consumed as a multiplier in `TaskCore__TickColorFade`
  (code_2cc8c.c) against `unk84` -- consistent with a per-instance
  frame/tick counter. What in-game effect the resulting product drives is
  NOT established, hence tier B. NOT renamed directly: shared with
  `code_2cc8c.c` (`TaskCore__SetState`, `TaskCore__TickColorFade`, and likely
  `TaskCore__SetFadeRate`/`TaskCore__TickFadeCallback`'s own callers of `self->unk1C`). Head
  applies by type scope.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__IncrementFrameCounter (class prefix; tier A, the body is the name). Occupies +0x05C, slot `update`: OnNotify's case for a sender of root class 5 (FrameClock), as SceneNode names that slot. Overrides: TaskCore__Update, Class86668__CheckTimeout, ObjM__Update, GraphRoom__Update; the first two call this base first.
