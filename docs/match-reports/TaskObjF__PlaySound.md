# TaskObjF__PlaySound -- MATCH

> Renamed from `TaskObjF__SetChildFlag8` on 2026-09-26 (tools/rename.py). Address 0x8004fff4.

> Renamed from `Class86E00_3bb8c_g__SetChildFlag8` on 2026-09-23 (tools/rename.py). Address 0x8004fff4.

> Renamed from `func_8004FFF4` on 2026-09-23 (tools/rename.py). Address 0x8004fff4.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TaskObjF__PlaySound`: 16/16 words match.

## Source

```c
void TaskObjF__PlaySound(Class86E00_3bb8c_g *self, s32 arg1)
{
    if (self->unk6C != NULL) {
        self->unk6C->methods->slot80(self->unk6C, arg1, 0x7F, 0x7F);
    }
}
```

First attempt, byte-exact. `arg1` is forwarded straight through with no
local setup of its own -- retail's own delay slot for the call sets only
the two literal `0x7F`s, and `$a1` is never touched between the function's
entry and the `jalr`, which is what proves this function's own 2nd
parameter is the call's `arg1` rather than an unused register.

## Struct changes (a within-unit retype, caught before any cross-unit
exposure)

- `Class86E00_3bb8c_g::unk6C` **retyped** from `s32` to the new
  `Class86E00Unk6CObj_3bb8c_g *`. This function is the first to
  dereference it (through its own `+0x080` vtable slot); the earlier
  `s32` reading came only from seeing it forwarded opaquely as
  `Class86E00SubObj_3bb8c_g::slot4C`'s 3rd argument while surveying the
  unit (see `TaskObjF__ReleaseCardIcon`'s report), which is equally consistent with a
  scalar or a pointer. Same size, no layout change.
- `Class86E00SubObjMethods_3bb8c_g::slot4C`'s 3rd parameter retyped to
  match (`Class86E00Unk6CObj_3bb8c_g *` instead of `s32`) -- this slot has
  no other known caller yet in this unit, so the retype is fully
  contained.
- New type `Class86E00Unk6CObj_3bb8c_g` / `Class86E00Unk6CObjMethods_3bb8c_g`
  (`slot80` the only reached slot).

Both retyped declarations were introduced by this unit's own earlier
functions THIS round (not inherited from `main`), so there is nothing to
flag for `class_3bb8c_e`/`class_3bb8c_f` here -- the correction is fully
internal to `class_3bb8c_g`'s own work.

### Proposed learning

None new.

## Naming

`TaskObjF__PlaySound` (was `func_8004FFF4`), tier B: forwards
`(self->unk6C, arg1, 0x7F, 0x7F)` to `self->unk6C`'s own vtable slot,
the exact call shape of the already-named `TimedTask__SetChildFlag8` (renamed `TimedTask__PlaySound` in round 84)
(`src/class_39e08.c`: `sub->methods->setFlag8(sub, value, 0x7F, 0x7F)`,
that slot itself named `SceneNode__SetBackClip`) -- same argument
count, same two trailing literals. Named by analogy to that established
shape rather than from an independent derivation of what `0x7F, 0x7F`
means here, so kept at tier B rather than A.

## Track 4 (2026-09-26, round 89)

Renamed from `TaskObjF__SetChildFlag8`. The field it calls (+0x06C, was `childC`) is set only by TaskObjF__Init's 7th argument, and Init's one caller, TitleMenu__BeginCardAccess, passes `self->sound` (TaskCore's VabStreamObj, include/TaskCore.h). +0x080 of that table is `playTone(index, vol, endVol)` (include/VabStreamObj.h), called here with (index, 0x7F, 0x7F): the same shape and the same rename as `TimedTask__PlaySound` (round 84). Callers: TaskObjF__AdvanceState (index 0 and 0x10), TaskObjF__AbortFromState (0x10), through slot +0x08C.

## Track 7 (2026-09-27, round 95)

Parameter `arg1` -> `index` (the playTone index, program << 4 | tone);
volumes written 127. Image byte-identical.
