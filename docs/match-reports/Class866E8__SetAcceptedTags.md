# Class866E8__SetAcceptedTags

> Renamed from `func_8004ADD0` on 2026-09-22 (tools/rename.py). Address 0x8004add0.

**Unit:** class_3ac78 · **Size:** 2 words · **Status:** MATCHED (2/2 words)

## What it does

`Class866E8`'s slot +0x0CC setter: stores its argument into `self->unkE8`.

## Derivation

```
jr $ra
 sw $a1, 0xE8($a0)
```

A one-field `s32` setter, leaf, tail instruction in the delay slot of `jr`.

## Proposed learning

None beyond what's already documented for `Class866E8` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ADD0` | `Class866E8__SetAcceptedTags` | A | Occupant of vtable slot `+0x0CC`; a one-word setter into `+0x0E8`. The field's only reader is `Class866E8__ForwardAcceptedCommand` in this same unit, which walks it as a NUL-terminated array of vtable header words and uses a match to decide whether to act on a sender. The name is that reader's contract, stated once. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Class866E8+0x0E8` | `acceptedTags` | A | Sole reader establishes it exactly: a NUL-terminated list of class header words that gates sender acceptance. |
