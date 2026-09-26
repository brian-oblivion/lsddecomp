# Class86B60__RefreshViewValue -- MATCH

> Renamed from `func_8004DABC` on 2026-09-24 (tools/rename.py). Address 0x8004dabc.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__RefreshViewValue`: 23/23 words match.

## Source

```c
void Class86B60__RefreshViewValue(Class86B60 *self)
{
    s32 buf;

    Get_vtable_TaskCore()->slot94(self);
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
  Distinct from `Class86B60Methods::slot94` (established by
  `Class86B60__Tick`'s report) -- same offset number, unrelated table, no
  conflict.
- New type `Class86B60Unk60Obj_3bb8c_d` (self->unk60's pointee, only
  `unk14` reached, a plain `s32`).
- `Class86B60::unk60` -- new field, carved from the `pad05C` gap.
- `DreamSysViewMethods_3bb8c_c::slot19C` -- new slot (already added ahead
  of this function while deriving `Class86B60__Reset`'s neighbourhood; this
  is the function that actually exercises it).

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DABC` -> `Class86B60__RefreshViewValue`. **Tier B**: Calls the base class's `slot94`, copies `self->unk60->unk14` into a stack buffer, and forwards its address to `DreamSysView::slot19C` -- the same "read a value, push it through the view's slot19C out-parameter call" idiom that recurs in `Class86B60__UpdateMemcardSaveWithIcon` and `Class86B60__CommitNameEntry` in this same unit. Also the dispatch target for `Class86B60__Tick`'s case 1/case 4. Purpose of the value itself not established.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
