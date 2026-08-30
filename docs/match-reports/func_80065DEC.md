# func_80065DEC

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x104` (`slot_teardown70`). The teardown mirror
of `func_80065DBC`, matching `func_80065C2C`'s shape for the `+0x70` array
pair, deferring to `func_80065F2C(self)`.

```c
void func_80065DEC(Class65650 *self)
{
    if (self->unk70 != NULL) {
        func_80065F2C(self);
    }
}
```

Matched on the direct translation, no reshaping.

### Proposed learning

None beyond `func_80065BFC.md`'s.
