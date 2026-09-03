# func_8004EEA0

**Unit:** class_3bb8c_f · **Size:** 51 words (0xCC) · **Status:** STALL —
register-identity PERMUTATION at zero address drift, best 33/51

## What it does

`s32 func_8004EEA0(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5,
s32 arg6, s32 arg7)`. The same bounded-retry shape as `func_8004ED40`
(this unit, matched): calls `func_800507F8(handle, a1)` once up front,
then retries `func_8004EF6C(self, a1, handle, a3 & 0xFF, arg5, arg6,
arg7)` (this unit, also a predicted-hard stall — see its own report) up
to 11 times via the identical `do { ...; if (result) break; } while
(count-- != 0);` idiom, and — only if every attempt returned 0 — calls
`func_800507F8(handle, 0)` before returning the last result.

Register census: **9 distinct callee-saved registers, fully saturated**
(`$fp`/`$s8` + `$s0`-`$s7` — confirmed with the `$fp`-vs-`$s8` naming
caveat from CLAUDE.md, both `objdump`'s `s8` rendering and splat's `$fp`
counted as the same register). Blocker screen clean (no `gp_rel`, no
`addiu $at,$at,%lo`, no jump table).

## The residue

**Every branch target, every field/argument, and the total compiled
LENGTH are settled** — the best attempt compiles to exactly 51 words with
**zero address drift** (no "differs outside this range" warning), the
same "confirmed zero-drift register PERMUTATION" signature CLAUDE.md
documents for `func_8004C93C` in the `class_3bb8c_b`/`_c` header family.
Retail's own register assignment: `self`→`$fp`, `a1`→`$s3`, `handle`→
`$s2`, `a3`→`$s7`, `arg5`→`$s6`, `arg6`→`$s5`, `arg7`→`$s4`, plus `$s1`
(retry counter) and `$s0` (result). The best attempt reached (preserved
below) assigns the identical SET of 9 values to the identical 9
registers, but permuted: `self`→`$s4` (not `$fp`), `a3`→`$s5` (not
`$s7`), `arg5`→`$s8`, `arg6`→`$s7`, `arg7`→`$s6` — a genuine permutation,
not a missing or extra value.

## What was tried

1. **Direct transcription** (the exact `func_8004ED40` retry idiom,
   proven correct there). Score 31/51, full permutation as described
   above.
2. **Reordering the two independent top-of-function statements**
   (`count = 10;` before vs. after the initial `func_800507F8(handle,
   a1);` call) — moved the score from 31/51 to 33/51 (2 more words) but
   did NOT change which register any of the 9 live values landed in; the
   2 extra matching words are incidental instruction-encoding overlap,
   not progress on the permutation itself.
3. **Local-declaration order** (`count`/`result` swapped) — reverted
   immediately, made things worse (back to 31/51).

Given `func_8004C93C`'s own report (this same header-family class,
documented in CLAUDE.md) already tried declaration-order and
branch-structure reshaping across 7 variants with an isolated
sub-second reproducer and never moved a SINGLE value off its
mis-assigned register, and this function's own attempt 3 reproduces
that same non-responsiveness on a much smaller scale (2 variants, one
direction), further reshaping attempts were not pursued past the two
above — see CLAUDE.md's explicit finding that this residue class is
"not fixable by reshaping."

## Preserved near-miss body (`#if 0`, best variant, 33/51, zero drift)

```c
#if 0
s32 func_8004EEA0(TaskObjF *self, s32 a1, s32 handle, s32 a3, s32 arg5, s32 arg6, s32 arg7) {
    s32 count;
    s32 result;

    count = 10;
    func_800507F8(handle, a1);
    do {
        result = func_8004EF6C(self, a1, handle, a3 & 0xFF, arg5, arg6, arg7);
        if (result != 0) {
            break;
        }
    } while (count-- != 0);
    if (result == 0) {
        func_800507F8(handle, 0);
    }
    return result;
}
#endif
```

Needs a forward declaration of `func_8004EF6C` (defined later in this
unit's ROM order; already present near the top of `src/class_3bb8c_f.c`)
and `func_800507F8`'s extern (declared in `include/class_3bb8c.h`).

## Header additions (`include/class_3bb8c.h`, additive only)

- `extern s32 func_800507F8(s32 handle, s32 flag);` — **an extern for a
  function outside this unit**, typed from this call site's own register
  usage (the pre-loop call's `a1` is EEA0's OWN incoming `a1` parameter,
  left untouched in its hardware register rather than re-set, which is
  how its value was recovered without a caller elsewhere in this unit to
  cross-check against).

## Proposed learning

**A second confirmed instance of the zero-drift register-PERMUTATION
class outside the `class_3bb8c_b`/`_c` header family it was first
documented in** (this is `class_3bb8c_f`) — this residue is not specific
to one header's functions; it recurs whenever a function has ~8-9
simultaneously-live values and GCC 2.6.3 happens to pick a different
(but equally valid) bijection from values to registers than retail's
source did. Worth flagging in `docs/DECOMPILATION_LEARNINGS.md`'s
existing entry on this class as a THIRD confirmed instance, now spanning
two units.
