# RegisterFileTableEntries

> Renamed from `func_80027024` on 2026-09-18 (tools/rename.py). Address 0x80027024.

**Unit:** GameApplicationFileResource · **Size:** 34 words · **Status:** MATCHED, round 43
(2026-09-15, runner bravo). 34/34 words, byte-exact whole-image build.

## History

Never attempted (round-2026-08-29-a/30-a stub, `gp_rel`-blocked before any
derivation). Round 42 resolved `gp_rel`. Round 43 derived this fresh from
`asm/nonmatchings/code_171e0/RegisterFileTableEntries.s`.

**First attempt (1 word too long) -- a branch-polarity mistake, not a
`gp_rel` residue.** The natural first reading is an early-return guard:

```c
if (gActiveDataSource != 0x13) {
    return 1;
}
<work>
return ResolveFileEntries(...);
```

This compiles 35 words, one over: GCC places the small "return 1" arm as
the FALLTHROUGH of the `bne` and the large "work" arm at the branch target,
which means the work arm (ending in a tail call) needs an extra `j` to skip
over the "return 1" arm and reach the shared epilogue. Retail's actual
layout is the opposite: `beq` branches on the EQUAL condition straight to
the work arm, which sits immediately before the epilogue with no jump
needed, while the fallthrough ("not equal") is the cheap 2-word `j
epilogue; li v0,1`. The fix is to write the condition the way the branch
actually reads it -- positive, wrapping the work, with the constant return
falling out the bottom:

```c
if (gActiveDataSource == 0x13) {
    <work>
    return ResolveFileEntries(...);
}
return 1;
```

Same logic, byte-exact. **General lesson: when a big/small pair of
`if`-arms differ in size, whichever arm the compiler places LAST (adjacent
to the epilogue) needs no trailing jump, and retail's branch condition
polarity tells you which arm that is -- `beq`/`bne` to the branch TARGET is
the arm placed away from the epilogue only when it is also the shorter one
placed at the fallthrough; check both, don't assume the "early return"
shape is neutral.** This one-word residue reads exactly like a stray extra
instruction (easy to misattribute to something exotic); it was pure branch
polarity.

## What it does

Mode-gated: if `sActiveDataSource == 0x13`, sets `gFileTableRegistered = 1`, forwards
`arg0` to `SetFileTable`, fetches an index via `GetFileTableCount()`, adds it
to `arg1` and forwards to `SetFileTableCount`, then tail-calls `ResolveFileEntries`
with `arg0` advanced by `idx * 0x1C` (28 bytes -- computed by retail via
`sll x3; subu; sll x2` = `x8 - x1`, then `x4`, the classic shift/subtract
strength-reduction for a multiply by 28, i.e. `arg0[idx]` over a 28-byte
element). Otherwise returns `1`. All four callees
(`SetFileTable`/`FF0`/`FE4`/`FFC`) are still uncarved
(`asm/code_179d8.s`).

## Final body

```c
extern s32 gFileTableRegistered;
extern void SetFileTable(void *arg0);
extern s32 GetFileTableCount(void);
extern void SetFileTableCount(s32 arg0);
extern s32 ResolveFileEntries(void *arg0, s32 arg1);

s32 RegisterFileTableEntries(void *arg0, s32 arg1) {
    s32 idx;

    if (sActiveDataSource == 0x13) {
        gFileTableRegistered = 1;
        SetFileTable(arg0);
        idx = GetFileTableCount();
        SetFileTableCount(idx + arg1);
        return ResolveFileEntries((u8 *) arg0 + idx * 0x1C, arg1);
    }
    return 1;
}
```

## Proposed learning

**Promoted to the top-level report because it cost a real attempt and the
cause is general, not local to this function.** A 1-word (or few-word)
overshoot on a function with an unbalanced `if`-arm size (one arm much
bigger than the other) is worth checking for inverted branch polarity
BEFORE assuming a deeper residue: rewrite the condition to match the
`beq`/`bne` target's polarity directly, putting the LARGER arm at the
branch target (so it lands next to the epilogue, no trailing jump) and the
smaller arm as the plain fallthrough. This cost one wasted build in round
43; the fix was a straight swap of `if`/`else` polarity with identical
logic.

## Naming

Round 52 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80027024` | `RegisterFileTableEntries` | B |

**Evidence.** When the CD driver is active: installs the caller's array as
the file table, advances the running entry count by `arg1`, and resolves
the new entries' disc positions via `ResolveFileEntries`; otherwise a no-op
returning `1`. Mechanics fully derived from the body and the callees'
already-established names (`SetFileTable`/`GetFileTableCount`/
`SetFileTableCount`/`ResolveFileEntries`).

## Naming (round 99, charlie, track 7)

| was | now | tier |
| --- | --- | --- |
| `D_8008A850` | `gFileTableRegistered` | B |

**Evidence.** It is a `.sdata` word, zero in retail. This function's CD arm
sets it to 1 before installing the table, and that is the only access in
the image: `grep -rl D_8008A850 asm/` found only its definition and this
function. The name says what the one write does. Nothing reads the flag,
so what it was for is not known (tier B).
