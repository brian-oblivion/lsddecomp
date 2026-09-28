# TitleMenu__DestroySaveTitle -- MATCH

> Renamed from `TitleMenu__DestroyNameField` on 2026-09-26 (tools/rename.py). Address 0x8004dc08.

> Renamed from `Class86B60__DestroyNameField` on 2026-09-26 (tools/rename.py). Address 0x8004dc08.

> Renamed from `func_8004DC08` on 2026-09-24 (tools/rename.py). Address 0x8004dc08.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__DestroySaveTitle`: 23/23 words match.

## Source

```c
void TitleMenu__DestroySaveTitle(TitleMenu *self)
{
    self->nameField->methods->release(self->nameField);
    GetTaskCoreMethods()->slotDC(self);
}
```

First attempt, byte-exact. Same shape as `TitleMenu__Finalize` (this unit's
destructor): release an owned sub-object through the shared BasicClass-
family `release` slot, then forward to the base class table. This one has
no null check at all (unconditional release, no guard), and only one
sub-object.

## Struct changes (additive, `include/class_3bb8c.h`)

- `TitleMenu::nameField` -- new field, `GenericReleaseObj_3bb8c_d *`,
  reusing the same release-only view established by `TitleMenu__Finalize`.
  Carved from the `pad0B0` gap (now `pad0B4`).
- `BaseTaskCtorTable_3bb8c_c::slotDC` -- new slot, `void (*)(void *self)`.

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DC08` -> `TitleMenu__DestroySaveTitle`. **Tier B**: Unconditionally releases `self->nameField` (the sub-object `TitleMenu__CreateSaveTitle` constructs) then forwards to the base class's own `slotDC`. Mirror-image counterpart to `CreateNameField`; not the class's own destructor (that is `TitleMenu__Finalize`, a different base slot).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The releaseTarget override (+0x0DC). `nameField` is a TextRow; its release is BasicClass's. Byte-identical (whole image green, 0 new warnings, nonmatching green).
