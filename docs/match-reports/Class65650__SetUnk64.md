# Class65650__SetUnk64

> Renamed from `func_80065BF4` on 2026-09-24 (tools/rename.py). Address 0x80065bf4.

**Unit:** code_55dd4 · **Size:** 2 words (0x8 bytes) · **Status:** MATCHED
(2/2 words, whole-image `./build-and-verify.sh` green)

## What it does

A trivial setter: stores its second argument into `self->unk64` (`+0x64` of
the `Class65650` object). Retail never sets `$v0` in this function, so its
return value is treated as unused (`void`).

```c
void Class65650__SetUnk64(Class65650 *self, s32 value)
{
    self->unk64 = value;
}
```

### Proposed learning

None; this is the plain single-instruction setter shape.

## Naming

Round 75 (charlie), track 3.

- `Class65650__SetUnk64` (was `func_80065BF4`), tier B. Occupies +0x0F0 (first slot of this class's own range); a one-line setter of +0x64. InitDefaults sets it to 1 and TickCallbackA acts only while it is 1; what the value means is not known, so the field keeps its offset name.
