# StreamTask__SetSkipOnConfirm

> Renamed from `StreamTaskObj__SetUnkCC` on 2026-09-26 (tools/rename.py). Address 0x8003be6c.

> Renamed from `func_8003BE6C` on 2026-09-23 (tools/rename.py). Address 0x8003be6c.

**Unit:** task · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkCC = value;`. Third of the run of five described in
`StreamTask__SetKeepActive`'s report (same class, table slot `+0x12C`); see that report
for the shared context.

## Derivation

```
jr   $ra
 sw  $a1, 0xCC($a0)
```

```c
void StreamTask__SetSkipOnConfirm(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTask__SetKeepActive`'s report — same header, `include/task.h`.

## Proposed learning

None beyond `StreamTask__SetKeepActive`'s.

## Naming

**StreamTask__SetSkipOnConfirm** -- tier A. Plain setter, third of the run of
five described in `StreamTask__SetKeepActive`'s report; same convention.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__SetUnkCC. Own slot +0x12C. unkCC is read only by StreamTask__OnPadConfirm: nonzero, a confirm press ends the task with result 2. Reset: 1. game_shell's GraphRoom, cinematic and init-stream starters call it with 0 (the intro-logo and weekly starters leave it 1). Field `skipOnConfirm`, slot `setSkipOnConfirm`.
