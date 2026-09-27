# SetNdivOverride -- MATCHED (round 44, 6/6 words)

> Renamed from `SetPolyOtCodeOverride` on 2026-09-26 (tools/rename.py). Address 0x8001a54c.

> Renamed from `func_8001A54C` on 2026-09-24 (tools/rename.py). Address 0x8001a54c.

Unit `TmdRenderer`. Reopened round 42 after the `gp_rel` blocker that stalled
it at carve time (round 13) was resolved (`--gp-symbols`, see
`docs/research/gp-relative-blocker.md`). This report supersedes the round-13
stub, which recorded no attempt and no score.

## Result

Matched on the first build, no permuter needed.

```
SetNdivOverride: 6/6 words match (file 0xAD4C-0xAD64)
```

Whole-image `./build-and-verify.sh` passes (`build exit=0`).

## Derivation

The function is a straight-line two-field setter with no branches beyond an
early-out:

```
sw   $a0, %gp_rel(sNdivOverrideSet)($gp)
beqz $a0, .L8001A55C
 nop
sw   $a1, %gp_rel(sNdivOverride)($gp)
.L8001A55C:
jr   $ra
 nop
```

`sNdivOverrideSet` and `sNdivOverride` (`asm/data/7B018.sdata.s`) are plain `.sdata`
words, initialized to `1` and `2` respectively, and referenced from nowhere
else in the executable except this function and its `TmdRenderer` sibling
`FillDivPolygonHeader` (also assigned this round -- see `FillDivPolygonHeader.md`). No
existing type information anywhere else in the codebase constrains them
further, so they are declared `s32`.

```c
extern s32 sNdivOverrideSet;
extern s32 sNdivOverride;

void SetNdivOverride(s32 arg0, s32 arg1)
{
    sNdivOverrideSet = arg0;
    if (arg0) {
        sNdivOverride = arg1;
    }
}
```

`arg0` is stored unconditionally; `arg1` is stored only when `arg0` is
truthy, which the compiler collapses into a single `beqz` early-out (the
first store is not inside the guarded region because it does not depend on
the branch outcome). No struct or vtable involved -- this is not a case for
any GTE macro.

### Proposed learning

`sNdivOverrideSet`/`sNdivOverride` are declared locally in `src/TmdRenderer.c` (not in
`include/BMemPMgr.h`) per the project convention: nothing outside this unit
currently references them, so putting the extern in a shared header would
just be an unused collision surface for a sibling unit that never touches
them.

## Naming (round 77, alpha)

`func_8001A54C` -> `SetNdivOverride`, parameters (`arg0`, `arg1`) ->
(`enable`, `code`). **Tier A**: a two-field setter whose read side
(FillDivPolygonHeader, this unit) is fully derived -- `enable` gates whether
FillDivPolygonHeader's header word 0 comes from `code` (stored only when
`enable` is set) or the per-object D_80090C18 default. Matches the
globals it writes, `sNdivOverrideSet`/`sNdivOverride` (named
alongside this function).

## Round 91 polish (bravo)

Renamed from `SetPolyOtCodeOverride` (`python3 tools/rename.py
SetPolyOtCodeOverride SetNdivOverride`), with its globals
(`sNdivOverrideSet`, `sNdivOverride`). **Tier A.** The word
FillDivPolygonHeader takes from `sNdivOverride` is stored at DIVPOLYGON
`+0x0`, which is Sony's `ndiv` (number of subdivisions), not an OT code.
Parameter `code` -> `ndiv`. No caller anywhere in the image (no `jal`, no
table word holding 0x8001A54C).
