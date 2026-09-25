# StreamTaskObj__func_8003BDF4

> Renamed from `func_8003BDF4` on 2026-09-23 (tools/rename.py). Address 0x8003bdf4.

**Unit:** code_2c054 · **Size:** 26 words · **Status:** MATCHED (26/26)

## Summary

An `if`/`else` selecting one of two forwarding calls based on `self->unkD4`.

```c
void StreamTaskObj__func_8003BDF4(StreamTaskObj *self) {
    if (self->unkD4 != 0) {
        self->unkB4->methods->slot4C(self->unkB4);
    } else {
        self->methods->slot60(self, 7);
    }
}
```

## Evidence

- `self->unkD4` is the field set by this unit's `StreamTaskObj__SetUnkD4` (already
  established).
- `self->unkB4->methods->slot4C(self->unkB4)`: `self->unkB4` is
  `StreamTaskUnkB4Obj*` (established). Its vtable had only slot `+0x004`
  named before this function; this one dereferences `unkB4->methods` and
  calls slot `+0x04C` with `unkB4` as the sole argument and a discarded
  return, so it is typed `void (*)(StreamTaskUnkB4Obj *self)` here, no
  counter-evidence.
- `self->methods->slot60(self, 7)` reuses the slot established matching
  `StreamTaskObj__func_8003BD10` in the same round (`StreamTaskObjMethods::slot60`,
  occupied by `StreamTaskObj__func_8003BC14` per `classtable.py gStreamTaskObjMethods`).

## Proposed learning

None beyond what `StreamTaskObj__func_8003BD10`'s report already states.

## Naming

**StreamTaskObj__func_8003BDF4** -- tier C. Occupies `gStreamTaskObjMethods`
slot `+0x094`; an `if`/`else` choosing between tearing down through the
private `unkB4` sub-object's slot `+0x04C` or re-entering this class's own
state-7 transition, gated by `unkD4` -- the same `unkB4->methods->slot4C`
call `StreamTaskObj__func_8003BC14`'s `case 8` makes under the inverse
condition. Neither `unkD4`'s nor "state 7"'s game meaning is confirmed, so
left `Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

StreamTaskObj now expands TASKCORE_SLOTS (include/TaskCore.h, round 84): its `slot60` call is `setState`. Byte-identical.
