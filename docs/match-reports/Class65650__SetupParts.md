# Class65650__SetupParts

> Renamed from `func_80065DBC` on 2026-09-24 (tools/rename.py). Address 0x80065dbc.

**Unit:** code_55dd4 · **Size:** 12 words (0x30 bytes) · **Status:** MATCHED
(12/12 words, whole-image `./build-and-verify.sh` green)

## What it does

`Class65650Methods` slot `+0x100` (`slot_setup70`). The same guard/defer
shape as `Class65650__SetupModelData`, but for the `+0x70`/`+0x74` array pair instead of
`+0x5C`, and deferring to `Class65650__CreateParts` instead of `Class65650__AcquireModelData`. Unlike
`Class65650__AcquireModelData`, `Class65650__CreateParts` never references its own `$a1` (confirmed
by reading its body: it only ever writes outgoing `$a1`, never reads an
incoming one), so this dispatcher takes a single `self` parameter, not two.

```c
s32 Class65650__SetupParts(Class65650 *self)
{
    if (self->unk70 != NULL) {
        return 0;
    }
    return Class65650__CreateParts(self);
}
```

Matched on the direct translation — same zero-residue result as
`Class65650__SetupModelData`, for the same reason (see that report's note on the
`goto` lever: the fall-through arm's return value is the callee's own
`$v0`, nothing to materialize).

### Proposed learning

None beyond `Class65650__SetupModelData.md`'s.

## Naming

Round 75 (charlie), track 3.

- `Class65650__SetupParts` (was `func_80065DBC`), tier A. Occupies +0x100 (AcquireModelData's success path). Returns 0 if parts is already set, otherwise CreateParts.
