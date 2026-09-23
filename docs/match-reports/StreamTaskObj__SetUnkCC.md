# StreamTaskObj__SetUnkCC

> Renamed from `func_8003BE6C` on 2026-09-23 (tools/rename.py). Address 0x8003be6c.

**Unit:** code_2c054 · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkCC = value;`. Third of the run of five described in
`StreamTaskObj__SetUnkC4`'s report (same class, table slot `+0x12C`); see that report
for the shared context.

## Derivation

```
jr   $ra
 sw  $a1, 0xCC($a0)
```

```c
void StreamTaskObj__SetUnkCC(StreamTaskObj *self, s32 a1) {
    self->unkCC = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTaskObj__SetUnkC4`'s report — same header, `include/code_2c054.h`.

## Proposed learning

None beyond `StreamTaskObj__SetUnkC4`'s.
