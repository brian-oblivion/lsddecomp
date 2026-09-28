# RatioToFixed12 -- MATCHED 30/30 words

> Renamed from `func_8001EC84` on 2026-09-17 (tools/rename.py). Address 0x8001ec84.

Unit `SceneNode`, carved round 13. Reopened round 42 as `nop_mflo_mfhi`-blocked
(the blocker is RESOLVED, see CLAUDE.md); the stub above was never actually
attempted until now.

## Round 44 (echo)

20.12 fixed-point division, matching the header's own prediction
(`include/SceneNode.h`, the `Ratio16`-adjacent comment above the
`RatioToFixed12` prototype): `whole << 12 | frac`'s own division-derived low
bits, via the classic split-division idiom (divide once for
quotient+remainder, then divide the shifted remainder again for the
fractional part). Signature kept as the header already declares it
(`void *pair`) since that declaration is shared with `SceneNode.c`, a
sibling unit outside this runner's scope this round.

```c
s32 RatioToFixed12(void *pair) {
    Ratio16 *p;
    s32 q1, r1, q2;

    p = (Ratio16 *)pair;
    q1 = p->whole / p->frac;
    r1 = p->whole % p->frac;
    q2 = (r1 << 12) / p->frac;
    return (q1 << 12) + q2;
}
```

GCC 2.6.3 combines the `/` and `%` against the same operands into a single
`div` instruction (one `mflo`+`mfhi` pair), exactly reproducing retail's own
two-`div`-block shape with no `nop` inserted between the `mflo`/`mfhi` and
the following `div` -- confirming (again) that `--no-nop-mflo-mfhi` fully
resolves this construct with no C-side workaround needed.

Matched on the first build: **30/30 words**, whole-image SHA1 clean (`build
exit=0`). Zero attempts beyond this one.

## Proposed learning

The split-division idiom (`q = a/b; r = a%b; frac_q = (r << shift)/b; return
(q << shift) + frac_q;`) is a recognizable retail source shape for 20.12 (or
similar) fixed-point division on this soft-float target, and GCC 2.6.3 -O2
reliably fuses the `/`+`%` pair sharing operands into one `div`. Worth
recognizing on sight in any future `nop_mflo_mfhi`-reopened function that
shows the same two-`div`-block disassembly pattern.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EC84` -> `RatioToFixed12`. Tier A.** Free function, complete
  and visible semantics: it returns `(pair->whole << 12) / pair->frac` in
  20.12 fixed point, computed as a split division (one divide for quotient
  and remainder, a second for the shifted remainder) so the shift cannot
  overflow. Nothing about the name is inferred from context.
- **A finding the name exposes: the pair is a RATIO.** The inherited field
  names `whole`/`frac` on `Ratio16` (include/SceneNode.h) describe a
  mixed number; this body divides the first field BY the second, so they are
  numerator and denominator. Every producer in this unit
  (`SceneNode__GetRotationDegrees`, `SceneNode__FaceTarget`) writes a
  degrees value and a constant 1, which is consistent with both readings and
  is why the weaker one survived. Renaming the type and its two fields is
  left to track 4: `Ratio16` is also used by `src/DreamSys.c` and
  named in `include/Task.h`, outside this runner's unit.
- Signature kept as `void *pair`, as the shared header already declares it.


## Round 95 (bravo): moved from include/SceneNode.h

The header's banner was rewritten as documentation in round 95; the comment it carried about this function, verbatim:

```c
/* RatioToFixed12 (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c, not SceneNode, and the
 * nop_mflo_mfhi toolchain flag it was once blocked on is RESOLVED per
 * CLAUDE.md's "Open toolchain blockers" table; see
 * docs/match-reports/RatioToFixed12.md for the current history): reads
 * a `Ratio16` at the
 * given pointer and returns a 20.12 fixed-point value (`whole << 12 |
 * frac`'s own division-derived low bits) -- read off its own
 * disassembly (a `div` by the pair's own two fields, not decompiled
 * here). `SceneNode__UpdateScale`/`SceneNode__UpdateRotation` (both matched, this unit) apply
 * it three times in a row, at offsets +0x0/+0x4/+0x8 of their own 3rd
 * argument. Declared here with a `void *` argument since this unit's
 * chosen functions only ever pass the pointer through, never dereference
 * the pair themselves. */
```

## Round 98 (echo): track 7, moved from src/graphics/SceneNode.c

Locals `q1`/`r1`/`q2` -> `whole`/`rem`/`frac`, and `<< 12` -> `* ONE` (GCC emits the same `sll`): byte-identical.

The source comment was rewritten as documentation; the one it replaced, verbatim (field names as they were then):

```c
/* `(pair->num << 12) / pair->den` in 20.12 fixed point, computed as a
 * split division so the shift cannot overflow: quotient and remainder from
 * one divide, then the shifted remainder divided again. GCC 2.6.3 fuses the
 * `/` and `%` over the same operands into a single `div`.
 *
 * The pair is a ratio, numerator over denominator; every producer in this
 * unit sets `den` to 1. */
```
