# StreamTask__OnInit

> Renamed from `StreamTaskObj__func_8003BAB4` on 2026-09-26 (tools/rename.py). Address 0x8003bab4.

> Renamed from `func_8003BAB4` on 2026-09-23 (tools/rename.py). Address 0x8003bab4.

**Unit:** task · **Size:** 42 words · **Status:** MATCHED (42/42)

## Summary

```c
void StreamTask__OnInit(StreamTaskObj *self) {
    GetTaskCoreMethods()->slot4C(self);
    self->unkA4 = 0;
    self->unkB4->methods->slot6C(self->unkB4, self->unkC0);
    if (self->unkB4->methods->slot40(self->unkB4, self->unkB8, self->unkBC, self->unkC4, self->unkC8) != 0) {
        self->methods->slot6C(self, 0);
    }
}
```

## Evidence

- `GetTaskCoreMethods()->slot4C`: `TaskCoreMethods` slot `+0x04C`. `classtable.py
  gTaskCoreMethods` shows it occupied by `TaskCore__OnInit`, this unit's own (still
  queued, larger) function — confirms existence/arity, result discarded here
  so typed `void`.
- `self->unkB4->methods->slot6C(self->unkB4, self->unkC0)`: two-argument
  forward on `StreamTaskUnkB4Methods`, return discarded, typed `void`.
- `self->unkB4->methods->slot40(...)`: five-argument call (4 register args
  plus one stack-spilled 5th, `self->unkC8`), **return value tested directly
  by the following `beqz`, never stored anywhere** — typed `s32`.
- `self->methods->slot6C(self, 0)`: reuses the slot established to be
  `StreamTask__SetFrameBound`'s occupied slot (`+0x06C` on `gStreamTaskMethods`), called only
  when `slot40`'s result is nonzero.
- New field `self->unkA4` (`+0x0A4`, `s32`): reset to 0 unconditionally here;
  a different function (`StreamTask__Update`, this same round) both reads it and
  assigns a call result to it — this function only zeroes it.

## Pitfall hit and corrected

First attempt wrongly assumed the `slot40` call's return was stored into
`self->unkA4` before the test (`self->unkA4 = ...; if (self->unkA4 != 0)`),
by analogy with a structurally similar pattern in `StreamTask__Update` seen while
reading ahead in the same unit. That produced a **41/42-ish residue**: retail
computes the branch's argument-setup (`move a0, s0` — preparing the call
inside the `if`-body) *in the branch's own delay slot*, which only happens
when the value tested is not first materialized into a store. Rereading the
raw disassembly showed there is in fact **no `sw` of the call result at all**
in this function — the `beqz` tests `$v0` straight off the `jalr`. Removing
the spurious assignment and testing the call expression directly fixed both
residue words at once.

## Proposed learning

**Do not backfill a struct-field write into a call site by analogy with a
different, structurally similar function in the same unit — reread that
function's own disassembly line by line first.** Two calls to the same
vtable slot (`slot40` here) can differ in whether the caller *stores* the
return value at all; only one of `StreamTask__OnInit`/`StreamTask__Update` does. The
tell, from the delay-slot-scheduling angle already documented for `if`-body
argument setup: if retail schedules an unconditional value (like the callee's
`self` argument) into the guarding branch's own delay slot, the source is not
storing the tested value to a struct field — it is testing a live temporary
directly.

## Naming

**StreamTask__OnInit** -- tier C (class known, purpose not
established). Occupies `gStreamTaskMethods` slot `+0x04C`; up-calls
`TaskCore__OnInit` at the same slot, then forwards the fields
`StreamTask__Init` set into the private `unkB4` sub-object and
conditionally resets a value. No external caller was found (dispatched only
through the vtable), and no single verb for the combined effect is
confidently supported by the body alone -- kept `Class__func_xxxxx` rather
than guess.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green). The onInit up-call casts the slot to `void (*)(TaskCore *)`: IntermediateBase types onInit (self, s32, s32, s32) from init's call, and this call passes self alone.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BAB4. Occupies +0x04C onInit and up-calls TaskCore's first, then MoviePlayer__SetAutoPlay(autoPlay) and MoviePlayer__Play(streamName, streamGroup, unkC4, loopCount); a nonzero Play result is setFrameBound(0).

## Track 4 (2026-09-26, round 89)

The player is a MoviePlayer (`include/MoviePlayer.h`); task.h's StreamTaskUnkB4Obj view is gone and task.c's `PLAYER()` casts `player` (still `BasicClass *` in StreamTask.h) to `MoviePlayer *`. The calls are `setAutoPlay` (+0x06C) and `play` (+0x040), `play`'s name argument cast `(char *)self->streamName` (no code). Byte-identical.

## Track 10 (2026-09-28, round 104, alpha)

`StreamTask::streamName`, StreamTask__Init's parameter and StreamTaskInitFn's are `const char *` (were `s32`): every caller passes a path (GetAsmkMovie's string or a FilePathRecord), so the five `(s32)` casts in GameApplicationFileResource.c are gone. Byte-identical.
