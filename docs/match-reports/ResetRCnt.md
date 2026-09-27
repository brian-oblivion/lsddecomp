# ResetRCnt -- MATCHED (byte-exact, 14/14 words). Round 32, head.

> Renamed from `func_80032C60` on 2026-09-23 (tools/rename.py). Address 0x80032c60.

> **ROUND 32 (2026-09-12), head. CLOSED. The residue was real and correctly
> characterised for three rounds; what was missing was not a better SHAPE but
> the right TYPE.**
>
> Everything below the "Superseded derivation" line is the round-16/19 record
> and is kept verbatim: its measurement of the residue (one store hoisted into
> the `j`'s delay slot, every register already matching) was exactly right, and
> its conclusion that **barrier placement is exhausted** was also right. Three
> `__asm__("")` positions were tried across two rounds and all three regressed.
> The mistake was treating the barrier as the only instrument for the
> "instruction order only" class.

## What it does

`ResetRCnt`: clears the `count` field (offset `0x0`) of root-counter block
`D_8006DCB0[idx]` to `0` and returns `1`; returns `0` without touching the
table if `idx >= 3`, where `idx = (u16)n`.

## The match

```c
s32 ResetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = D_8006DCB0;
    base[idx].count = 0;
    return 1;
}
```

**That is character-for-character round 16's "best C reached".** The body was
never wrong. What changed is the table's declaration:

```c
typedef struct {
    volatile u16 count;     /* 0x0 */
    u8  pad2[0x4 - 0x2];
    volatile u16 mode;      /* 0x4 */
    u8  pad6[0x8 - 0x6];
    volatile u16 target;    /* 0x8 */
    u8  padA[0x10 - 0xA];
} RCntEntry;
```

## The mechanism, and why `volatile` succeeds where the barrier failed

The residue was GCC filling the unconditional `j`'s delay slot with the
`sh zero,0(v1)` store, where retail leaves a genuine `nop` and keeps the store
before the jump. A `volatile` store is an observable side effect, so it may not
be moved across the jump — the slot stays empty and retail's form falls out.

A bare `__asm__("")` is a blunt SCHEDULING FENCE: it constrains everything
crossing that one point, including the early `li $v0,1` and the CSE of the
return constants, which is precisely why all three placements regressed while
fixing the store. `volatile` constrains only the ACCESS, and only in the way
the hardware actually requires. It is both the narrower instrument and the
semantically honest one: this table shadows the PSX root-counter registers at
`0x1F801100`/`0x1F801110`/`0x1F801120`, so these fields genuinely are MMIO and
`volatile` is what they should always have carried.

Measured, all against a green whole-image SHA1:

| form | result |
| --- | --- |
| plain `u16`, three barrier placements (rounds 16, 19) | all three REGRESS past the 1-instruction residue |
| plain `u16`, single-exit join variable (round 32, new) | WORSE: 1 word short, adds a trailing `move v0,a1` |
| `*(volatile u16 *)&base[idx].count = 0;` at the use site | **MATCH 14/14** |
| `volatile` on the struct field | **MATCH 14/14** |
| `volatile` on all three hardware fields | **MATCH 14/14** — kept |

**`volatile` also SUBSUMES the barrier `SetRCnt` was carrying.** That function
had a load-bearing `__asm__("")` after its `target` store, documented in round
16 as fixing an equivalent hoist. With the volatile typing in place the barrier
was removed and `SetRCnt` still verifies at 40/40 on a green whole-image SHA1.
It is gone from the source, because leaving both would tell the next reader the
barrier is doing work that the type is doing.

**And the same typing closed the sibling `GetRCnt` the same day**, which
had been filed as "register identity, not fixable by reshaping" — a terminal
verdict under HARD RULE 6. See `GetRCnt.md`; that is the more expensive
half of this finding.

### Proposed learning

**`volatile` is a legitimate and much narrower instrument for the
"instruction ORDER only" residue class, and it is not the banned
construct.** HARD RULE 6's test is whether removing it changes WHICH REGISTER
holds a value; a `volatile` qualifier on a memory access changes what may move
across what, exactly like the sanctioned bare `__asm__("")`, and nothing about
it names a register. Two practical consequences:

1. **"Barrier placement exhausted" is not "instruction-order residue
   exhausted."** Three rounds stopped at the first conclusion having written
   the second. When the displaced instruction is a MEMORY ACCESS, reach for
   `volatile` before declaring the class closed — the barrier fences the whole
   program point, `volatile` fences one access, and the blunter tool regressing
   says nothing about the sharper one.
2. **Prefer it where it is TRUE, not as a trick.** The lever applies cleanly to
   hardware register shadows, device tables and anything else the hardware
   observes. Use it where the semantics justify it and the honest typing is
   also the matching one; do not scatter `volatile` over ordinary globals to
   nudge a schedule.

---

## Superseded derivation (rounds 16 and 19) — kept verbatim

The residue measurement and the barrier census below are accurate and are why
this function was closable at all. Only the verdict is superseded.

Unit `libsnd_ssinit`, carved round 16 (2026-09-04). **Attempted, restored
to `INCLUDE_ASM`.**

## What it does

Setter/clear for the `count` field (offset `0x0`) of the root-counter
shadow table entry `D_8006DCB0[idx]` (see `SetRCnt.md` for the table
layout), where `idx = (u16)n`. Clears the field to `0` and returns `1` if
`idx < 3`, else returns `0` without touching the table.

## Best C reached

```c
#if 0
s32 ResetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;

    if (idx >= 3) {
        return 0;
    }
    base = D_8006DCB0;
    base[idx].count = 0;
    return 1;
}
#endif
```

This gets register allocation, comparison signedness (`slti`, matching
`GetRCnt`'s and `SetRCnt`'s pattern) and every operand exactly
right. The `base = D_8006DCB0;` split (rather than inlining
`D_8006DCB0[idx]` directly) was load-bearing: without it, GCC evaluated
the shift (`idx*16`) before the pointer load instead of after, which
ALSO scrambled register allocation the same way `GetRCnt` is stuck
(see that report) -- so this split may be the fix `GetRCnt` itself
is missing, except `GetRCnt` has no assignment to split the same
way (it only reads). Worth trying on `GetRCnt` as e.g. `base =
D_8006DCB0; return base[idx].count;` -- **tried, no change there**, so
the technique doesn't transfer; noted for whoever picks this back up.

## Residue: one instruction misplaced across a jump, register-identity NOT involved

```
retail                              built (best reached)
...
23478:  sll   v1,v1,0x4             23478:  sll   v1,v1,0x4
2347c:  addu  v1,v1,a0              2347c:  addu  v1,v1,a0
23480:  sh    zero,0(v1)            23484:  j     32c8c
23484:  j     32c90                 23488:  sh    zero,0(v1)      <- in the jump's delay slot
23488:  nop
```

Every register matches. The only disagreement: the built version's
delay-slot filler moves `sh zero,0(v1)` (an ordinary, branch-independent
store) into the `j`'s delay slot, where retail leaves that slot a genuine
`nop` and keeps the store BEFORE the jump. This is CLAUDE.md's "a nop
retail has and you do not" residue class, explicitly allowed to be
targeted with a bare `__asm__("")` scheduling barrier since it changes
order only, never register identity.

**Barrier placement was tried and made this specific case WORSE, not
better** -- the opposite of what happened in `SetRCnt` for a
structurally similar hoist:

- Barrier immediately after the store, before `return 1;`: reintroduced
  the register swap AND moved the early `li $v0,1` to the very end of the
  function (past the store), a strictly worse diff.
- Barrier immediately before the store (after `base = D_8006DCB0;`):
  produced a redundant extra instruction (`move v0,zero` AND `li v0,1`
  both present -- CSE broke), 4 bytes longer than retail.

Contrast with `SetRCnt`, where a barrier in the analogous spot (right
after the hoisted store) fixed an equivalent hoist with NO side effects
elsewhere in that function. The two are not identical situations -- in
`SetRCnt` the early `li` sits before a CONDITIONAL branch with real work
on both sides; here it sits before an UNCONDITIONAL jump straight to a
shared epilogue with very little else in the function. Barrier-based
delay-slot fixes are evidently sensitive to how much scheduling latitude
exists around them, not just to which instruction is displaced -- treat
each occurrence as its own experiment, not a transferable recipe, even
within the same source file.

**Attempt 6 (round 2, head-directed): flip the guard from
negative-first to positive-first**, on the reasoning that the `j 32c90`
this residue hoists into only exists because the work block is
out-of-line; making the work block the fallthrough (positive-first)
should remove the jump entirely and with it the delay slot to hoist
into:

```c
#if 0
s32 ResetRCnt(s32 n)
{
    s32 idx = (u16)n;

    if (idx < 3) {
        D_8006DCB0[idx].count = 0;
        return 1;
    }
    return 0;
}
#endif
```

Checked negative, and worse than the 1-instruction residue, not better:

```
retail                                    positive-first guard (attempt 6)
andi v1,a0,0xffff                         andi v1,a0,0xffff
slti v0,v1,3                              slti v0,v1,3
beqz v0,2348c  (-> shared "return 0")     bnez v0,23478  (-> jumps to valid-path body)
li   v0,0x1                               li   v0,0x1
lui a0,... / lw a0,...                    j    32c8c          <- NEW inline early-return jump
sll v1,v1,0x4                             move v0,zero        <- NEW inline "return 0"
addu v1,v1,a0                             lui a0,... / lw a0,...   (now reached via ~>)
sh  zero,0(v1)                            sll v1,v1,0x4
j   32c90                                 addu v1,v1,a0
nop                                       sh  zero,0(v1)
                                          (falls straight into jr ra -- no `j 32c90` at all,
                                           but at the cost of the whole new early-return block)
```

The predicted mechanism was half right: the trailing `j 32c90` +
delay-slot hoist DOES disappear, because the valid-path body now falls
through directly into the shared epilogue instead of jumping to it. But
this doesn't reproduce retail -- it just relocates the mismatch. Retail's
"return 0" case lives ONCE, at the very end, reached by a forward `beqz`
with no code duplicated inline; the positive-first shape instead
duplicates an early-return stub (`j`/`move v0,zero`) right after the
guard test and reaches the valid body via a backward-labelled `~>`
jump target. Net effect: a WORSE diff than the delay-slot-only residue
(new content mismatch spanning most of the function, not just the one
hoisted store), even though the SPECIFIC "j 32c90 delay slot" symptom is
gone. Confirms the same boundary found on `GetRCnt`: guard clauses
of the "range check, then bail" shape want negative-first in this
project's compiled output, and `_SsSeqCalledTbyT_1per2`'s raw-flag lever does not
transfer to them. Reverted; no change kept in `src/`.

### Proposed learning

1. A bare `__asm__("")` fixing a delay-slot hoist in one function is not
   evidence the same placement (relative to the same kind of store/jump
   pair) will work in a sibling function with a different amount of
   surrounding code. Confirm per-function; do not batch-apply.
2. **Boundary on the `_SsSeqCalledTbyT_1per2` block-order finding** (same
   conclusion independently reached via `GetRCnt`): it applies to a
   raw truthy/flag value feeding `beqz`/`bnez` directly, not to a "guard
   clause + range comparison" (`slti`/`sltiu` against a small constant).
   For THAT shape, retail's own compiled form is negative-first
   (`if (x >= N) return fail;`), and flipping it does not just fail to
   help, it introduces an inline early-return block retail doesn't have,
   which is a worse diff than the original one-instruction residue.

## Round 19 re-check (runner charlie)

Re-verified the 1-instruction residue is still exactly as documented
(confirmed with `asm-differ`: only the `sh zero,0(v1)` hoisted into the
`j`'s delay slot where retail leaves a genuine `nop`, everything else
byte-identical including operands and the register allocation the
sibling `GetRCnt` cannot reach). The whole-file 268446-byte "drift"
funcdiff reports is not a regression signal here -- it's the same
1-instruction/4-byte size difference propagating through every
downstream address in the image, which is what this exact residue has
always looked like at the whole-image level; `asm-differ`'s per-function
alignment view is the one to trust for this function, not funcdiff's
raw-offset dump.

Tried a third barrier placement not in the round-16 list: `__asm__("")`
as the very first statement of the valid branch, before `base =
D_8006DCB0;` (rather than after it, or after the store). **Worse, not
neutral**: the compiler now drops the `j` entirely and duplicates
`move v0,zero` (both the early-return path's implicit zero AND a second
redundant zero-move appear), a strictly worse diff than the 1-instruction
residue. This is the third of three barrier positions tried across two
rounds (after-store, before-store, before-block) and all three regress a
residue that the un-barriered form leaves at exactly one instruction.
**Barrier placement is now exhausted for this specific residue** -- do
not re-try it without a materially different idea.

No new axis from the round-19 brief (retype, whole-struct, split-
statement, wrong-void-return) applies here beyond what round 16 already
covered: the function already uses the pointer-split form (axis 2), the
return type is genuinely non-void (`s32`, consumed as a boolean success
flag by callers of the analogous `SetRCnt`), and there's only one
expression per statement already (axis 3 doesn't apply -- the residue is
a scheduling choice, not a combined expression). Still restored to
`INCLUDE_ASM`.

## Provenance

round 16 (2026-09-04), runner delta, unit libsnd_ssinit (fresh carve). ~6
attempts (statement-order fix reached the 1-instruction residue; 2
barrier placements and 1 head-directed control-flow flip all regressed
and were reverted); restored to `INCLUDE_ASM`.

round 19 (2026-09-05), runner charlie. Re-confirmed the 1-instruction
residue with asm-differ; tried a third barrier placement (before the
block), regressed like the other two; restored to `INCLUDE_ASM`.

## ROUND 20 (runner echo): re-tested with the extra-temp lever that helped `GetRCnt` -- regressed, confirmed negative

Following this round's `GetRCnt` finding (splitting a combined
`base = D_8006DCB0; return D_8006DCB0[idx].count;` into a THIRD,
independently-live `entry` pointer local closed most of that function's
register-identity residue), tested the same lever here:

```c
s32 ResetRCnt(s32 n)
{
    s32 idx = (u16)n;
    RCntEntry *base;
    RCntEntry *entry;

    if (idx >= 3) {
        return 0;
    }
    base = D_8006DCB0;
    entry = &base[idx];
    entry->count = 0;
    return 1;
}
```

**Regressed badly: `funcdiff.py` reports 2/14 words with a genuine 268446-
byte outside-range drift** (a real size/shape change, not the expected
"same length, different order" residue this function otherwise has).
This function already has one more live value than `GetRCnt`
(the store's own operands, plus the already-load-bearing `base` split
from this function's existing best body) -- adding a THIRD live pointer
pushed register pressure past what this function's existing register
file can absorb without changing shape, unlike `GetRCnt` where the
same lever had slack to use. Reverted immediately; confirms this round's
`GetRCnt.md` note that the lever is not universal and must be
tested per-function. This function's own best body remains the
already-documented `base = D_8006DCB0; base[idx].count = 0;` (13/14
content-correct per `asm-differ`'s aligned view, one missing `nop`/
hoisted-store-into-delay-slot residue) -- re-confirmed this round via
direct rebuild (see below), unchanged from round 19.

**Drift-check (round 20):** rebuilt this exact best body and re-verified
with `asm-differ` directly rather than trusting the report's prior
number: aligned view shows ONLY the documented single-instruction
residue (`sh zero,0(v1)` scheduled into the `j`'s delay slot here,
where retail leaves that slot a genuine `nop` and keeps the store
before the jump) -- everything else byte-identical, confirming this
report's long-standing characterisation is still accurate. Restored to
`INCLUDE_ASM` (still not byte-exact).

## Naming

Round 69 (delta). `ResetRCnt` (was `func_80032C60`): clears
`D_8006DCB0[idx].count` to 0, guarded by the same range check as `GetRCnt`
-- tier A (a clamp/reset leaf whose mechanics are its purpose). Name taken
directly from this report's own heading.

## Identification (round 69, head)

Sony's `ResetRCnt`, `libapi/counter`: the five functions at 0x80032B18 are that module's exports in its own order (SetRCnt, GetRCnt, StartRCnt, StopRCnt, ResetRCnt), with the first three offsets exact against the 3.5/3.6 `counter.o` and the last two 4 and 8 bytes later (a library build the discs do not carry, so no object can be linked); `KERNEL.H` prototypes agree on arity. Two evidence kinds per FINISHING-PLAN track 2. 
