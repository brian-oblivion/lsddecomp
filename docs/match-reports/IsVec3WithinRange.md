> Renamed from `func_8001EF14` on 2026-09-17 (tools/rename.py). Address 0x8001ef14.

# IsVec3WithinRange -- MATCHED (19/19 words)

Unit: `code_d294_c` (round 14). A standalone leaf, not yet reached by any
caller in this round's queue -- a 3-element range check: returns 1 if
every `b[i]` is within `[a[i]-range, a[i]+range]`, 0 as soon as one isn't.
`s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b)`.

## Final source

```c
s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b) {
    s32 i;

    for (i = 0; i < 3; i++, a++, b++) {
        if (*b < *a - range) {
            return 0;
        }
        if (*a + range < *b) {
            return 0;
        }
    }
    return 1;
}
```

## Derivation notes

- **Residue: `for (i = 0; i < 3; i++)` with array-indexed `a[i]`/`b[i]`
  access scored 16/19, with the increment instructions in the wrong
  order.** Retail's loop-continuation block is `t0++ (i); a0+=4 (a++);
  [slti test]; bnez; a2+=4 (b++, in the branch's delay slot)`. My indexed
  version let GCC choose its own strength-reduction increment order,
  which put `b`'s pointer bump (`a2+=4`) FIRST instead of last. Writing
  the loop with explicit pointer walks in the `for`'s own increment
  clause (`i++, a++, b++`, matching retail's own grouping -- `i`/`a`
  together, `b` deferred to the branch delay slot as the one truly
  independent-of-the-test increment) reproduced the exact order.
- No named struct fits either `a`/`b` array -- same situation as
  `SubVec3S16` (a 3-element `s32` vector pair), and plausibly related
  to it (a range/tolerance check against the SAME kind of 3-axis data),
  but that connection is not asserted, only noted, since no caller in
  this round's queue reaches this function to confirm it.

No new struct or vtable-slot knowledge.

### Proposed learning

**When a `for` loop walks two or more independent pointers alongside its
counter, write ALL of them in the loop's own increment clause
(`i++, a++, b++`) rather than indexing (`a[i]`, `b[i]`) and letting the
compiler infer the pointer walk.** GCC 2.6.3's own strength-reduction
order for indexed access does not necessarily match retail's -- here it
reordered which pointer bump landed in the branch's delay slot. Explicit
walks in the increment clause let the SOURCE dictate the grouping
directly, matching this project's existing preference for reproducing
retail's literal shape over the "cleaner" idiomatic form.

## Naming (round 50, charlie -- FINISHING-PLAN track 3)

- **`func_8001EF14` -> `IsVec3WithinRange`. Tier A.** Free function,
  complete semantics: returns 1 when every component of `b` lies within
  +/- `range` of the matching component of `a`, 0 on the first component
  that does not. `Is...` marks the predicate; the two early returns and the
  trailing `return 1` are the whole body.
- Corroborated by its one caller: `DreamSys.c`'s `func_8005942C` calls it
  as `IsVec3WithinRange(local, tolerance, reference)`, where `local` is a
  position it has just computed via `Class6B5CC__LocalOffsetToWorldPos` --
  a tolerance test between two positions, which is what the name says.
- Parameters left as `(a, range, b)`: the test is symmetric in `a` and `b`,
  so naming one "actual" and the other "expected" would assert a direction
  the code does not have. The explicit pointer walks in the `for`
  increment clause are load-bearing (see the derivation above) and unchanged.
