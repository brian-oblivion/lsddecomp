# Class866E8__SetCallback

> Renamed from `func_8004ADC4` on 2026-09-22 (tools/rename.py). Address 0x8004adc4.

**Unit:** class_3ac78 · **Size:** 3 words · **Status:** MATCHED (3/3 words)

## What it does

`Class866E8`'s slot +0x0C8 setter: stores its two arguments into
`self->unk60` and `self->unk64`.

## Derivation

```
sw $a1, 0x60($a0)
jr $ra
 sw $a2, 0x64($a0)
```

A two-field setter, both `s32` (plain `sw`, no shift/sign-extend). Field
offsets and the `Class866E8` type come from `include/class_3ac78.h`
(established this round; see `Class86668__PlaySound.md` for how the class was
identified via `tools/classtable.py gClass866E8Methods`).

## Proposed learning

None beyond what's already documented for `Class866E8` in `Class86668__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ADC4` | `Class866E8__SetCallback` | A | Occupant of vtable slot `+0x0C8`; a two-word setter whose stores land at `+0x060` and `+0x064`. `class_3bb8c`'s MATCHED `Class866E8__ComputeRateEntry` invokes exactly that pair as `unk60(unk64, value, 0, 0)` and stores the result -- so the first word is a function pointer and the second its context. A pure setter whose mechanics are its purpose. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Class866E8+0x060` | `valueFn` | A | Called as a function pointer by the matched sibling above. |
| `Class866E8+0x064` | `valueFnCtx` | A | Passed as that call's first argument and never dereferenced anywhere. |

The two fields keep their `s32` declared type here (this unit only stores raw
words into them); `class_3bb8c.h` already carries the function-pointer typing.
Unifying the two declarations is track-4 work.
