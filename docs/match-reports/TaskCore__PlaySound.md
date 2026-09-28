# TaskCore__PlaySound — MATCH (16/16 words)

> Renamed from `Obj86B60__ForwardToChild` on 2026-09-25 (tools/rename.py). Address 0x8003c7b4.

> Renamed from `func_8003C7B4` on 2026-09-24 (tools/rename.py). Address 0x8003c7b4.

**Unit:** task · **Size:** 16 instructions

## What it does

```c
void TaskCore__PlaySound(Obj86B60 *self, s32 a1)
{
    Unk48Obj *child;

    child = self->unk48;
    if (child != NULL) {
        child->methods->slot80(child, a1, 0x60, 0x60);
    }
}
```

`self->unk48` is a pointer to a DIFFERENT class from `Obj86B60` -- its own
vtable slot `+0x080` takes four arguments (self, a1, a2, a3), whereas
`Obj86B60`'s OWN slot `+0x080` (`TaskCore__OnPadPrev`) takes none beyond self. Two
literal `0x60` constants are passed as the trailing two arguments; `a1` is
forwarded from the caller unchanged.

## Struct knowledge established

- `Obj86B60::unk48` (`Unk48Obj *`, +0x048) -- OBSERVED here, only user in
  this unit.
- `Unk48Obj` -- a minimal opaque type: `methods` at +0x000, one modelled
  slot (`slot80`, `void (*)(Unk48Obj*, s32, s32, s32)` at +0x080).

## Provenance

round 2026-09-02, runner echo, unit task.

## Naming (round 78, delta)

**Tier B.** `func_8003C7B4` -> `Obj86B60__ForwardToChild`. Occupies slot70.
Body: when `self->unk48` (a distinct, still-`Unk48Obj`-typed child) is
non-NULL, forwards `(child, a1, 0x60, 0x60)` to `child->methods->slot80`.
Mechanics are clear (a conditional forward to a child object, two args
fixed); what the fixed `0x60, 0x60` pair or the child's real identity
represent in the game is not established, so tier B rather than A.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__ForwardToChild (tools/rename.py). Occupant of +0x070 (`playSound`). The "child" is +0x048, New_VabStreamObj(soundBankPath) with "ETC\ETCSE" at both subclass ctors, and the call is its +0x080, VabStreamObj__PlayTone (tone, 0x60, 0x60). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87, VabStreamObj)

`include/task.h`'s `Unk48Obj`/`Unk48ObjMethods` view is deleted.
`sound` is now cast to `VabStreamObj *` (`include/vab_stream_obj.h`), and the
call is `playTone`. The view had typed the slot `void`, but the occupant
`VabStreamObj__PlayTone` returns the voice. The whole image stays
byte-identical with the s32 slot, because the call is this void function's
last statement and nothing reads `$v0`. `TaskCore::sound` stays
`BasicClass *`: it is TaskCore's field, and it can also hold the ctor's own
`sound` argument.

## Track 7 (round 98, alpha)

PlayTone's `0x60, 0x60` (vol, endVol) -> TASKCORE_TONE_VOLUME (96). Tones are TASKCORE_TONE_CURSOR 0x00 / TASKCORE_TONE_BUTTON 0x10, PlayTone indices (program << 4 | tone), include/task_core.h. Byte-identical.
