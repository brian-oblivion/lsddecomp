# TitleMenu__AttachSaveTitle -- MATCH

> Renamed from `TitleMenu__ForwardToNameField` on 2026-09-26 (tools/rename.py). Address 0x8004dc64.

> Renamed from `Class86B60__ForwardToNameField` on 2026-09-26 (tools/rename.py). Address 0x8004dc64.

> Renamed from `func_8004DC64` on 2026-09-24 (tools/rename.py). Address 0x8004dc64.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__AttachSaveTitle`: 27/27 words match.

## Source

```c
void TitleMenu__AttachSaveTitle(TitleMenu *self, s32 arg1)
{
    GetTaskCoreMethods()->slotE0(self, arg1);
    self->nameField->methods->slot4C(self->nameField, arg1, &sSaveTitleOffset);
}
```

First attempt, byte-exact. Two calls: base table forward, then a second
slot on `self->nameField` (the sub-object `TitleMenu__DestroySaveTitle` releases).

## Struct changes (additive, `include/class_3bb8c.h`)

- `TitleMenu::nameField` **retyped** from the generic
  `GenericReleaseObj_3bb8c_d *` (set by `TitleMenu__DestroySaveTitle`'s report) to a new
  dedicated `TitleMenuUnkB0Obj_3bb8c_d *`, which carries BOTH the shared
  `release` slot at `+0x004` (same signature as before, so
  `TitleMenu__DestroySaveTitle`'s call site is unaffected) and this function's own
  `+0x04C` slot. Done because `iconHandle`/`unkAC` never reach a second slot
  and there is no evidence they share this fuller shape -- keeping
  `nameField` on its own local view avoids projecting one instance's richer
  interface onto the other two.
- `BaseTaskCtorTable_3bb8c_c::slotE0` -- new slot, `void (*)(void *self,
  s32 arg1)`.
- New `extern s32 sSaveTitleOffset;` (address-of only, placeholder type).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DC64` -> `TitleMenu__AttachSaveTitle`. **Tier B**: Forwards `arg1` to the base class's `slotE0`, then dispatches `arg1` and a fixed global (`&sSaveTitleOffset`) through `self->nameField`'s own `slot4C`. Purpose of the forwarded value/event not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The updateSlotElements override (+0x0E0). Its argument is typed as the slot's `void *parent`; the name field's +0x04C is TextRow's attachToParent (SceneNode's), so the call casts parent to SceneNode * and &sSaveTitleOffset to LongVec3 * (its offset). No code from either cast. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 7 (round 96, echo)

Naming: `D_8008A9B4` -> `sSaveTitleOffset` (tier A: the position
attachToParent places the save title at, -4, -23). Retyped `s32` ->
`struct ScreenSpritePos` in include/class_3bb8c.h: a TextRow's position
is a ScreenSpritePos (include/text_row.h's banner) passed through
SceneNode's LongVec3 slot, so the `(LongVec3 *)` cast stays.
