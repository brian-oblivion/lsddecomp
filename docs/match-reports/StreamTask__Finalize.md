# StreamTask__Finalize

> Renamed from `StreamTaskObj__Destroy` on 2026-09-26 (tools/rename.py). Address 0x8003b9dc.

> Renamed from `func_8003B9DC` on 2026-09-23 (tools/rename.py). Address 0x8003b9dc.

**Unit:** task · **Size:** 23 instructions (0x5C bytes) · **Status:** MATCHED (23/23 words, whole-image SHA1 green), first attempt

## What it does

Two dispatches in a row, both discarding/forwarding through `self`, no other
side effect. First, a genuine virtual call through `self->unkB4`'s own
1-slot vtable (a small object type distinct from `StreamTaskObj`, discovered
here for the first time in this unit); second, the same
`GetTaskCoreMethods()`-mediated delegation to the sibling class `gTaskCoreMethods`
(`LoaderTaskMethods`) used by `StreamTask__OnPadPrev`/`StreamTask__OnPadNext`, this time
slot `+0x00C`. Occupies `gStreamTaskMethods` slot `+0x00C` itself.

## Derivation

```
lw   $a0, 0xB4($s0)          ; a0 = self->unkB4
lw   $v0, 0x0($a0)            ; v0 = a0->methods
lw   $v0, 0x4($v0)             ; v0 = methods->slot04
jalr $v0                          ; a0 (still the sub-object) unchanged
jal  GetTaskCoreMethods
lw   $v0, 0xC($v0)                ; v0 = table->slot0C
jalr $v0
 addu $a0, $s0, zero               ; a0 = self, explicitly reloaded
...epilogue
```

```c
void StreamTask__Finalize(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    GetTaskCoreMethods()->slot0C(self);
}
```

Matched first attempt. The first call's argument register (`$a0`) is never
reloaded between the two loads and the `jalr` -- confirming the call target
is `self->unkB4` itself (a virtual self-call on the sub-object), not `self`.

## New struct/header knowledge

Added `include/task.h`'s `StreamTaskUnkB4Obj`/`StreamTaskUnkB4Methods`
(a new, previously-unseen 1-slot-vtable object reached through
`StreamTaskObj::unkB4`, `+0x0B4`) and `TaskCoreMethods::slot0C` (this unit's
local view of `gTaskCoreMethods`, see `StreamTask__OnPadPrev`'s report).

## Proposed learning

Same open return-type question as `StreamTask__OnPadPrev`/`StreamTask__OnPadNext` for the
tail call through `slot0C` -- typed `void` on the same sibling-slot-
convention basis, unconfirmed by any found caller.

## Naming

**StreamTask__Finalize** -- tier A. Occupies `gStreamTaskMethods`'s dtor
slot `+0x00C` (a base-class layout convention independently confirmed in
`include/dream_day.h`'s own `ctor`/`dtor` pair at `+0x008`/`+0x00C`, and in
`include/data_source.h`'s `FileResource__Finalize`). Tears down the private
`unkB4` sub-object, then up-calls `TaskCore__Finalize` at the same slot --
the "override, do extra work, call the base" shape this whole unit's slot
comparison confirms (see the unit header comment).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/stream_task.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). Was StreamTaskObj__Destroy. It occupies +0x00C, the finalize slot (TaskCore__Finalize in the parent, `classtable.py gStreamTaskMethods --vs gTaskCoreMethods`), releases the MoviePlayer at +0x0B4 and up-calls TaskCore's finalize: named for its slot.

## Track 4 (2026-09-26, round 89)

The player is a MoviePlayer (`include/movie_player.h`); task.h's StreamTaskUnkB4Obj view is gone and task.c's `PLAYER()` casts `player` (still `BasicClass *` in stream_task.h) to `MoviePlayer *`. The +0x004 call is `release`. Byte-identical.
