# StreamTask__RefreshViewValue

> Renamed from `StreamTaskObj__func_8003BDF4` on 2026-09-26 (tools/rename.py). Address 0x8003bdf4.

> Renamed from `func_8003BDF4` on 2026-09-23 (tools/rename.py). Address 0x8003bdf4.

**Unit:** Task · **Size:** 26 words · **Status:** MATCHED (26/26)

## Summary

An `if`/`else` selecting one of two forwarding calls based on `self->unkD4`.

```c
void StreamTask__RefreshViewValue(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}
```

## Evidence

- `self->unkD4` is the field set by this unit's `StreamTask__SetAbortBeforeFade` (already
  established).
- `self->unkB4->methods->slot4C(self->unkB4)`: `self->unkB4` is
  `StreamTaskUnkB4Obj*` (established). Its vtable had only slot `+0x004`
  named before this function; this one dereferences `unkB4->methods` and
  calls slot `+0x04C` with `unkB4` as the sole argument and a discarded
  return, so it is typed `void (*)(StreamTaskUnkB4Obj *self)` here, no
  counter-evidence.
- `self->methods->slot60(self, 7)` reuses the slot established matching
  `StreamTask__OnPadConfirm` in the same round (`StreamTaskObjMethods::slot60`,
  occupied by `StreamTask__SetState` per `classtable.py gStreamTaskMethods`).

## Proposed learning

None beyond what `StreamTask__OnPadConfirm`'s report already states.

## Naming

**StreamTask__RefreshViewValue** -- tier C. Occupies `gStreamTaskMethods`
slot `+0x094`; an `if`/`else` choosing between tearing down through the
private `unkB4` sub-object's slot `+0x04C` or re-entering this class's own
state-7 transition, gated by `unkD4` -- the same `unkB4->methods->slot4C`
call `StreamTask__SetState`'s `case 8` makes under the inverse
condition. Neither `unkD4`'s nor "state 7"'s game meaning is confirmed, so
left `Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

StreamTaskObj now expands TASKCORE_SLOTS (include/TaskCore.h, round 84): its `slot60` call is `setState`. Byte-identical.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BDF4. Occupies +0x094 refreshViewValue (reached from setState(0x12)). `abortBeforeFade` set: MoviePlayer__Abort at once; clear: setState(7), and SetState's case 8 aborts after the fade.

## Track 4 (2026-09-26, round 89)

The player is a MoviePlayer (`include/MoviePlayer.h`); Task.h's StreamTaskUnkB4Obj view is gone and Task.c's `PLAYER()` casts `player` (still `BasicClass *` in StreamTask.h) to `MoviePlayer *`. The +0x04C call is `abort`. Byte-identical.
