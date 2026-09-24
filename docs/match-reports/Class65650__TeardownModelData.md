# Class65650__TeardownModelData

> Renamed from `func_80065C2C` on 2026-09-24 (tools/rename.py). Address 0x80065c2c.

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x0F8` (`slot_teardown5C`, called from the
destructor `Class65650__Destructor`). The teardown mirror of `Class65650__SetupModelData`'s setup:
if `self->unk5C` is set, tears it down via `Class65650__ReleaseModelData(self)`; otherwise
does nothing. Retail never explicitly sets `$v0` to a fixed value here (the
reload of `self->unk5C` for the guard already leaves it at 0 on the
not-taken path, and the call's own return overwrites it on the taken path),
consistent with the caller ignoring the return value — typed `void` here.

```c
void Class65650__TeardownModelData(Class65650 *self)
{
    if (self->unk5C != NULL) {
        Class65650__ReleaseModelData(self);
    }
}
```

Matched on the direct translation, no reshaping.

### Proposed learning

None beyond `Class65650__SetupModelData.md`'s (the setup/teardown pair share the same
guard shape).

## Naming

Round 75 (charlie), track 3.

- `Class65650__TeardownModelData` (was `func_80065C2C`), tier A. Occupies +0x0F8 (called by the Destructor). Calls ReleaseModelData only if modelData is set.
