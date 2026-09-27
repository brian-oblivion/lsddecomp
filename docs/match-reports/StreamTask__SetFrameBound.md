# StreamTask__SetFrameBound

> Renamed from `StreamTaskObj__SetUnk40` on 2026-09-26 (tools/rename.py). Address 0x8003bcf4.

> Renamed from `func_8003BCF4` on 2026-09-23 (tools/rename.py). Address 0x8003bcf4.

**Unit:** Task · **Size:** 7 instructions (0x1C bytes) · **Status:** MATCHED (7/7 words, whole-image SHA1 green), first attempt

## What it does

Sets `self->unk40 = a1`, then, only when `a1 >= 0`, overwrites it with
`a1 * 15`. Textbook instance of the project's already-confirmed "default
value, then conditionally overwritten by an `if` with no `else`" idiom
(`docs/DECOMPILATION_LEARNINGS.md`, `CheckTriggerDayParity`): 2.6.3 slides the
unconditional store into the guarding branch's delay slot for free.

## Derivation

```
bltz  $a1, .L8003BD08
 sw   $a1, 0x40($a0)
sll   $v0, $a1, 4
subu  $v0, $v0, $a1        ; a1*16 - a1 == a1*15
sw    $v0, 0x40($a0)
.L8003BD08:
jr    $ra
 nop
```

```c
void StreamTask__SetFrameBound(StreamTaskObj *self, s32 a1) {
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 15;
    }
}
```

Matched first attempt. `a1 * 15` reproduces retail's shift-subtract
multiply-by-constant expansion directly; no need to write the shift/subtract
by hand.

## New struct/header knowledge

Named `StreamTaskObj::unk40` in `include/Task.h`.

## Proposed learning

Another confirmed instance of the "default value, conditionally overwritten,
no `else`" idiom from `docs/DECOMPILATION_LEARNINGS.md` -- worth keeping on
the shortlist of shapes to try first when the residue is "one extra
instruction" or a delay-slot store that looks unconditional at a glance.

## Naming

**StreamTask__SetFrameBound** -- tier A. Plain setter (with a `* 15` scale
when the argument is non-negative) for `self->unk40`; a pure leaf whose
mechanics are its whole purpose, matching the `Class__SetUnkNN` convention
already used elsewhere for a field of unconfirmed game meaning
(`Entity__SetTargetReached`, `src/Entity.c`).

## Track 4 (2026-09-25, round 84, alpha)

StreamTaskObj now expands TASKCORE_FIELDS/TASKCORE_SLOTS (include/TaskCore.h, round 84): the field this sets is TaskCore's +0x040 `frameBound`, and this function is StreamTaskObj's override of TaskCore's +0x06C setFrameBound (x15 where TaskCore__SetFrameBound multiplies by 20). Byte-identical.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__SetUnk40 (+0x040 is TaskCore's frameBound word, not a slot). Occupies +0x06C setFrameBound: frameBound = bound * 15 (TaskCore__SetFrameBound: * 20), negative kept. code_1677c's GameApplication__StartGraphRoomStreamTask calls it with count / 15.
