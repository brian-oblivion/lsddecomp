# TitleMenu__TitleMenu

> Renamed from `Class86B60__Class86B60` on 2026-09-26 (tools/rename.py). Address 0x8004d578.

> Renamed from `func_8004D578` on 2026-09-22 (tools/rename.py). Address 0x8004d578.

**Unit:** TitleMenuTaskObjF · **Size:** 64 words · **Status:** MATCHED (64/64)

## What it does

The ctor (slot +0x008) for the new `TitleMenu` sibling class (allocated by
`New_TitleMenu`, matched alongside this function). Chains to a base ctor
(`Get_vtable_TaskCore()->slot08`, 4 args), installs this class's own vtable
directly (rather than fetching it through another getter -- the base-class
constructor chaining pattern already documented for
`TaskCoreMethods::slotD8` in `include/Task.h`), tears down an object
handed to it by the base ctor chain (`self->unk48`), stores its own
`dreamSys` argument, zeroes one field, makes two calls through `dreamSys`'s
own vtable (storing one result, forwarding the other's return value to the
still gp_rel-blocked `StampSaveTitleDay`), then makes two more calls through
its own freshly-installed vtable.

## The C

```c
void TitleMenu__TitleMenu(TitleMenu *self, void *dreamSys)
{
    DreamSysView_3bb8c_c *dream;
    TitleMenuUnk48Obj *obj;

    Get_vtable_TaskCore()->slot08(self, &sTitleMenuTarget, &sTitleMenuSoundBankPath, 0);
    self->methods = GetTitleMenuMethods();
    obj = self->unk48;
    obj->methods->slot9C(obj, -1);
    self->unkA4 = dreamSys;
    self->unkAC = 0;
    dream = dreamSys;
    self->unkBC = dream->methods->slot1B0(dream, &self->unkC0);
    StampSaveTitleDay(dream->methods->slot1A0(dream, 0));
    self->methods->slotD8(self, &sTitleMenuTarget);
    self->methods->slot40(self, dreamSys);
}
```

## New header content (`include/class_3bb8c.h`)

All new declarations -- this class (`TitleMenu`) had no prior C-level
presence anywhere in the project. Placed as one new block right before the
`Ctx678_3bb8c_c`/`UpdateFlashbackLock` section, since ROM order puts
`New_TitleMenu`/`TitleMenu__TitleMenu` right before `UpdateFlashbackLock`.

- `TitleMenu`/`TitleMenuMethods`: `ctor` (+0x008, this function),
  `slot40` (+0x040, this function's own last call), `slotD8` (+0x0D8, this
  function's own second-to-last call). Vtable is `gTitleMenuMethods`, resolved
  via `GetTitleMenuMethods` (still raw asm in the uncarved
  `asm/TitleMenuTaskObjF.s`, called directly by `jal` -- same "vtable getter"
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
  SEPARATE local type from `include/Task.h`'s `StreamTaskUnkB4Obj`
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
  `gTaskCoreMethods`, as `include/Task.h`'s already-established
  `TaskCoreMethods`** (that header's own comment documents `Get_vtable_TaskCore`
  returning `&gTaskCoreMethods`, and its `slot08` is independently confirmed
  3-argument-plus-self by a byte-exact call in `StreamTask__StreamTask`, matching
  THIS call site's arity exactly -- both units agree on the real arity
  here, unlike the `GetSceneNodeMethods` situation elsewhere in this file). Kept
  as a separate local declaration rather than `#include "Task.h"`,
  since each translation unit in this project gets its own extern
  prototype for a given external symbol and this unit does not otherwise
  need that header.
- `sTitleMenuTarget`, `sTitleMenuSoundBankPath`: address-of-only placeholder `s32` globals
  (same convention as this file's existing `sDefaultTargetSpecs`).
- `StampSaveTitleDay` prototype: `extern void StampSaveTitleDay(s32 arg0);` -- the
  unit's own still-blocked (gp_rel) function; needed here only as a
  forward declaration so this function can call it. Return value unused at
  this call site, hence `void`.

## Notes

Matched on the first attempt, no residue. Statement order in the C mirrors
retail's instruction order exactly (including reading `self->unk48` before
overwriting `self->methods`, and computing `dream->methods->slot1B0(...)`
before the `StampSaveTitleDay` forward, matching the retail read-then-store and
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
`src/ui/TitleMenuTaskObjF.c` (most of its other slots are dispatched from
functions there). Renamed the field in the struct DEFINITION alone,
rebuilt, and the compiler's error was confined to this unit's own call
site (`src/ui/TitleMenuTaskObjF.c`, this function's own last statement) --
nothing in `TitleMenuTaskObjF.c` or anywhere else references this specific
slot. Fixed the one call site, oracle green. Same name and same evidence
shape ("runs right after self->methods is installed") as
`NodeGuardedViewportMethods::onConstruct`, which this unit's other ctor
(`NodeGuardedViewport__NodeGuardedViewport`) already established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The body now reads at TaskCore's names: `unk48` is TaskCore's `sound`, called at +0x09C through VabStreamObj (setPitchOffset, as GraphRoom's ctor does); `slotD8` is setTarget, with sTitleMenuTarget retyped TaskCoreTarget; `onConstruct` (+0x040) is resetCounters, called through TitleMenuResetCallFn because the call passes dreamSys and the slot (and TitleMenu__Reset) take self alone; `unkBC`/`unkC0` are `saveBlock` (`s32 *`, getSaveBlock's result, no cast now) and `saveBlockSize`. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 6 (2026-09-26, round 93, delta): the class is TitleMenu

`python3 tools/renametype.py Class86B60 TitleMenu` (was named by its table's
address, gClass86B60Methods at 0x80086B60). Tier A for the class name, from
data the class itself reads:

- its TaskCoreTarget, sTitleMenuTarget, has `names` = D_80086CF8 = "START",
  "FLASHBACK", "SAVE", "LOAD", "GRAPH", "SHAKE" (asm/data/7B12C.sdata.s,
  1C34.rodata.s), and TitleMenu__Reset sets the backdrop to "ETC\TITLE.TIM";
- tick's cases agree with that order: 1 (FLASHBACK) opens a flashback session
  (DreamSys getSetFlashbackSession(0, 1)), 2 (SAVE) calls saveCtrl's
  beginSave, 3 (LOAD) its beginLoad, 4 (GRAPH) returns 2, and
  GameApplication__RunTitleMenu runs GraphRoom again on a 2;
- refreshViewValue stores slotCounts[5] (SHAKE, the one slot with an item
  list, unk24[5] = D_80086CA8) through getSetScreenShake;
- registrationSlots D_80086CDC = {0, 1, 0, ...}: FLASHBACK starts locked, and
  UpdateFlashbackLock (called from RefreshMenu with `self->target`) writes
  registrationSlots[1];
- the TextRow shows gSaveTitle's buffer, 0x8001149C, SJIS "LSD   Day001",
  the memory-card save title; StampSaveTitleDay writes the day at +0x12.

Method renames, each its own `rename.py` commit (tier B: mechanics named,
from the body and the menu entry that reaches them):

| old | new | evidence |
| --- | --- | --- |
| UpdateMemcardSaveWithIcon | SaveToCard | tick's SAVE case; calls saveCtrl's beginSave |
| UpdateMemcardSaveStatus | LoadFromCard | tick's LOAD case; calls saveCtrl's beginLoad |
| BeginMemcardSave | BeginCardAccess | run before both save and load: makes saveCtrl, gives it input |
| EndMemcardSave | EndCardAccess | undoes BeginCardAccess on saveCtrl's 0x16/0x17 |
| OnTagBValue | OnCardEvent | onNotify's case for a sender whose class id ends 0xB (TaskObjF) |
| CommitNameEntry | RefreshMenu | no name is entered here: it reloads the title text, FLASHBACK's lock, the widgets and SHAKE's cursor |
| CreateNameField | CreateSaveTitle | the TextRow is the save title, not an editable name |
| DestroyNameField | DestroySaveTitle | releaseTarget override |
| ForwardToNameField | AttachSaveTitle | updateSlotElements override: attachToParent |
| TickNameFieldCursor | CycleSaveTitleColor | broadcastToSlots override: one colour channel a frame |

Header edit (one commit): the six own slots take the methods' names, and the
fields `nameField` -> `saveTitle`, `iconHandle` -> `saveIcon`; all their
accessors are in src/ui/TitleMenuTaskObjF.c. The old banner's history ("unified
round 88", "Named by its table's address") is this section and the Track 4
sections above.

Known, pending an operator decision: renametype.py and rename.py rewrote the
old names inside the history prose of this class's reports, so earlier
sections read with today's names.

## Round 94 (track 6, charlie): history moved from include/class_3bb8c.h

The header's comments on this function's data carried their history; the
comments now say only what the data is. Moved here:
- `D_80086D44` (the menu description, a TaskCoreTarget: 0x28 bytes in
  asm/data/76DC8.data.s, a path word, three zero words, the two colour
  triples and four pointers), `D_800114DC` ("ETC\ETCSE") and `sTitleTimPath`
  ("ETC\TITLE.TIM") were RETYPED from placeholder `s32`s in round 88
  (address-of only; no byte change).
- The `FormatNumberIntoBuffer` prototype's comment still called it raw asm,
  gp-relative-blocked, and its argument `slot1A0`'s result. It has been
  matched since round 45 (gp_rel resolved round 42), and the argument is
  DreamSys's getCurrentDayAndYear.
- The unrelated section markers ("class_3bb8c_c additions below ... Named
  by their vtable's address") and the pointers "X is defined in
  include/X.h (round 87/88/89, track 4)" for NodeGuardedViewport, GridCell,
  TitleMenu, TaskObjF, ItemList, TextEntry, ObjM and gObjMMethods are
  replaced by the unit-to-class table in the header's banner.

## Naming (round 100, track 7)

- `sTitleMenuTarget` (was `D_80086D44`), tier A: the TaskCoreTarget this
  ctor passes both to TaskCore's ctor and to setTarget, TitleMenu's menu
  description. `s`: only this unit's code reads it.
- `sTitleMenuSoundBankPath` (was `D_800114DC`), tier A: "ETC\ETCSE", the
  `soundBankPath` TaskCore's ctor makes `sound` from; same form as
  GraphRoom's `sGraphSoundBankPath`.

## Track 7 (round 100)

The local `DreamSys *dream = dreamSys;` alias is gone and the two DreamSys
calls go through `dreamSys` directly: `struct DreamSys *` and `DreamSys *`
are one type, and the image is byte-identical. The null `sound` argument,
the `saveCtrl` clear and getCurrentDayAndYear's `outYear` are `NULL`;
setPitchOffset(-1) is commented (octave -1: pitchOffset = -1 * 12 - 24 =
-36 semitones, VabStreamObj.h).
