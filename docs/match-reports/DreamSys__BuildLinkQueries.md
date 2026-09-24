# DreamSys__BuildLinkQueries -- STALL (best 13/116 words, structurally correct)

NON_MATCHING body promoted, round 74.

> Renamed from `func_80057784` on 2026-09-19 (tools/rename.py). Address 0x80057784.

Unit: `src/class_3bb8c_p.c`. Class: `DreamSys` family -- plain internal
helper, not a vtable slot. Called by this unit's own `DreamSys__FindNearbyLink`
(also still queued/stalled -- see its own report, which depends on this
one).

## What's confirmed correct

The CONTROL FLOW and DATA FLOW are fully reverse-engineered and believed
correct (cross-checked against `m2ctx.py`'s own independent translation,
which agrees on every branch and field access). What's stalled is purely
REGISTER-ALLOCATION/SCHEDULING matching -- a genuine residue, not a
misread of the logic. ~20 attempts were spent narrowing it; stopping here
per the 30-attempt budget with a documented near-miss rather than
burning the rest chasing the same residue class.

## Signature

```c
s32 DreamSys__BuildLinkQueries(DreamSys *self, GridQuery *arr1, GridArrElem **arr2, LinkQueryBuf *arg3, s32 arg4);
```

New types this function needed, declared once (shared with
`DreamSys__FindNearbyLink`, `DreamSys__ScanLinkCandidates`, `DreamSys__ScanGridWindow`) above `DreamSys__FindNearbyLink`
in this file: `LinkQueryBuf` (`arg3`'s type -- `s8 queryCol`/`queryRow`, `GridArrElem
*source`), and `DreamSysUnk4C68Obj`/`DreamSysUnk4CObj::unk_0x68` (added to
`include/DreamSys.h`, additive). `DreamSysUnk4CMethods::queryLinkAtPos`/
`getGridArrElemAt` were also added there (splitting the existing
`pad_0x110[0x11C-0x110]`).

## Best-reached body (13/116, preserved for the next attempt)

```c
#if 0
s32 DreamSys__BuildLinkQueries(DreamSys *self, GridQuery *arr1, GridArrElem **arr2, LinkQueryBuf *arg3, s32 arg4) {
    s32 f2 = arg3->queryCol;
    s32 f3 = arg3->queryRow;
    s32 oddCount = (arg4 & 1) ? arg4 : arg4 + 1;
    s32 idx = 1;

    if (oddCount == 1) {
        DreamSysUnk4CObj *unk4C;
        DreamSysUnk4C68Obj *unk68;
        GridArrElem *src;
        s16 s3;
        s32 pos;

        arr1[0].startCol = (s16) f2;
        arr1[0].startRow = (s16) f3;
        arr1[0].numCols = oddCount;
        arr1[0].numRows = oddCount;
        src = arg3->source;
        arr2[0] = src;
        unk4C = self->unk_0x4C;
        unk68 = unk4C->unk_0x68;
        if (unk68->unk_0x4 != 1) {
            return 1;
        }
        s3 = src->info->unk32;
        pos = s3 + 1;
        if (pos < unk68->unk_0x2) {
            idx = 2;
            arr2[1] = unk4C->methods->getGridArrElemAt(unk4C, pos, f3, src);
            arr1[1] = arr1[0];
        }
        pos = s3 - 1;
        if (pos >= 0) {
            arr2[idx] = unk4C->methods->getGridArrElemAt(unk4C, pos, f3, src);
            arr1[idx] = arr1[0];
            idx++;
        }
        return idx;
    }

    {
        s32 t0 = oddCount;
        s32 vv0 = oddCount;
        s32 a1 = f2;
        s32 a2 = f3;

        if (f2 != 0) {
            a1 = f2 - 1;
        } else {
            t0 = oddCount - 1;
        }
        if (f2 == 0x13) {
            t0 -= 1;
        }
        if (f3 != 0x13) {
            a2 = f3 + 1;
        } else {
            vv0 -= 1;
        }
        arr1[0].startCol = (s16) a1;
        if (f3 == 0) {
            vv0 -= 1;
        }
        arr1[0].startRow = (s16) a2;
        arr1[0].numCols = t0;
        arr1[0].numRows = vv0;
        arr2[0] = arg3->source;
        return 1;
    }
}
#endif
```

This body is semantically the exact translation of the disassembly
(verified line-by-line against `asm/nonmatchings/class_3bb8c_p/DreamSys__BuildLinkQueries.s`
and cross-checked with `m2ctx.py`), truncates and stores every field
correctly, and takes the right branches. It compiles clean and scores
13/116 (frame `-0x30` vs retail's `-0x28`, one extra callee-saved
register throughout -- `s5`/`s6` where retail uses none).

## Residues found and their fixes (all applied above; what's LEFT is
listed after)

1. **`s8`-typed locals for a truncated struct byte produce `lbu`
   (unsigned) loads; `s32`-typed locals holding the same field produce
   `lb` (signed).** Confirmed with a standalone reproducer through the
   pinned toolchain: identical code, only the LOCAL's declared width
   changed, and the emitted load instruction changed with it. Since the
   loaded byte is later STORED into a `s16` field (`arr1[0].startCol = ...`),
   it must be properly sign-extended by then; `s32 f2 = arg3->queryCol;`
   (not `s8`) is what gives `lb`, matching retail. Cost: went from
   completely unrelated (0/116, with a further-drifted frame) to a
   recognizable near-miss.
2. **Caching `arg3->queryCol`/`queryRow` into a local ONCE and reusing it across
   both branches is correct** (retail's own disassembly shows the SAME
   `t2`/`t1` registers, loaded once at function entry, used in BOTH the
   `oddCount==1` branch and the `else` branch) -- this is the opposite of
   the project's usual "don't cache, re-read" lesson, and confirmed by
   direct inspection of retail's own instruction stream, not guessed.

## What's LEFT unresolved (the actual residue)

- **An extra register (`s5`/`s6`) survives throughout the whole
  function**, most visibly holding `arg2` (this function's own 2nd
  parameter, `arr2`) -- retail keeps it in `s4`, this version in `s6`,
  and everything downstream is renamed accordingly. Two extra
  callee-saved registers are live compared to retail's 5 (`s0`-`s4`);
  this version uses 7 (`s0`-`s6`).
- **The `oddCount` ternary's OWN codegen differs even in isolation.**
  Retail's disassembly for `arg4 & 1 ? arg4 : arg4+1` is NOT the compact
  form GCC normally emits for a ternary/if-else with a shared result
  variable -- it goes through an extra `j`/`move v1,v0`/`move v0,v1`
  chain, ending with v0 AND v1 both holding the final value in SEPARATE
  registers (a genuine redundant copy, not simplified away). Every
  reshaping tried (ternary, if/else with one shared var, if/else with
  two explicitly-named vars mirroring `v0`/`v1`) either reproduced the
  SAME compact form mine already has, or made the match WORSE (dropped
  to 0/116). This suggests the value feeding the `== 1` check is NOT
  simply `oddCount` reused, but something reconstructed from a genuinely
  different expression not yet identified.
- **The second `getGridArrElemAt` call re-materializes `f3`/`src` into
  `a2`/`a3`**, even though (per retail's own disassembly) those
  registers should already hold the right values unchanged from the
  first call. Casting the field to a 2-argument function-pointer type at
  the second call site (to avoid re-listing all 4 arguments) was tried
  and made the score WORSE (8/116) -- the cast itself cost bytes
  elsewhere. Whatever keeps `a2`/`a3` untouched between the two calls in
  retail is not reproduced by any variant tried.

## Axes tried (for whoever picks this back up)

- Local variable TYPE for the truncated bytes (`s8` vs `s32`) -- RESOLVED,
  keep `s32`.
- Whether `f2`/`f3` are cached once (shared across both branches) or
  re-declared per-branch -- tried both; per-branch declaration (the body
  above) scores best, but caching-once was tried too (worse, 4/116) after
  a related restructuring, so isolate this axis again before trusting
  that number.
- Ternary vs. explicit if/else, and one-variable vs. two-variable
  (`v0`/`v1`-mirroring) forms for `oddCount` -- three variants tried, none
  beat the ternary.
- Function-pointer cast at the second `getGridArrElemAt` call site to suppress
  argument re-materialization -- tried once, worse.

**Not yet tried:** varying the SHARED-prep hypothesis more carefully (only
`t0`/`a1` were unified across branches in one attempt; `a2` and the `v0`/`v1`
pair were not independently isolated), and checking whether `arg2`
(`arr2`) needs to be threaded through as a genuinely different C type
(e.g. `void **` instead of `GridArrElem **`) to change its register
class. Given `arr2`'s the field holding the extra register, that's the
next axis worth a fresh session's attempts.

## Naming

**`DreamSys__BuildLinkQueries` -- tier B, STALL.** Control and data flow
are independently confirmed (cross-checked against `m2ctx.py`, byte
residue is register-allocation only): builds one or two `GridQuery` +
`GridArrElem*` pairs from a `LinkQueryBuf` position query, optionally
consulting an adjacent-cell lookup (`getGridArrElemAt`) when a neighbour
object's `unk_0x68->unk_0x4` flag reads `1`. "Build link queries" names
what the function assembles for its caller (`DreamSys__FindNearbyLink`)
to scan; still a STALL, so kept as a mechanics-only tier B name per
CLAUDE.md ("a wrong tier-A name is worse than a placeholder").

## Verify

```
./build-and-verify.sh   # build exit=0 with DreamSys__BuildLinkQueries restored to INCLUDE_ASM
tools/funcdiff.py DreamSys__BuildLinkQueries   # still INCLUDE_ASM -- do not trust a "full match" reading, it is retail-vs-retail
```

## Round: re-verified from raw asm, claim confirmed accurate (runner delta, round 19)

Re-derived this function's control flow directly from
`asm/nonmatchings/class_3bb8c_p/DreamSys__BuildLinkQueries.s`, instruction by
instruction, as a check against the kind of misread that turned out to be
real elsewhere this round (`DreamSys__SoundCueCallback`). No discrepancy found -- every
branch, field offset, and the two `getGridArrElemAt` call sites (including the
confirmed fact that the SECOND call reuses `f3`/`src` in `$a2`/`$a3`
unchanged from the first call's setup, rather than retail re-loading them)
match this report's existing C exactly.

Also rebuilt the exact preserved body into `src/class_3bb8c_p.c` directly
(not just re-read) to confirm the score claim itself: **13/116, frame
`-0x30` vs retail's `-0x28`, confirmed accurate.** No hidden bug found, no
new axis tried beyond what this report already records exhausted (the
`void **`/type-class idea for `arr2` flagged above as the next lead was
not attempted this round -- it requires touching the caller
`DreamSys__FindNearbyLink`'s buffer types too, and this round's time budget went to
functions with a clearer path to closing). Left as-is, `INCLUDE_ASM`
restored, no source changes.

## Round: NON_MATCHING body promoted (runner delta, round 74)

Placed the preserved body from this report into `src/class_3bb8c_p.c` under
`#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` (track 1b). Hand-derived,
not permuter-searched -- confirmed by this report itself (round 19's
instruction-by-instruction re-derivation, and the original derivation's own
residue analysis above), so no separate review was needed beyond checking
current symbol/field names.

One rename needed: `self->unk_0x4C` is now `self->linkMgr` in
`include/DreamSys.h` (renamed since this report was written); the promoted
body uses the current name. Every other name the body references
(`DreamSysUnk4CObj`, `DreamSysUnk4C68Obj`, `unk_0x68`, `unk_0x4`, `unk_0x2`,
`getGridArrElemAt`, `LinkQueryBuf::queryCol/queryRow/source`) already matches
`include/DreamSys.h` as committed; `tools/stalesyms.py` found nothing stale
for this function.

Re-measured the score with the body compiled live (temporarily, in place of
`INCLUDE_ASM`, then reverted): still 13/116 in-range, confirming the figure
above, but `funcdiff.py` flags the reading untrustworthy (98230 bytes differ
outside the function's window) because this body is not length-exact, so
swapping it in shifts every following symbol. That drift is expected and
harmless for track 1b -- the verified build never compiles this branch -- so
the comment cites the already-confirmed 13/116 figure rather than this
drifted re-read.

`./build-and-verify.sh` exit=0, unchanged (build/SLPS_015.56 still matches
retail byte-exact -- this promotion changed no linked bytes).
`tools/check-nonmatching.sh` exit=0, green (47 NON_MATCHING bodies in 16
units compile and resolve, including this one).
