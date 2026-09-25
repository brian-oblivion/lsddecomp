# GetGraphRoomStreamChannel -- MATCHED (39/39 words)

> Renamed from `func_800493E4` on 2026-09-25 (tools/rename.py). Address 0x800493e4.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on build 12; whole-image SHA1 green, funcdiff 39/39.

## What it does

`rec = GetCinematicBank(&count, n)`; sums `gStreamTypeToGroupTable[i] + 10` (s16 table) over
`i` in `[count, count + len*2)` into `*total`, subtracts 10 (so: widths plus
a 10-unit gap between entries), returns `rec`.

## Source

Declarations: `Rec1C` and `extern s16 gStreamTypeToGroupTable[];` in `src/code_39094.c`.

```c
Rec1C *GetGraphRoomStreamChannel(s32 *total, s32 n, s32 len) {
    s32 count;
    s32 i;
    s32 start;
    Rec1C *rec = GetCinematicBank(&count, n);
    len *= 2;
    *total = 0;
    start = count;
    len += start;
    for (i = start; i < len; i++) {
        *total += gStreamTypeToGroupTable[i] + 10;
    }
    *total -= 10;
    return rec;
}
```

## Levers (12 builds)

- Loop body: `*total = *total + 10 + D[i]` reassociates to `(D+10)+*total`
  (30/39); `*total += D[i] + 10` gives retail's `(*total+10)+D` (35/39).
- Prologue: `count` has its address taken and `*total = 0` may alias it, so it
  is re-read after the store. Retail reads it once into `$v1`, copies it to the
  loop counter AND adds it into the register that held `len*2`. Only
  `start = count;` (a named copy read after `*total = 0`), `len += start`
  (end reuses len's register), `for (i = start; ...)` gives that.
  Misses: `end = count + len*2` (35, addu uses the counter copy),
  `end = ...` before `*total = 0` (33, count loaded twice), `for (; count < end; count++)`
  and `i < count + len*2` in the condition (length changes), `end = start + len` in a
  new variable (32, end gets its own register), `i < start + len` (30).

### Proposed learning

When retail loads an address-taken local once and uses it both as a loop
counter's start and in the bound, copy it to a named local after the last
aliasing store, and fold the bound into the register of the variable it
extends (`len += start`) rather than a fresh `end`.
