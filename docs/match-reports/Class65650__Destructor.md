# Class65650__Destructor

> Renamed from `func_8006573C` on 2026-09-24 (tools/rename.py). Address 0x8006573c.

**Unit:** code_55dd4 · **Size:** 21 words (0x54 bytes) · **Status:** MATCHED
(21/21 words, whole-image `./build-and-verify.sh` green)

## What it does

The destructor override for the class at `gClass65650Methods` (table slot `+0x00C`,
overriding the intermediate class `D_800878D4`'s own dtor at the same
offset). Calls this class's own teardown helper (`self->methods->slot0xF8`,
i.e. `Class65650__TeardownModelData`, the guarded teardown for the `+0x5C` sub-object — still
`INCLUDE_ASM` this round), then chains to the base class's dtor
(`DreamSys__GetBaseMethods()->dtor(self)`), matching the "destructor calls its own
cleanup, then the base dtor through the base table's own slot" idiom from
`docs/research/class-framework.md`.

```c
void Class65650__Destructor(Class65650 *self)
{
    self->methods->slot_teardown5C(self);
    DreamSys__GetBaseMethods()->dtor(self);
}
```

No reshaping needed; matched on the direct translation.

### Proposed learning

None beyond what `Class65650__Class65650.md` and `New_Class65650.md`
already record for this class.

## Naming

Round 75 (charlie), track 3.

- `Class65650__Destructor` (was `func_8006573C`), tier A. Occupies +0x00C (overrides Class6B5CC__Finalize): teardownModelData, then the base dtor. Named like Entity__Destructor, the subclass's own +0x00C.
