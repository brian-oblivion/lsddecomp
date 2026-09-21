# RatioToFixed12 -- MATCHED 30/30 words

> Renamed from `func_8001EC84` on 2026-09-17 (tools/rename.py). Address 0x8001ec84.

Unit `code_d294_c`, carved round 13. Reopened round 42 as `nop_mflo_mfhi`-blocked
(the blocker is RESOLVED, see CLAUDE.md); the stub above was never actually
attempted until now.

## Round 44 (echo)

20.12 fixed-point division, matching the header's own prediction
(`include/code_d294.h`, the `WholeFrac_d294`-adjacent comment above the
`RatioToFixed12` prototype): `whole << 12 | frac`'s own division-derived low
bits, via the classic split-division idiom (divide once for
quotient+remainder, then divide the shifted remainder again for the
fractional part). Signature kept as the header already declares it
(`void *pair`) since that declaration is shared with `code_d294.c`, a
sibling unit outside this runner's scope this round.

```c
s32 RatioToFixed12(void *pair) {
    WholeFrac_d294 *p;
    s32 q1, r1, q2;

    p = (WholeFrac_d294 *)pair;
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
  names `whole`/`frac` on `WholeFrac_d294` (include/code_d294.h) describe a
  mixed number; this body divides the first field BY the second, so they are
  numerator and denominator. Every producer in this unit
  (`Class6B5CC__GetRotationDegrees`, `Class6B5CC__FaceTarget`) writes a
  degrees value and a constant 1, which is consistent with both readings and
  is why the weaker one survived. Renaming the type and its two fields is
  left to track 4: `WholeFrac_d294` is also used by `src/DreamSys.c` and
  named in `include/code_2cc8c.h`, outside this runner's unit.
- Signature kept as `void *pair`, as the shared header already declares it.
