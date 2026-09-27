# StreamTask__SetAbortBeforeFade

> Renamed from `StreamTaskObj__SetUnkD4` on 2026-09-26 (tools/rename.py). Address 0x8003be7c.

> Renamed from `func_8003BE7C` on 2026-09-23 (tools/rename.py). Address 0x8003be7c.

**Unit:** code_2c054 · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkD4 = value;`. Fifth (last) of the run of five
described in `StreamTask__SetKeepActive`'s report (same class, table slot `+0x134`); see
that report for the shared context.

## Derivation

```
jr   $ra
 sw  $a1, 0xD4($a0)
```

```c
void StreamTask__SetAbortBeforeFade(StreamTaskObj *self, s32 a1) {
    self->unkD4 = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTask__SetKeepActive`'s report — same header, `include/code_2c054.h`.

## Proposed learning

None beyond `StreamTask__SetKeepActive`'s.

## Naming

**StreamTask__SetAbortBeforeFade** -- tier A. Plain setter, fifth (last) of the run
of five described in `StreamTask__SetKeepActive`'s report; same convention.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__SetUnkD4. Own slot +0x134. unkD4 is read by StreamTask__RefreshViewValue (nonzero: MoviePlayer__Abort at once; zero: fade out first) and StreamTask__SetState case 8 (zero: abort after the fade). Reset: 1. Field `abortBeforeFade`, slot `setAbortBeforeFade`.
