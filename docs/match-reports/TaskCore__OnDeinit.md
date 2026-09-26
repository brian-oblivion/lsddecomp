# TaskCore__OnDeinit

> Renamed from `TaskCoreObj__func_8003C3D0` on 2026-09-25 (tools/rename.py). Address 0x8003c3d0.

> Renamed from `func_8003C3D0` on 2026-09-23 (tools/rename.py). Address 0x8003c3d0.

> **Type rename note (round 73, track 3):** `self->unk18`'s type
> (`TaskCoreObj *` / `TaskCoreObjMethods *` below) was renamed to
> `StreamTaskUnk18Obj *` / `StreamTaskUnk18Methods *` in
> `include/code_2c054.h` -- see `TaskCore__OnInit.md`'s own note
> for why. The text below is left as originally written for the history.

**Unit:** code_2c054 · **Size:** 47 words · **Status:** MATCHED (47/47)

## Summary

```c
void TaskCore__OnDeinit(StreamTaskObj *self) {
    TaskCoreObj *obj = self->unk18;
    obj->methods->slot90(obj);
    obj->methods->slot74(obj);
    self->unk78->methods->slot50(self->unk78);
    if (self->unk34 != 0) {
        self->unkC->unk0->methods->slot78(self->unkC->unk0, &self->unk93, 0);
    }
}
```

## New structure discovered

Three new fields on `StreamTaskObj`, all previously unnamed padding, plus
three new small "vtable at offset 0" object types (the same generic idiom as
`StreamTaskUnkB4Obj`, kept as SEPARATE types rather than reused — nothing
ties them to the same concrete class beyond the shared idiom):

- `self->unkC` (`+0x00C`, `StreamTaskUnkCObj *`): a 1-word holder whose own
  word 0 (`unk0`) is the real dispatch target, `TaskTextObj *`. Two levels of
  indirection: `self->unkC->unk0->methods->slot78(...)`.
- `self->unk18` (`+0x018`, `TaskCoreObj *`): a per-instance object.
  **CORRECTED (later this round, by `TaskCore__OnInit`):** originally modeled
  as sharing `TaskCoreMethods` itself (the class `Get_vtable_TaskCore()`'s
  gTaskCoreMethods singleton belongs to), on the strength of `classtable.py
  gTaskCoreMethods` having non-null entries at the two offsets (`+0x074`,
  `+0x090`) called here. That agreement was coincidence, not evidence:
  `TaskCore__OnInit` calls three more slots on `self->unk18`
  (`+0x048`/`+0x04C`/`+0x050`) with an arity `TaskCoreMethods`'s own
  `+0x04C` (occupied by `TaskCore__OnInit` itself, confirmed single-argument by
  `StreamTask__OnInit`'s byte-exact call) cannot have. `self->unk18`'s vtable is
  now its own separate type, `TaskCoreObjMethods` — see `TaskCore__OnInit.md`
  for the full reasoning. `slot74`/`slot90` (below) now live there, not in
  `TaskCoreMethods`.
- `self->unk34` (`+0x034`, `s32`): a boolean-ish guard.
- `self->unk78` (`+0x078`, `StreamTaskUnk78Obj *`): another 1-word "vtable at
  offset 0" object, only slot `+0x050` reached.
- `self->unk93` (`+0x093`, `s8`): only ever address-taken (`&self->unk93`),
  passed as a buffer pointer to the `unkC` chain's `slot78` call. Real extent
  past one byte is unknown — nothing in this function dereferences past the
  first byte.

## The one real residue and its fix

First attempt wrote the three field accesses inline
(`self->unk18->methods->slot90(self->unk18); self->unk18->methods->slot74(self->unk18); ...`)
with no local variable. That built and linked but scored 3/47 with a
**frame-size mismatch** (retail `addiu sp, sp, -0x20` with `s0`+`s1`+`ra`
saved; built `-0x18` with fewer saves) — the single biggest tell that
something structural, not cosmetic, was wrong.

Cause: `obj->methods->slot90(obj)` is a call, and GCC cannot assume a call
through an unknown function pointer leaves `*self` (specifically
`self->unk18`) unmodified — o32 has no aliasing guarantee here. Without a
local variable pinning the value, the compiler must reload `self->unk18` from
memory after the intervening call rather than keeping it live across it,
which changes the whole register-allocation shape (no need for a second
callee-saved register once nothing has to survive a call). Introducing
`TaskCoreObj *obj = self->unk18;` gives the loaded pointer its own
call-surviving lifetime, forcing GCC to keep it in a saved register across
the two calls — exactly retail's `s0`. Fixed all 44 residue words at once.

## Proposed learning

**A value read from `self->field` and used again AFTER an intervening call
through an unknown function pointer needs an explicit local variable, not a
repeated field access — even though both are "the same value" semantically.**
GCC 2.6.3 has no aliasing information about what an indirect call through a
vtable slot might write back into the caller's object, so a bare repeated
`self->field` access forces a reload after the call; a named local pins the
value in a register (callee-saved, if it must survive the call) instead. The
frame-size mismatch (fewer/more saved registers than retail) is the fast
tell for this class, distinguishable at a glance from a same-size residue.

## Naming

**TaskCoreObj__func_8003C3D0** -- tier C. Occupies slot `+0x050` in BOTH
`gTaskCoreMethods` and `gStreamTaskMethods` at the identical address --
i.e. StreamTaskObj does NOT override this slot, so the function genuinely
belongs to `TaskCoreObj` (confirmed by `classtable.py`'s slot-for-slot
comparison, see the unit header comment), not to `StreamTaskObj` despite its
`self` parameter being typed `StreamTaskObj *` (the common-caller
convention, per `TaskCoreMethods`'s own established parameter typing). Tears
down `self->unk18` through two slots, then `unk78`'s own slot `+0x050`,
then conditionally the `TaskText` sub-object -- teardown-shaped but not the
dtor slot, so left `Class__func_xxxxx` rather than assert "Stop" or
"Deactivate".

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__func_8003C3D0 (tools/rename.py). Occupant of +0x050 (`onDeinit`, IntermediateBase__Deinit's first call), named for the slot. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-25, round 85, bravo)

The viewport is cast to `Viewport *` (include/Viewport.h, round 85) instead of the local StreamTaskUnk18Obj view; +0x090 is deinitOt and +0x074 detachViewChild. Byte-identical.

## Track 4 (2026-09-26, round 88, alpha)

bgLayer is a `BgLayer *` (include/BgLayer.h): the StreamTaskUnk78Obj cast is gone and +0x050 is called as `detachFromParent` (Class6B5CC's; returns Class6B5CC *, discarded, where the view said void). Byte-identical.
