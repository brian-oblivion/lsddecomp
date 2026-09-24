# Class65650__TeardownParts

> Renamed from `func_80065DEC` on 2026-09-24 (tools/rename.py). Address 0x80065dec.

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x104` (`slot_teardown70`). The teardown mirror
of `Class65650__SetupParts`, matching `Class65650__TeardownModelData`'s shape for the `+0x70` array
pair, deferring to `Class65650__DestroyParts(self)`.

```c
void Class65650__TeardownParts(Class65650 *self)
{
    if (self->unk70 != NULL) {
        Class65650__DestroyParts(self);
    }
}
```

Matched on the direct translation, no reshaping.

### Proposed learning

None beyond `Class65650__SetupModelData.md`'s.
