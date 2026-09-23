# StreamTaskObj__SetUnkC8

> Renamed from `func_8003BE64` on 2026-09-23 (tools/rename.py). Address 0x8003be64.

**Unit:** code_2c054 · **Size:** 2 instructions (0x8 bytes) · **Status:** MATCHED (2/2 words, whole-image SHA1 green), first attempt

## What it does

Plain setter: `self->unkC8 = value;`. Second of the run of five described in
`StreamTaskObj__SetUnkC4`'s report (same class, table slot `+0x128`); see that report
for the shared context (class identity, table-slot derivation, why these are
setters and not BIOS trampolines).

## Derivation

```
jr   $ra
 sw  $a1, 0xC8($a0)
```

```c
void StreamTaskObj__SetUnkC8(StreamTaskObj *self, s32 a1) {
    self->unkC8 = a1;
}
```

Matched first attempt.

## New struct/header knowledge

See `StreamTaskObj__SetUnkC4`'s report — same header, `include/code_2c054.h`.

## Proposed learning

None beyond `StreamTaskObj__SetUnkC4`'s.

## Naming

**StreamTaskObj__SetUnkC8** -- tier A. Plain setter, second of the run of
five described in `StreamTaskObj__SetUnkC4`'s report; same convention.
