# TitleMenu__TitleMenu

> Renamed from `Class86B60__Class86B60` on 2026-09-26 (tools/rename.py). Address 0x8004d578.

> Renamed from `func_8004D578` on 2026-09-22 (tools/rename.py). Address 0x8004d578.

**Unit:** class_3bb8c_c · **Size:** 64 words · **Status:** MATCHED (64/64)

## What it does

The ctor (slot +0x008) for the new `TitleMenu` sibling class (allocated by
`New_TitleMenu`, matched alongside this function). Chains to a base ctor
(`Get_vtable_TaskCore()->slot08`, 4 args), installs this class's own vtable
directly (rather than fetching it through another getter -- the base-class
constructor chaining pattern already documented for
`TaskCoreMethods::slotD8` in `include/code_2c054.h`), tears down an object
handed to it by the base ctor chain (`self->unk48`), stores its own
`dreamSys` argument, zeroes one field, makes two calls through `dreamSys`'s
own vtable (storing one result, forwarding the other's return value to the
still gp_rel-blocked `FormatNumberIntoBuffer`), then makes two more calls through
its own freshly-installed vtable.

## The C

```c
void TitleMenu__TitleMenu(TitleMenu *self, void *dreamSys)
{
    DreamSysView_3bb8c_c *dream;
    TitleMenuUnk48Obj *obj;

    Get_vtable_TaskCore()->slot08(self, &D_80086D44, &D_800114DC, 0);
    self->methods = GetTitleMenuMethods();
    obj = self->unk48;
    obj->methods->slot9C(obj, -1);
    self->unkA4 = dreamSys;
    self->unkAC = 0;
    dream = dreamSys;
    self->unkBC = dream->methods->slot1B0(dream, &self->unkC0);
    FormatNumberIntoBuffer(dream->methods->slot1A0(dream, 0));
    self->methods->slotD8(self, &D_80086D44);
    self->methods->slot40(self, dreamSys);
}
```

## New header content (`include/class_3bb8c.h`)

All new declarations -- this class (`TitleMenu`) had no prior C-level
presence anywhere in the project. Placed as one new block right before the
`Ctx678_3bb8c_c`/`CheckSaveScoreFlag` section, since ROM order puts
`New_TitleMenu`/`TitleMenu__TitleMenu` right before `CheckSaveScoreFlag`.

- `TitleMenu`/`TitleMenuMethods`: `ctor` (+0x008, this function),
  `slot40` (+0x040, this function's own last call), `slotD8` (+0x0D8, this
  function's own second-to-last call). Vtable is `gTitleMenuMethods`, resolved
  via `GetTitleMenuMethods` (still raw asm in the uncarved
  `asm/class_3bb8c_d.s`, called directly by `jal` -- same "vtable getter"
  shape as `GetNodeGuardedViewportMethods`/`GetGridCellMethods`).
- `TitleMenu` struct fields: `unk48` (`TitleMenuUnk48Obj *`, set up by
  the base ctor chain, read here), `unkA4` (`void *`, stores `dreamSys`
  raw), `unkAC` (`s32`, zeroed), `unkBC` (`s32`, holds `slot1B0`'s return),
  `unkC0` (`s32`, an output buffer whose ADDRESS is passed to `slot1B0` --
  sized to exactly one word because it is the last word of the 0xC4-byte
  allocation: `0xC0 + 4 == 0xC4`, `New_TitleMenu`'s own alloc size, which
  is why this is a confident field boundary and not a guess).
- `TitleMenuUnk48Obj`/`TitleMenuUnk48ObjMethods`: opaque, only `slot9C`
  (+0x09C, this function's own call, arg `-1`) typed. Deliberately a
  SEPARATE local type from `include/code_2c054.h`'s `StreamTaskUnkB4Obj`
  (also a base-ctor-chain output at the same +0x048 offset on ITS class)
  since that type's own known slots don't include +0x09C and nothing ties
  the two classes together -- same independent-view policy used
  throughout this file.
- `DreamSysView_3bb8c_c`/`DreamSysViewMethods_3bb8c_c`: a LOCAL, minimal
  opaque view of `dreamSys`, typing only `slot1A0` (+0x1A0) and `slot1B0`
  (+0x1B0), the two slots this function reaches. The project already has a
  large canonical `DreamSys` type in `include/DreamSys.h` with its own
  `vt` field, but neither offset is established there yet, and this unit
  does not edit that header -- kept local per this project's established
  independent-view convention.
- `BaseTaskCtorTable_3bb8c_c`: this function's own local view of
  `Get_vtable_TaskCore`'s return type, typing `slot08` (+0x008) as a 4-argument
  call (`self, arg1, arg2, arg3`). **This is the SAME real global,
  `gTaskCoreMethods`, as `include/code_2c054.h`'s already-established
  `TaskCoreMethods`** (that header's own comment documents `Get_vtable_TaskCore`
  returning `&gTaskCoreMethods`, and its `slot08` is independently confirmed
  3-argument-plus-self by a byte-exact call in `StreamTask__StreamTask`, matching
  THIS call site's arity exactly -- both units agree on the real arity
  here, unlike the `GetSceneNodeMethods` situation elsewhere in this file). Kept
  as a separate local declaration rather than `#include "code_2c054.h"`,
  since each translation unit in this project gets its own extern
  prototype for a given external symbol and this unit does not otherwise
  need that header.
- `D_80086D44`, `D_800114DC`: address-of-only placeholder `s32` globals
  (same convention as this file's existing `sDefaultTargetSpecs`).
- `FormatNumberIntoBuffer` prototype: `extern void FormatNumberIntoBuffer(s32 arg0);` -- the
  unit's own still-blocked (gp_rel) function; needed here only as a
  forward declaration so this function can call it. Return value unused at
  this call site, hence `void`.

## Notes

Matched on the first attempt, no residue. Statement order in the C mirrors
retail's instruction order exactly (including reading `self->unk48` before
overwriting `self->methods`, and computing `dream->methods->slot1B0(...)`
before the `FormatNumberIntoBuffer` forward, matching the retail read-then-store and
call-then-call sequencing word-for-word).

## Proposed learning

> **A "base ctor sets self->methods directly rather than fetching it
> through a getter" ctor can still dispatch through `self->methods`
> immediately afterward in the SAME function** (here, `slotD8` and
> `slot40` right after `self->methods = GetTitleMenuMethods();`) -- write it as
> a plain sequential assignment-then-dispatch; no getter re-fetch or
> caching is needed for the match.

## Naming

**TitleMenu__TitleMenu** -- tier A. Canonical ctor (`Class__Class`
convention). Same evidence class as the other two ctors in this unit:
occupies `ctor` (+0x008) on `gTitleMenuMethods`, chains a base ctor,
installs its own vtable directly (`self->methods = ...`, the
"base-ctor-chain sets self->methods directly" pattern already documented
for `TaskCoreMethods::slotD8`), then dispatches through the freshly
installed table twice more in the same function (`slotD8`, then
`onConstruct` -- see the header-edit note below).

## Header edit: TitleMenuMethods::slot40 -> onConstruct

Renamed following the compiler-ownership recipe (FINISHING-PLAN.md track
3 step 3), not assumed safe: `TitleMenuMethods` is otherwise SHARED with
`src/class_3bb8c_d.c` (most of its other slots are dispatched from
functions there). Renamed the field in the struct DEFINITION alone,
rebuilt, and the compiler's error was confined to this unit's own call
site (`src/class_3bb8c_c.c`, this function's own last statement) --
nothing in `class_3bb8c_d.c` or anywhere else references this specific
slot. Fixed the one call site, oracle green. Same name and same evidence
shape ("runs right after self->methods is installed") as
`NodeGuardedViewportMethods::onConstruct`, which this unit's other ctor
(`NodeGuardedViewport__NodeGuardedViewport`) already established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The body now reads at TaskCore's names: `unk48` is TaskCore's `sound`, called at +0x09C through VabStreamObj (setPitchOffset, as GraphRoom's ctor does); `slotD8` is setTarget, with D_80086D44 retyped TaskCoreTarget; `onConstruct` (+0x040) is resetCounters, called through TitleMenuResetCallFn because the call passes dreamSys and the slot (and TitleMenu__Reset) take self alone; `unkBC`/`unkC0` are `saveBlock` (`s32 *`, getSaveBlock's result, no cast now) and `saveBlockSize`. Byte-identical (whole image green, 0 new warnings, nonmatching green).
