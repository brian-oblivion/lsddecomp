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
  of this function while deriving `Class86B60__ShowTitleIcon`'s neighbourhood; this
  is the function that actually exercises it).

### Proposed learning

None new.
