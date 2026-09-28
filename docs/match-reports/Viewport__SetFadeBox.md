# Viewport__SetFadeBox — MATCHED

> Renamed from `Viewport__SetSubHandle` on 2026-09-26 (tools/rename.py). Address 0x8003f1a8.

> Renamed from `Unk18Obj__SetSubHandle` on 2026-09-25 (tools/rename.py). Address 0x8003f1a8.

> Renamed from `func_8003F1A8` on 2026-09-23 (tools/rename.py). Address 0x8003f1a8.

Unit: `task`. Round 14, runner delta. 34/34 words, full match (2
real attempts).

## Signature

```c
void Viewport__SetFadeBox(Unk18Obj *self, SubHandleObj *arg1);
```

`Unk18ObjMethods`'s own `+0x0A8` slot occupant (`slotA8`) — a DIFFERENT
call shape than that slot's existing typing (`s32 a1`, from round 13's
`Viewport__Finalize`); per this project's per-call-site convention, this
function's own prototype uses its real parameter type without changing
the shared struct field's own declared shape.

## What it does

Only runs when `self->unk10` is `NULL`: releases the current
`self->unkB0` (if any) via its own `slot4`, then unconditionally installs
`arg1` as the new `self->unkB0` — even when `arg1` is `NULL` — and, only
if `arg1` is non-`NULL`, notifies it (`slot4C`) with `self->unkAC` and the
shared `sFadeBoxAttachPos` constant (already known from round 13's
`Viewport__Viewport`, the same 3rd argument).

```c
void Viewport__SetFadeBox(Unk18Obj *self, SubHandleObj *arg1) {
    if (self->unk10 != NULL) {
        return;
    }

    if (self->unkB0 != NULL) {
        self->unkB0->methods->slot4(self->unkB0);
    }

    self->unkB0 = arg1;
    if (arg1 != NULL) {
        arg1->methods->slot4C(arg1, self->unkAC, sFadeBoxAttachPos);
    }
}
```

## What the first attempt got wrong

Nested `self->unkB0 = arg1;` inside the `if (arg1 != NULL)` guard. Retail's
own `beqz $s0, .L8003F218` has the STORE (`sw $s0, 0xB0($s1)`) as its
OWN delay slot — which executes UNCONDITIONALLY, regardless of the branch
outcome, so `self->unkB0` is overwritten with `arg1` even when `arg1` is
`NULL`. Moving the assignment out of the guard (unconditional store,
THEN a separate conditional notify) reproduces this — the delay slot isn't
an artifact of the guard, it's a genuinely unconditional write that the
guard's own C source structure has to reflect directly.

## Header changes

`include/task.h`: `SubHandleObjMethods` gains `slot4`
(`void (*)(SubHandleObj*)`, release-shaped, no extra args), splitting the
old `pad000[0x04C]` span ahead of the already-typed `slot4C`.

## Naming

`Unk18Obj__SetSubHandle` -- tier A. Guarded by `self->unk10 == NULL`: releases the current `self->unkB0` via its own `slot4` if set, installs `a1`, and notifies it (`slot4C`) with `self->unkAC` and the shared `sFadeBoxAttachPos` constant. Plain "replace the held sub-handle" mechanics.

## Proposed field names

Not applied -- `unkB0`/`unkAC` are both shared with `task.c`
(`Viewport__Viewport` sets both, `Viewport__Finalize` releases `unkAC`), so
outside this unit's ownership per track 3's rule.

- `unkB0` -> `subHandle` (tier B): the field this function and
  `Viewport__GetFadeBox` (this unit) exclusively set/get; matches
  `SubHandleObj`'s own existing type name.
- `unkAC` -> not proposed beyond the existing `Unk18AcObj` typedef's own
  documentation; this unit only forwards it opaquely (`arg1->methods->
  slot4C(arg1, self->unkAC, sFadeBoxAttachPos)`), no new evidence over what
  `include/task.h`'s own comment on `Unk18Obj::unkAC` already records.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetSubHandle`. Slot +0x0A8 `setSubHandle`. The handle is a `SceneNode *` (the ctor's is a FadeBox, id 0x164): release is BasicClass's +0x004 and the attach is SceneNode's +0x04C `attachToParent`, whose occupant in FadeBox's table (BoxFill__AttachToParent) takes a screen position, so sFadeBoxAttachPos (-100, -100) is passed with a `(LongVec3 *)` cast (no code). The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

`D_8008A904` -> `sFadeBoxAttachPos` (tier A: (-100, -100), the position both this and Viewport's ctor attach the fade box at), parameter `handle` -> `fadeBox`.
