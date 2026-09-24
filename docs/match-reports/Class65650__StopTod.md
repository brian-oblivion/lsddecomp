# Class65650__StopTod

> Renamed from `func_800662B4` on 2026-09-24 (tools/rename.py). Address 0x800662b4.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk90` (`+0x90`) to 0. `$v0` is never touched, so the return
value is unused (`void`). The "set to 1 and return 1" sibling is
`Class65650__PlayTod`.

```c
void Class65650__StopTod(Class65650 *self)
{
    self->unk90 = 0;
}
```

### Proposed learning

None; plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `Class65650__StopTod` (was `func_800662B4`), tier A. Occupies +0x130; clears todPlaying. Called by InitDefaults and Entity__StopSoundCue.
