# TaskCore__Finalize

> Renamed from `TaskCoreObj__Destroy` on 2026-09-25 (tools/rename.py). Address 0x8003c008.

> Renamed from `func_8003C008` on 2026-09-23 (tools/rename.py). Address 0x8003c008.

**Unit:** task · **Size:** 69 words · **Status:** MATCHED (69/69)

## Summary

A teardown/finalize function: five identically-shaped
`field->methods->slot04(field)` calls (two of them conditional), followed by
two more forwarding calls.

```c
void TaskCore__Finalize(StreamTaskObj *self) {
    self->unk78->methods->slot04(self->unk78);
    self->unk7C->methods->slot04(self->unk7C);
    self->unk80->methods->slot04(self->unk80);
    if (self->unk44 != 0) {
        self->unk48->methods->slot04(self->unk48);
    }
    if (self->unk70 != 0) {
        self->unk74->methods->slot04(self->unk74);
    }
    self->methods->slotDC(self);
    GetIntermediateBaseMethods()->slot0C(self);
}
```

## UPDATE (later this round, by `TaskCore__OnInit`)

The "`StreamTaskUnk78Obj` folded into `StreamTaskUnkB4Obj`" unification
described just below was **reversed** by `TaskCore__OnInit`: it calls
`self->unk78`'s own slot `+0x04C` with 3 arguments, where
`StreamTaskUnkB4Methods::slot4C` (this same offset, established
single-argument by the already-matched `StreamTask__Exit`) cannot take 3. A
real arity conflict at a shared slot means the two are sibling classes that
merely agree at `slot04` (this function's own evidence), not one class.
`self->unk78` is back to its own `StreamTaskUnk78Obj/Methods` type; the code
snippet below (`self->unk78->methods->slot04(self->unk78);`) is unaffected
since only the DECLARED TYPE changed, not the field access syntax. See
`TaskCore__OnInit.md` for the full account. `unk48`/`unk74`/`unk7C`/`unk80`
remain unified with `StreamTaskUnkB4Obj` — no conflicting evidence has
appeared for any of those four.

## New structure discovered, and a correction to earlier work this round

This function is what triggered the retype documented in
`TaskCore__TaskCore.md`'s addendum: `self->unk48`, `self->unk7C`, and
`self->unk80` were modeled as plain `s32` there (no counter-evidence at the
time — they were just call results). Here, all three are dereferenced as
`field->methods->slot04(field)` — impossible for a plain integer. Retyped
all three to `StreamTaskUnkB4Obj *` in `include/task.h`; this changes
no compiled bytes anywhere (same register width, pure pointer/int
relabeling) and a full rebuild confirmed every one of this round's nine
prior matches (`StreamTask__OnPadConfirm` through `TaskCore__TaskCore`) is still
byte-exact. See `TaskCore__TaskCore.md` for the addendum recording this.

Beyond the correction:

- **`StreamTaskUnk78Obj`/`StreamTaskUnk78Methods` (from `TaskCore__OnDeinit`,
  earlier this round) is retired and folded into `StreamTaskUnkB4Obj`.**
  This function is five-way positive evidence that `unk48`, `unk74`,
  `unk78`, `unk7C`, and `unk80` are all the SAME generic sub-object class:
  every one of them is town down identically, by the SAME function, in
  sequence, through the SAME slot number (`+0x004`) that `StreamTaskUnkB4Obj`
  (`self->unkB4`) already had from `StreamTask__Finalize`. `StreamTaskUnkB4Methods`
  gained the former `StreamTaskUnk78Methods::slot50` as its own `+0x050`
  (both now point at the same code, `TaskCore__OnDeinit`'s call still compiles
  identically). This is a deliberate unification, not the project's default
  policy of keeping look-alike types separate — the default still applies to
  `TaskCoreObj`/`StreamTaskUnkCObj`, which have no such shared-slot evidence.
- New fields `unk70` (`+0x070`, `s32`, boolean guard) and `unk74` (`+0x074`,
  `StreamTaskUnkB4Obj *`).
- New `StreamTaskObjMethods` slot `+0x0DC`, called (not occupied) by this
  function: `gStreamTaskMethods+0x0DC = TaskCore__ReleaseTarget`, extern, void, confirmed to
  exist via `classtable.py gStreamTaskMethods`.
- New `TaskUtilMethods` slot `+0x00C` (`gIntermediateBaseMethods+0x00C =
  BasicClass__Finalize`, extern, void — the generic base-class slot name
  already seen elsewhere in this game).

## Third-learning check (per head's request)

**Not needed.** Every field access in this function is read exactly once,
with a `jalr` immediately following its own read and no re-read of the same
expression afterward. No locals were needed; matched on the first attempt.

## Proposed learning

**A field whose only known use so far is "assigned some call's return value"
carries NO type evidence on its own — wait for a DEREFERENCING use before
committing to a scalar type.** `unk48`/`unk7C`/`unk80` were typed `s32`
purely because nothing contradicted it at the time they were first written
(`TaskCore__TaskCore`); the very next function in ROM order dereferenced all
three as vtable objects. When a field is set from a call whose own return
type is otherwise unconstrained, treat the field's type as genuinely
open until a caller (or another accessor) proves it one way or the other —
and when the correction comes, retype-and-rebuild-full-unit is cheap
(seconds) and worth doing immediately rather than leaving two different
"true types" on record for the same bytes.

## Naming

**TaskCoreObj__Destroy** -- tier A. Occupies `gTaskCoreMethods`'s dtor slot
`+0x00C`; tears down five sub-objects then up-calls `IntermediateBase`'s own
dtor at the same slot. Same `Class__Destroy` convention as
`StreamTask__Finalize`.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from TaskCoreObj__Destroy (tools/rename.py). Occupant of +0x00C (`finalize`), named for the slot: releases bgLayer, tileMap, tileAtlas, `sound` when the ctor made it and `subHandle` when owned, runs releaseTarget, then IntermediateBase's finalize. The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, alpha)

bgLayer is a `BgLayer *` (include/bg_layer.h, was `BasicClass *`); `release` is the same inherited +0x004 slot. Source unchanged, byte-identical.

Later the same round (alpha, third class): TaskCore::tileAtlas (+0x080) is `struct TileAtlas *` (include/TileAtlas.h, was `BasicClass *`); its release is BasicClass's +0x004 slot, inherited unchanged by TileAtlasMethods. Byte-identical.
