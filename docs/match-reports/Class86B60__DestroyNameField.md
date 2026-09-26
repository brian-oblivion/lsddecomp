# Class86B60__DestroyNameField -- MATCH

> Renamed from `func_8004DC08` on 2026-09-24 (tools/rename.py). Address 0x8004dc08.

Unit `class_3bb8c_d`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py Class86B60__DestroyNameField`: 23/23 words match.

## Source

```c
void Class86B60__DestroyNameField(Class86B60 *self)
{
    self->nameField->methods->release(self->nameField);
    Get_vtable_TaskCore()->slotDC(self);
}
```

First attempt, byte-exact. Same shape as `Class86B60__Finalize` (this unit's
destructor): release an owned sub-object through the shared BasicClass-
family `release` slot, then forward to the base class table. This one has
no null check at all (unconditional release, no guard), and only one
sub-object.

## Struct changes (additive, `include/class_3bb8c.h`)

- `Class86B60::nameField` -- new field, `GenericReleaseObj_3bb8c_d *`,
  reusing the same release-only view established by `Class86B60__Finalize`.
  Carved from the `pad0B0` gap (now `pad0B4`).
- `BaseTaskCtorTable_3bb8c_c::slotDC` -- new slot, `void (*)(void *self)`.

### Proposed learning

None new.

## Naming (round 77, naming runner delta)

Renamed `func_8004DC08` -> `Class86B60__DestroyNameField`. **Tier B**: Unconditionally releases `self->nameField` (the sub-object `Class86B60__CreateNameField` constructs) then forwards to the base class's own `slotDC`. Mirror-image counterpart to `CreateNameField`; not the class's own destructor (that is `Class86B60__Finalize`, a different base slot).

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 88, bravo)

Class86B60 is unified in include/Class86B60.h (TASKCORE_SLOTS/TASKCORE_FIELDS plus its own). The releaseTarget override (+0x0DC). `nameField` is a TextRow; its release is BasicClass's. Byte-identical (whole image green, 0 new warnings, nonmatching green).
