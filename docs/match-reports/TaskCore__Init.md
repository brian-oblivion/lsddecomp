# TaskCore__Init

> Renamed from `TaskCoreObj__func_8003C1DC` on 2026-09-25 (tools/rename.py). Address 0x8003c1dc.

> Renamed from `func_8003C1DC` on 2026-09-23 (tools/rename.py). Address 0x8003c1dc.

**Unit:** Task · **Size:** 23 instructions (0x5C bytes) · **Status:** MATCHED (23/23 words, whole-image SHA1 green), first attempt

## What it does

Forwards to a SECOND sibling class's table (`GetIntermediateBaseMethods()`, returns
`&gIntermediateBaseMethods`) at slot `+0x044`, passing `self` and both of its own
arguments unchanged, discards that call's return, then reads and returns
`self->unk38`. This function itself occupies `gTaskCoreMethods` slot `+0x044` --
i.e. it is the delegation TARGET that `StreamTask__Init` (`gStreamTaskMethods::slot44`)
calls into (see that report). One level further down the same chain:
`gStreamTaskMethods::slot44` (`StreamTask__Init`) -> `gTaskCoreMethods::slot44`
(`TaskCore__Init`, this function) -> `gIntermediateBaseMethods::slot44` (unnamed,
uncarved).

## Derivation

```
addu $s2, $a0, zero            ; s2 = self
addu $s0, $a1, zero            ; s0 = a1
jal  GetIntermediateBaseMethods
 addu $s1, $a2, zero            ; s1 = a2 (delay slot)
addu $a0, $s2, zero
addu $a1, $s0, zero
lw   $v0, 0x44($v0)
jalr $v0
 addu $a2, $s1, zero
lw   $v0, 0x38($s2)              ; v0 = self->unk38, AFTER the call returns
...epilogue, returns v0
```

```c
s32 TaskCore__Init(StreamTaskObj *self, s32 a1, s32 a2) {
    GetIntermediateBaseMethods()->slot44(self, a1, a2);
    return self->unk38;
}
```

Matched first attempt.

**Return-type discrepancy with `include/GameApplication.h`, flagged for the
head:** that header already types this exact slot (`gTaskCoreMethods::slot44`,
its own `LoaderTaskMethods::slot44`) as `void`, established from a
*different, discarding* caller's call sites in another unit. This function's
own disassembly, however, unambiguously reads `self->unk38` into `$v0`
*after* the inner call returns and *before* the epilogue -- a load with no
purpose except to be the return value (nothing else touches it). A truly
`void` source would not need this load at all. The two typings are not in
conflict at the ABI level (a caller that ignores the return through a
`void`-typed function pointer simply never reads `$v0`, which is exactly
what `GameApplication.h`'s own caller does), so `GameApplication.h`'s existing callers
are unaffected either way -- but the FUNCTION's true return type is `s32`,
and `GameApplication.h`'s `LoaderTaskMethods::slot44` typing looks incomplete now
that the occupant is known. Not fixed here (out of this unit's scope to edit
that header under the parallel-run rules); worth the head reconciling in
`GameApplication.h` directly, in a later round.

## New struct/header knowledge

Added `include/Task.h`'s `TaskUtilMethods` (this unit's own local view
of `gIntermediateBaseMethods`) with slot `+0x044` typed
`void (*)(StreamTaskObj *self, s32 a1, s32 a2)` (its own return is
discarded at this call site, so its true type is unconfirmed either way).
Named `StreamTaskObj::unk38`.

## Proposed learning

**The "byte match tells you nothing about return type" trap applies to an
INTERMEDIATE link in a delegation chain, not just to the outermost
wrapper.** This function's own body proves its return type is non-void
(`self->unk38` is read purely to become the return value) even though an
ALREADY-ESTABLISHED header from a different unit types the slot it occupies
as void -- that header's typing describes what ITS caller does with the
value (nothing), not what the callee itself computes. When a function this
unit needs to write already occupies a slot typed elsewhere, read the
occupant's OWN disassembly before trusting the existing slot typing at face
value; the existing typing can be right for its own call site and still
under-describe the function.

## Naming

**TaskCoreObj__func_8003C1DC** -- tier C. Occupies `gTaskCoreMethods` slot
`+0x044`; up-calls `IntermediateBase`'s own slot `+0x044`, then returns
`self->unk38` as a status/result word (the report's own "return-type
discrepancy" section). The mechanics are fully known (an up-call followed
by a status read), but neither `unk38` nor the base slot's own game meaning
is established, so left `Class__func_xxxxx` rather than name it as a getter
for something unconfirmed.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__func_8003C1DC (tools/rename.py). Occupant of +0x044 (`init`), named for the slot: IntermediateBase's init, then returns +0x038, now `result` (onInit clears it, setState(6) sets 1, StreamTaskObj sets 2). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
