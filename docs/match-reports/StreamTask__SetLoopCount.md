# StreamTask__SetLoopCount

> Renamed from `StreamTaskObj__SetUnkC8` on 2026-09-26 (tools/rename.py). Address 0x8003be64.

> Renamed from `func_8003BE64` on 2026-09-23 (tools/rename.py). Address 0x8003be64.

**Unit:** task · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkC8 = value;`. Second of the run of five described in
`StreamTask__SetKeepActive`'s report (same class, table slot `+0x128`); see that report
for the shared context (class identity, table-slot derivation, why these are
setters and not BIOS trampolines).

## Derivation

```
jr   $ra
 sw  $a1, 0xC8($a0)
```

```c
void StreamTask__SetLoopCount(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTask__SetKeepActive`'s report — same header, `include/task.h`.

## Proposed learning

None beyond `StreamTask__SetKeepActive`'s.

## Naming

**StreamTask__SetLoopCount** -- tier A. Plain setter, second of the run of
five described in `StreamTask__SetKeepActive`'s report; same convention.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__SetUnkC8. Own slot +0x128. unkC8 is MoviePlayer__Play's fourth argument, which Play stores at the player's +0x058, the field MoviePlayer__Advance counts down as `loops`; StreamTask__Reset sets it to -1. Field `loopCount`, slot `setLoopCount`.
