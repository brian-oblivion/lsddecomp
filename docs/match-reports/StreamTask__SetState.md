# StreamTask__SetState

> Renamed from `StreamTaskObj__func_8003BC14` on 2026-09-26 (tools/rename.py). Address 0x8003bc14.

> Renamed from `func_8003BC14` on 2026-09-23 (tools/rename.py). Address 0x8003bc14.

**Unit:** Task · **Size:** 56 words · **Status:** MATCHED (56/56)

## Summary

A four-case `switch` on the second parameter, cases laid out in ascending
numeric order in the source even though the compiler's own comparison tree
tests them out of order (`7`, then `<8`, then `5`, then `8`, then `0x12` —
classic binary-search pivot, matches the already-documented
"switch case order != comparison order" idiom from earlier rounds).

```c
void StreamTask__SetState(StreamTaskObj *self, s32 a1) {
    GetTaskCoreMethods()->slot60(self, a1);
    switch (a1) {
    case 5:
        self->unkD8 = 0;
        break;
    case 7:
        self->unkD8 = 1;
        break;
    case 8:
        if (self->unkD4 == 0) {
            self->unkB4->methods->slot4C(self->unkB4);
        }
        break;
    case 0x12:
        self->methods->slot94(self);
        break;
    }
}
```

## Evidence

- `GetTaskCoreMethods()->slot60(self, a1)`: new `TaskCoreMethods` slot `+0x060`.
  `classtable.py gTaskCoreMethods` confirms it's occupied (`TaskCore__SetState`, a
  different unit) — result discarded, typed `void`.
- `self->methods->slot94(self)`: new `StreamTaskObjMethods` slot `+0x094`.
  `classtable.py gStreamTaskMethods` shows it occupied by **this unit's own,
  already-matched `StreamTask__Exit`** — `void StreamTask__Exit(StreamTaskObj
  *self)`, single argument, which is exactly the arity the disassembly here
  needs (only `a0` set before the `jalr`, no `a1`).
- `case 8`'s `self->unkB4->methods->slot4C(self->unkB4)` reuses the slot
  established for `StreamTask__Exit`'s own body this same round.
- `self->unkD4`/`self->unkD8` are both pre-existing fields.

## Third-learning check (per head's request)

**Not needed here.** No `self->field` value is read, survives a `jalr`, and
is read again — `self->methods` is fetched exactly once (folded into the
first call via `GetTaskCoreMethods()`, not `self->methods` at all), and every
other field access in each `case` arm happens without an intervening call in
between reads. Matched on the first attempt with plain inline field/slot
accesses, no local variables needed. Filed as a negative data point per the
head's request to record where the lever was and wasn't needed.

## Proposed learning

None new — this function is a clean confirmation of the pre-existing
"switch body order is source order, comparison order is the compiler's own
pivot choice" idiom, now with a 4-way switch instead of a 2-3 way one.

## Naming

**StreamTask__SetState** -- tier C. Occupies `gStreamTaskMethods`
slot `+0x060`; a `switch` on its own second argument over four literal codes
(5/7/8/0x12), each toggling `unkD8` or forwarding through a couple of other
slots, after up-calling the base slot unconditionally. Reads like a small
state dispatcher, but nothing consumes or documents what the four codes
themselves mean in the game (no caller was found), so a name like
`SetState` would assert more than the body supports -- kept
`Class__func_xxxxx` per round 72's tier-B ceiling for unconfirmed data
meaning.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BC14. Occupies +0x060 setState and up-calls TaskCore's first: 5 clears `fadingOut`, 7 sets it, 8 aborts the player unless `abortBeforeFade`, 0x12 refreshViewValue.

## Track 4 (2026-09-26, round 89)

The player is a MoviePlayer (`include/MoviePlayer.h`); Task.h's StreamTaskUnkB4Obj view is gone and Task.c's `PLAYER()` casts `player` (still `BasicClass *` in StreamTask.h) to `MoviePlayer *`. The +0x04C call is `abort`. Byte-identical.
