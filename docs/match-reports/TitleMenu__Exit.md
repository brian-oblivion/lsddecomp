# TitleMenu__Exit -- MATCH

> Renamed from `TitleMenu__RefreshViewValue` on 2026-09-28 (tools/rename.py). Address 0x8004dabc.

> Renamed from `Class86B60__RefreshViewValue` on 2026-09-26 (tools/rename.py). Address 0x8004dabc.

> Renamed from `func_8004DABC` on 2026-09-24 (tools/rename.py). Address 0x8004dabc.

Unit `title_menu`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py TitleMenu__Exit`: 23/23 words match.

## Source

```c
void TitleMenu__Exit(TitleMenu *self)
{
    s32 buf;

    GetTaskCoreMethods()->slot94(self);
    buf = self->unk60->unk14;
    self->unkA4->methods->slot19C(self->unkA4, &buf);
}
```

First attempt, byte-exact. Straightforward transcription: a call to the
shared base table, then a one-word value copied from `self->unk60` into a
stack-local buffer whose address is forwarded to `self->unkA4`
(the `DreamSysView_3bb8c_c`)'s own `slot19C`.

## Struct changes (additive, `include/class_3bb8c.h`)

- `BaseTaskCtorTable_3bb8c_c::slot94` -- new slot, `void (*)(void *self)`.
  Distinct from `TitleMenuMethods::slot94` (established by
  `TitleMenu__ConfirmSlot`'s report) -- same offset number, unrelated table, no
  conflict.
- New type `TitleMenuUnk60Obj_3bb8c_d` (self->unk60's pointee, only
  `unk14` reached, a plain `s32`).
- `TitleMenu::unk60` -- new field, carved from the `pad05C` gap.
- `DreamSysViewMethods_3bb8c_c::slot19C` -- new slot (already added ahead
  of this function while deriving `TitleMenu__Reset`'s neighbourhood; this
  is the function that actually exercises it).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DABC` -> `TitleMenu__Exit`. **Tier B**: Calls the base class's `slot94`, copies `self->unk60->unk14` into a stack buffer, and forwards its address to `DreamSysView::slot19C` -- the same "read a value, push it through the view's slot19C out-parameter call" idiom that recurs in `TitleMenu__SaveToCard` and `TitleMenu__RefreshMenu` in this same unit. Also the dispatch target for `TitleMenu__ConfirmSlot`'s case 1/case 4. Purpose of the value itself not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

TitleMenu is unified in include/TitleMenu.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). `unk60->unk14` is TaskCore's `slotCounts[5]` (s32 *, +0x060); the TitleMenuUnk60Obj_3bb8c_d view is gone. Byte-identical (whole image green, 0 new warnings, nonmatching green).

## Track 7 (round 96, echo)

Local `buf` -> `shake` (it carries SHAKE's setting to DreamSys);
`slotCounts[5]` -> `slotCounts[TITLEMENU_SHAKE]`.
