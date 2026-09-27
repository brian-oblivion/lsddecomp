# SetActiveDataSourceDriverMode

> Renamed from `SetActiveDataSourceDriverMode` on 2026-09-18 (tools/rename.py). Address 0x80026f34.

**Unit:** code_171e0 · **Size:** 30 words · **Status:** MATCHED, round 43
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
lui $s0,%hi(SetVabDriverMode); addiu $s0,$s0,%lo(SetVabDriverMode)   # default fn
lw $v1,%gp_rel(gActiveDataSource)($gp); ori $v0,0x13
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

`SetVabDriverMode` is independently defined elsewhere
(`src/code_179d8_e.c`: `s32 SetVabDriverMode(s32 a, s32 b) { gVabDriverMode=a; gVabDriverModeArg=b; return 1; }`)
taking only **2** parameters, not 3. This unit's own local extern declares it
with 3 (matching the call site's actual register usage: `a0`,`a1`,`a2` are
all loaded before the `jalr`, since the alternate target `SetCdDriverMode` may
use the third). This is a deliberate "multiple independent local views"
declaration (CLAUDE.md), not an error: the real function ignores the extra
register, and the linker only checks the symbol name, not the prototype.

## Final body

```c
typedef s32 (*DataSourceSetDriverModeFn)(s32, s32, s32);
extern s32 SetVabDriverMode(s32 arg0, s32 arg1, s32 arg2);
extern s32 SetCdDriverMode(s32 arg0, s32 arg1, s32 arg2);

void SetActiveDataSourceDriverMode(s32 arg0, s32 arg1, s32 arg2) {
    DataSourceSetDriverModeFn fn;

    fn = SetVabDriverMode;
    if (gActiveDataSource == 0x13) {
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

**Evidence.** Picks `SetCdDriverMode` or the SPU-side `SetVabDriverMode` by
active source, then spins (`do { } while (fn(...) == 0)`) until the call
reports success. Same family as the Lock/Unlock/Is* wrappers, generalised to
a 3-argument setter with a retry loop.
