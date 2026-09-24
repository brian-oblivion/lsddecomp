# Class65650__PlayTod

> Renamed from `func_800662A8` on 2026-09-24 (tools/rename.py). Address 0x800662a8.

**Unit:** code_55dd4 · **Size:** 3 words (0xC bytes) · **Status:** MATCHED
(3/3 words, whole-image `./build-and-verify.sh` green)

## What it does

Sets `self->unk90` (`+0x90`) to 1 and returns 1 — the same
"assignment-expression reuses the literal register" shape as
`Class65650__EnableTickCallback`, for the `+0x90` field instead of `+0x8C`.

```c
s32 Class65650__PlayTod(Class65650 *self)
{
    return self->unk90 = 1;
}
```

### Proposed learning

None beyond `Class65650__EnableTickCallback.md`'s (same shape, different field).
