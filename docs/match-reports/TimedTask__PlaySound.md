# TimedTask__PlaySound

> Renamed from `Class86668__PlaySound` on 2026-09-26 (tools/rename.py). Address 0x8004a478.

> Renamed from `TimedTask__SetChildFlag8` on 2026-09-25 (tools/rename.py). Address 0x8004a478.

> Renamed from `func_8004A478` on 2026-09-22 (tools/rename.py). Address 0x8004a478.

**Unit:** dream_day · **Size:** 16 words · **Status:** MATCHED (16/16 words)

## What it does

`TimedTask`'s own slot +0x070 method. Reads the `StageMap` instance held at
`self->unk34`; if non-NULL, forwards its own second parameter plus two literal
`0x7F` values into that sub-object's vtable slot +0x080.

## Derivation

```
lw    $a0, 0x34($a0)        ; a0 = self->unk34 (overwrites self)
beqz  $a0, .L8004A4A8
 ori  $a2, $zero, 0x7F
lw    $v0, 0x0($a0)         ; v0 = sub->methods
lw    $v0, 0x80($v0)        ; v0 = sub->methods->slot80
jalr  $v0
 ori  $a3, $zero, 0x7F
```

Resolved via `tools/classtable.py gTimedTaskMethods` and `tools/classtable.py
gStageMapMethods`: `gTimedTaskMethods` (28 slots, header 0x230) is this function's own
containing class's vtable, and this IS its last slot (+0x070) — the only
reason this class is visible from this unit at all. `gStageMapMethods` (80 slots,
header 0x114) is the vtable of the sub-object at `self->unk34`; slot +0x080
there is `SceneNode__SetBackClip` (outside this unit, in `SceneNode.s`), which reads
its own incoming `$a0`/`$a1`/`$a2` (three real params) and ignores a fourth —
consistent with the call here supplying four register args.

**The second parameter is real, not leftover garbage.** The call sets `$a2`
and `$a3` to literal `0x7F` but leaves `$a1` untouched. MIPS's calling
convention fills `$a0..$a3` in order with no gaps: if the call had only three
real arguments (self + two literals), the literals would occupy `$a1`/`$a2`,
not `$a2`/`$a3`. Since they demonstrably occupy the *third* and *fourth*
argument slots, `$a1` must be a real (fourth total, second real) argument —
which can only be `TimedTask__PlaySound`'s own second parameter, forwarded unchanged
(no `move` needed since it's already resident in the right register).

Named the two classes by vtable address per project convention (see
`game_application.h`): `TimedTask` (gTimedTaskMethods) and `StageMap` (gStageMapMethods).
Both declared in the new `include/dream_day.h`.

## Proposed learning

When a vtable-dispatch call sets later argument registers (`$a2`/`$a3`) to
literals but leaves an earlier one (`$a1`) untouched, that's proof the earlier
register carries a real forwarded parameter, not coincidental leftover state —
MIPS's strict left-to-right register-filling convention means there's no
"skip a register" call shape. This resolves an ambiguity that pure "no
instruction was needed either way" reasoning can't: a caller with no argument
in that slot at all would use `$a1`, not `$a2`, for its first literal.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004A478` | `TimedTask__SetChildFlag8` | B | `gTimedTaskMethods`'s own last slot (`+0x070`). Reads the child object at `self->unk34`, and if non-NULL dispatches its `+0x080` slot. In `StageMap` that slot is the inherited `SceneNode__SetBackClip` (matched, `SceneNode`), a get-or-set of bit 8 of the object's `unk10` bitfield, taking `(self, value)` -- so the two literal `0x7F`s this call sets up are not read by that occupant. Tier B, and prefixed with the containing class rather than the callee's: the name says what this method does (push a flag down to the held child), not what the child's class is. |

Deliberately NOT asserted: that `TimedTask::unk34` is a `StageMap`. The
header claims it, but the only evidence is that slot `+0x080` exists in
`gStageMapMethods` -- true of every `SceneNode` descendant. The name avoids
depending on it.

The two `0x7F` arguments are left in the call: they cost nothing, they are
what retail's caller sets up, and other occupants of slot `+0x070` in this
table family may read them.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Renamed from `TimedTask__SetChildFlag8`, tier A. The ctor settles what +0x034 is: `New_VabStreamObj(soundBankPath)` when the first ctor argument is non-NULL (Obj865C8 passes GetSoundEffectDir()), the caller's object otherwise (ObjM passes Obj865C8's, itself a VabStreamObj); ObjM calls the same field's +0x088/+0x08C, VabStreamObj's Mute/Unmute. gVabStreamObjMethods' +0x080 is VabStreamObj__PlayTone, so this is `sound->PlayTone(tone, 0x7F, 0x7F)`, the shape of TaskCore__PlaySound (which passes 0x60, 0x60). The StageMap reading above is withdrawn: the field is `TimedTask::sound` (`BasicClass *`), cast in dream_day.c to a local SoundObj_3ac78 view with +0x080 `playTone`. Slot +0x070 is named `playSound`. Image byte-identical.

## Track 4 (2026-09-26, round 87, VabStreamObj)

`src/world/dream_day.c`'s `SoundObj_3ac78`/`SoundObjMethods_3ac78` view is
deleted. `sound` is cast to `VabStreamObj *` (`include/VabStreamObj.h`) and
calls `playTone`, the same slot at the same type. The whole image stays
byte-identical. `TimedTask::sound` stays `BasicClass *`.

## Track 7 (2026-09-27, round 98, charlie)

`playTone(sound, tone, 0x7F, 0x7F)` -> decimal `127, 127` (volumes are levels, decimal by the base rule; libsnd volumes run 0..127). No name: none exists in Sony's headers and one would restate the value. Comment now says it plays at full volume.
