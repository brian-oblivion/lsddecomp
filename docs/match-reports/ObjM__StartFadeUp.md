# ObjM__StartFadeUp

> Renamed from `ObjM__ForwardToSubChild` on 2026-09-26 (tools/rename.py). Address 0x80053eb4.

> Renamed from `func_80053EB4` on 2026-09-23 (tools/rename.py). Address 0x80053eb4.

**Unit:** class_3bb8c_k · **Size:** 52 instructions · **Status:** MATCHED (52/52 words)

## Context

This is the shared helper called by `ObjM__EnterState7`, `ObjM__EnterState8` and
`ObjM__EnterStateA` (all matched this round, see their own reports). Its
5th argument arrives on the stack in the standard o32 convention (caller
reserves 0x10 bytes for `$a0`-`$a3`, so a 5th argument lands at
`0x10($sp)` from the caller's own frame).

Establishes `ObjM::unk18` (a pointer to a second new type, `FieldM18`,
with vtable slot `0xAC`) and the return type of that slot — `ChildM_AC`,
whose own vtable exposes `slotD0` and `slotD8`.

## What this function does

```c
void ObjM__StartFadeUp(ObjM *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    ChildM_AC *obj = self->unk18->methods->slotAC(self->unk18);
    if (arg3 != 0) {
        obj->methods->slotD0(obj, arg3);
    }
    if (arg4 != 0) {
        self->methods->slot10(self, obj);
    }
    obj->methods->slotD8(obj, self->unk10, arg1, arg2);
}
```

`self->unk18->methods->slotAC(self->unk18)` is called with a single
argument (self only) — the call site sets up no `$a1`/`$a2`/`$a3`, and
nothing downstream reads them as if they mattered, so `slotAC` is modelled
as arity 1. `self->methods->slot10(self, obj)` establishes
`ObjMMethods::slot10`.

## Residue

None — matched on the first attempt. The trickiest part was recognizing
that `arg3`/`arg4` gate two INDEPENDENT calls (not one conditional with two
branches) and that the final `slotD8` call always executes regardless of
either guard.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_k`.

## Naming

**ObjM__StartFadeUp** -- tier B. Shared helper for the three `EnterState*` functions: fetches a `ChildM_AC` from `self->unk18`'s `slotAC`, optionally sets it up (`slotD0`) and notifies the base (`self->methods->slot10`), then dispatches `slotD8(self->unk10, arg1, arg2)`. Mechanically clear; what the pushed value represents in-game is not established.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__ForwardToSubChild` (rename.py). The "sub child" is the viewport's fade box: IntermediateBase::viewport (+0x018) is the NodeGuardedViewport of the building DayTask's init args, its +0x0AC is Viewport's getSubHandle, whose object is Viewport's New_FadeBox; +0x0D0 and +0x0D8 on it are FadeBox's setStep and startFadeUp, the source being IntermediateBase::unk10 (the FrameClock). So: set the fade step (when nonzero), add the box as a child (when asked, so its 5/6 notifications reach ObjM__OnFadeNotify), start a fade up with the given channels. Tier A for the mechanics. Parameters named (channels, arg2, step, addChild).

## Track 7 (2026-09-27, round 98, delta)

The third parameter, `arg2`, is now `fadeMode` (here and in ObjM.h's
prototype): it goes to FadeBox's startFadeUp and on to configure, which
stores it in FadeBox::unk7C; FadeBox__Update does not step the colour
while it is 9. Every caller passes 0. Tier B (mechanics). Zero bytes.
