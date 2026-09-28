# StreamTask__SetUnkD0

> Renamed from `StreamTaskObj__SetUnkD0` on 2026-09-26 (tools/rename.py). Address 0x8003be74.

> Renamed from `func_8003BE74` on 2026-09-23 (tools/rename.py). Address 0x8003be74.

**Unit:** task · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkD0 = value;`. Fourth of the run of five described in
`StreamTask__SetKeepActive`'s report (same class, table slot `+0x130`); see that report
for the shared context.

## Derivation

```
jr   $ra
 sw  $a1, 0xD0($a0)
```

```c
void StreamTask__SetUnkD0(StreamTaskObj *self, s32 a1) {
    self->unkD0 = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTask__SetKeepActive`'s report — same header, `include/task.h`.

## Proposed learning

None beyond `StreamTask__SetKeepActive`'s.

## Naming

**StreamTask__SetUnkD0** -- tier A. Plain setter, fourth of the run of
five described in `StreamTask__SetKeepActive`'s report; same convention.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). Own slot +0x130. unkD0 is written by this setter and StreamTask__Reset (0) and read by none of the class's own methods.
