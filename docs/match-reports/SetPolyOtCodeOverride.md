# SetPolyOtCodeOverride -- MATCHED (round 44, 6/6 words)

> Renamed from `func_8001A54C` on 2026-09-24 (tools/rename.py). Address 0x8001a54c.

Unit `code_8220_c`. Reopened round 42 after the `gp_rel` blocker that stalled
it at carve time (round 13) was resolved (`--gp-symbols`, see
`docs/research/gp-relative-blocker.md`). This report supersedes the round-13
stub, which recorded no attempt and no score.

## Result

Matched on the first build, no permuter needed.

```
SetPolyOtCodeOverride: 6/6 words match (file 0xAD4C-0xAD64)
```

Whole-image `./build-and-verify.sh` passes (`build exit=0`).

## Derivation

The function is a straight-line two-field setter with no branches beyond an
early-out:

```
sw   $a0, %gp_rel(gPolyOtCodeOverrideSet)($gp)
beqz $a0, .L8001A55C
 nop
sw   $a1, %gp_rel(gPolyOtCodeOverride)($gp)
.L8001A55C:
jr   $ra
 nop
```

`gPolyOtCodeOverrideSet` and `gPolyOtCodeOverride` (`asm/data/7B018.sdata.s`) are plain `.sdata`
words, initialized to `1` and `2` respectively, and referenced from nowhere
else in the executable except this function and its `code_8220_c` sibling
`FillRCPolyHeader` (also assigned this round -- see `FillRCPolyHeader.md`). No
existing type information anywhere else in the codebase constrains them
further, so they are declared `s32`.

```c
extern s32 gPolyOtCodeOverrideSet;
extern s32 gPolyOtCodeOverride;

void SetPolyOtCodeOverride(s32 arg0, s32 arg1)
{
    gPolyOtCodeOverrideSet = arg0;
    if (arg0) {
        gPolyOtCodeOverride = arg1;
    }
}
```

`arg0` is stored unconditionally; `arg1` is stored only when `arg0` is
truthy, which the compiler collapses into a single `beqz` early-out (the
first store is not inside the guarded region because it does not depend on
the branch outcome). No struct or vtable involved -- this is not a case for
any GTE macro.

### Proposed learning

`gPolyOtCodeOverrideSet`/`gPolyOtCodeOverride` are declared locally in `src/code_8220_c.c` (not in
`include/code_8220.h`) per the project convention: nothing outside this unit
currently references them, so putting the extern in a shared header would
just be an unused collision surface for a sibling unit that never touches
them.
