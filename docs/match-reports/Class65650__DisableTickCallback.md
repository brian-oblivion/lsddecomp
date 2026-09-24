# Class65650__DisableTickCallback

> Renamed from `func_80066148` on 2026-09-24 (tools/rename.py). Address 0x80066148.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk8C` (`+0x8C`) to 0. `$v0` is never touched, so the return
value is unused (`void`). The "set to 1 and return 1" sibling is
`Class65650__EnableTickCallback`.

```c
void Class65650__DisableTickCallback(Class65650 *self)
{
    self->unk8C = 0;
}
```

### Proposed learning

None; plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `Class65650__DisableTickCallback` (was `func_80066148`), tier A. Occupies +0x114; clears tickCallbackEnabled. Called by InitDefaults and Entity__StopSoundCue.
