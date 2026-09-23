# GetRCnt -- MATCHED (byte-exact, 14/14 words). Round 32, head.

> Renamed from `func_80032BB8` on 2026-09-23 (tools/rename.py). Address 0x80032bb8.

> **ROUND 32 (2026-09-12), head. CLOSED — and the prior verdict's CAUSE was
> wrong in the most expensive direction available.**
>
> Three rounds classified this as **"register identity, not fixable by
> reshaping"**, which in this project is a terminal verdict: HARD RULE 6
> bans the only constructs that fix register identity, so a function filed
> that way is one nobody re-measures. It was never a register-identity
> residue. It is ordinary matchable C, and the fix is a TYPE, not a shape.

## What it does

`GetRCnt`: reads the `count` field (offset `0x0`) of root-counter block
`gRCntRegs[idx]`, where `idx = (u16)n`. Returns `0` without touching the
table if `idx >= 3`.

## The match

```c
s32 GetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = gRCntRegs;
    return base[idx].count;
}
```

That is byte-for-byte the shape earlier rounds had already reached and
rejected. **The only thing that changed is that `RCntEntry`'s three
hardware fields are now declared `volatile`** (see `ResetRCnt.md` for
how that was found). With the plain `u16` typing this body produces the
documented register scramble; with the volatile typing it is exact, first
attempt, no barrier, no reshaping.

## Why the old verdict read as register identity

The residue genuinely LOOKED like one: the built code used different
registers from retail for the pointer and the index. But that was a
CONSEQUENCE, not the cause. Without `volatile`, GCC is free to reorder the
table load against the index arithmetic, and the order it chooses
determines which values are live across the branch — so the allocation
falls out differently. Constrain the memory access and the ordering is
forced; the allocation then follows retail's on its own.

**This is the wrong-CAUSE hazard from CLAUDE.md, in its expensive
direction.** A wrong SCORE gets corrected the next time anyone measures. A
wrong CAUSE is what the next round acts on — and when the wrong cause is
"register identity", the project's own rules say stop. The function
attracted three rounds of attempts and then correctly stopped attracting
them, which is exactly how matchable ground gets deleted permanently.

### Proposed learning

**A register-identity verdict is a hypothesis about a MECHANISM, and the
observation that registers differ does not establish it.** Registers
differing is compatible with at least two causes: GCC genuinely wanting a
different allocation (terminal under HARD RULE 6), or GCC being free to
schedule in an order retail was not (ordinary, and fixable from the
source). The discriminator is whether anything in the C constrains the
ORDER. Before filing register-identity, ask what forces the order in
retail's build that does not force it in yours — and if the answer is "an
access the hardware makes observable", the fix is `volatile` and the
verdict is wrong. See `ResetRCnt.md` for the full mechanism and for
the barrier-vs-volatile comparison.

## Provenance

Round 16 (2026-09-04) runner delta, round 19 (2026-09-05) runner charlie,
plus head-directed attempts: filed as register identity, not fixable.
Round 32 (2026-09-12), head: matched 14/14 first attempt once the
`volatile` typing was in place. Whole-image SHA1 green.

## Naming

Round 69 (delta). `GetRCnt` (was `func_80032BB8`): a pure getter (reads
`gRCntRegs[idx].count`, guarded by a range check) -- tier A by the
project's own rule ("a getter ... is tier A by definition"). Name taken
directly from this report's own heading, which already called it `GetRCnt`
before the symbol itself was renamed.
