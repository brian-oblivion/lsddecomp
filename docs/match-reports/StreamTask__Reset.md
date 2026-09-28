# StreamTask__Reset

> Renamed from `StreamTaskObj__Reset` on 2026-09-26 (tools/rename.py). Address 0x8003ba38.

> Renamed from `func_8003BA38` on 2026-09-23 (tools/rename.py). Address 0x8003ba38.

**Unit:** task · **Size:** 8 instructions (0x20 bytes) · **Status:** MATCHED (8/8 words, whole-image SHA1 green), first attempt

## What it does

Resets the five fields the `StreamTask__SetKeepActive`.."`7C`" setters (see that
report) write individually: `self->unkC4=0; unkC8=-1; unkCC=1; unkD0=0;
unkD4=1;`. Slot `+0x040` of `gStreamTaskMethods` (`GetStreamTaskMethods`'s report), so
presumably an "init"/"reset" method for this `StreamTaskObj` class.

## Derivation

```
addiu $v0, $zero, -0x1
sw    $v0, 0xC8($a0)
ori   $v0, $zero, 0x1
sw    $zero, 0xC4($a0)
sw    $v0, 0xCC($a0)
sw    $zero, 0xD0($a0)
jr    $ra
 sw   $v0, 0xD4($a0)
```

```c
void StreamTask__Reset(StreamTaskObj *self) {
    self->unkC8 = -1;
    self->unkC4 = 0;
    self->unkCC = 1;
    self->unkD0 = 0;
    self->unkD4 = 1;
}
```

Matched first attempt, source order following the store order exactly (the
compiler reuses one temp register, `$v0`, first for `-1` then for `1`,
which falls out naturally from writing the five assignments straight-line in
this order -- no reshaping needed).

## New struct/header knowledge

Named the five fields in `include/task.h`'s `StreamTaskObj` (shared
with the setters' report).

## Proposed learning

None new beyond `StreamTask__SetKeepActive`'s.

## Naming

**StreamTask__Reset** -- tier A. Occupies `gStreamTaskMethods` slot
`+0x040`, the SAME numbered slot independently named `Reset` in two other,
unrelated classes in this codebase (`StageMap__Reset`,
`include/dream_day.h`; `SceneNode__Reset`, `include/scene_node.h`) --
both also called from their own class's ctor chain, both also just a run of
fixed-literal field stores, exactly this function's own shape.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/stream_task.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). Occupies +0x040 resetCounters, as TaskCore__Reset does: the name follows TaskCore's.
