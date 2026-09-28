# SetActiveDataSourceDriverMode

> Renamed from `SetActiveDataSourceDriverMode` on 2026-09-18 (tools/rename.py). Address 0x80026f34.

**Unit:** GameApplicationFileResource · **Size:** 30 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 30/30 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/SetActiveDataSourceDriverMode.s`; matched on the first build.

## What it does

Picks a 3-argument function pointer by mode, then calls it in a `do`/`while`
loop until it returns nonzero, re-passing the SAME three arguments every
iteration (`$s1`/`$s2`/`$s3` hold the original `a0`/`a1`/`a2` for the whole
function; they never change):

```
lui $s0,%hi(SetNullDriverMode); addiu $s0,$s0,%lo(SetNullDriverMode)   # default fn
lw $v1,%gp_rel(sActiveDataSource)($gp); ori $v0,0x13
bne $v1,$v0,.L80026F74
  lui $s0,%hi(SetCdDriverMode); addiu $s0,$s0,%lo(SetCdDriverMode) # override fn
.L80026F74:
.L80026F78:
  jalr $s0(a0=s1,a1=s2,a2=s3)
  beqz $v0,.L80026F78
```

There's no separate "check-then-enter" -- the function falls straight into
the loop body, confirming a `do { } while (cond)` (not a pre-tested `while`),
matching CLAUDE.md's `SetActiveDataSourceDriverMode` prediction area for this unit's
accessor-family shape.

`SetNullDriverMode` is independently defined elsewhere
(`src/sound/PlacementGridVabSound.c`: `s32 SetNullDriverMode(s32 a, s32 b) { sNullDriverMode=a; sNullDriverModeArg=b; return 1; }`)
taking only **2** parameters, not 3. This unit's own local extern declares it
with 3 (matching the call site's actual register usage: `a0`,`a1`,`a2` are
all loaded before the `jalr`, since the alternate target `SetCdDriverMode` may
use the third). This is a deliberate "multiple independent local views"
declaration (CLAUDE.md), not an error: the real function ignores the extra
register, and the linker only checks the symbol name, not the prototype.

## Final body

```c
typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);
extern s32 SetNullDriverMode(s32 arg0, s32 arg1, s32 arg2);
extern s32 SetCdDriverMode(s32 arg0, s32 arg1, s32 arg2);

void SetActiveDataSourceDriverMode(s32 arg0, s32 arg1, s32 arg2) {
    DataSourceSetDriverModeFn fn;

    fn = SetNullDriverMode;
    if (sActiveDataSource == 0x13) {
        fn = SetCdDriverMode;
    }
    do {
    } while (fn(arg0, arg1, arg2) == 0);
}
```

## Proposed learning

**A "pick a function pointer by mode, then call it in a body-less
`do { } while (fn(constant args) == 0)` loop" idiom matched clean on the
first build with no register-pinning tricks needed**, because the three
loop-invariant arguments live in genuinely separate callee-saved registers
(`s1`/`s2`/`s3`) that never change across iterations -- contrast this with
`SetActiveDataSource` in the same unit, where the loop-carried value DOES change
every iteration and needs a different, harder-won source shape (see that
report).

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `SetActiveDataSourceDriverMode` | `SetActiveDataSourceDriverMode` | B |

**Evidence.** Picks `SetCdDriverMode` or the SPU-side `SetNullDriverMode` by
active source, then spins (`do { } while (fn(...) == 0)`) until the call
reports success. Same family as the Lock/Unlock/Is* wrappers, generalised to
a 3-argument setter with a retry loop.

### Type naming (round 97, bravo, track 6)

| was | now | tier |
| --- | --- | --- |
| `Func80026F34Fn` | `DataSourceSetDriverModeFn` | A |

**Evidence.** The typedef's only use is this function's local `fn`, and it
has exactly two occupants, one per branch of `sActiveDataSource ==
DATASOURCE_CD`: `SetCdDriverMode` (CD driver, `CdDriver.c`, 3 args) and
`SetNullDriverMode` (SPU/VAB driver, `PlacementGridVabSound.c`, 2 args, ignores the
third). Both occupants agree on what they do -- set the selected data
source's driver mode, returning 0 while not yet accepted, which the caller
polls. The name says that and nothing about why; the placeholder was the
address of this function (`func_80026F34`) before it was named.

## History moved from src/code_171e0.c (round 99, charlie, track 7)

This comment sat above the two driver-mode externs in `src/code_171e0.c`
until the track-7 pass. It is moved here unchanged:

> round 58 (alpha, externcheck.py): SetVabDriverMode's real definition
> (code_179d8_e.c) takes 2 args; SetCdDriverMode's (code_179d8_q.c)
> genuinely takes 3. Both are only ever REFERENCED here, never called
> directly -- `fn` dispatches through the shared 3-arg DataSourceSetDriverModeFn
> pointer type SetCdDriverMode needs, with SetVabDriverMode's own body
> simply not reading the 3rd word. Declaring SetVabDriverMode's own
> arity honestly (2, matching its definition) costs nothing byte-wise --
> a function-pointer VALUE assignment emits no argument-count-dependent
> code, just an address load -- and produces only a benign "incompatible
> pointer type" warning at the `fn = SetVabDriverMode;` line below.

Round 99 also named the parameters after SetCdDriverMode's definition:
`(async, mode2, useVSyncCallback)`, with `(async, mode2)` for SetVabDriverMode.
