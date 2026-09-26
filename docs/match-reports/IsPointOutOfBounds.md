# IsPointOutOfBounds — MATCHED 27/27 (round 77, charlie; REVISIT-2)

> Renamed from `func_8004CD38` on 2026-09-24 (tools/rename.py). Address 0x8004cd38.

> **REVISITED, round 77: MATCHED 27/27, whole-image SHA1 green; the in-range test written as its negation returning 0, with `return 1` after it, so jump.c presets the constant into `$v0`; names/types not relevant (signature, struct and caller unchanged).**

> **REVISITED, round 71: STALL, unchanged at 2/27; the recorded cause is refined to ONE root (block-local temps avoid `$v0` in retail), and a result-variable shape reaches 1/1 ins/del against the given body's 6/6; names/types not relevant (no type lever reached it; a narrower return type changes the caller).**

## Round 77 (charlie) — REVISIT-2, MATCHED

**Rebuilt as given first.** The `#ifdef NON_MATCHING` body made live:
**2/27, exact length, zero drift, funcdiff `insertions 6 / deletions 6`,
positional skeleton diffs 25** — the same as round 71.

**The body that matches** (live in `src/class_3bb8c_b.c`, `INCLUDE_ASM` and
the `NON_MATCHING` block removed):

```c
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minX && bounds->maxX >= point[0]
        && point[1] >= bounds->minY && bounds->maxY >= point[1]) {
        return 0;
    }
    return 1;
}
```

`./build-and-verify.sh`: exit 0, `OK: build matches retail`; funcdiff 27/27,
`insertions 0 / deletions 0`; `tools/check-nonmatching.sh` green. The caller
`Class866E8__SetFootprintFromQuery` is untouched (same signature).

**Why it works (the mechanism round 71 named, and the construct that
reaches it).** Round 71 established that the one root is that retail's
block-local temps avoid `$v0`, which needs `$v0` live across the check blocks
before local-alloc. The construct that makes the HARD return register live
there is jump.c's "`if (...) x = a; else x = b;` -> `x = b; if (...) x = a;`"
preset, applied with `x` = `$v0`:

- the tests may be a multi-branch chain, provided every one jumps to the
  else label (an `&&` chain does; an `||` chain jumps to the THEN label and
  leaves a label between the tests and `x = a`, which blocks it);
- `x = a` must be ONE insn directly before the `goto` over the else arm, so
  `a` must be a constant or register — `return bounds->maxY < point[1]` is
  several insns and never qualifies, which is why every earlier shape that
  RETURNED the comparison missed;
- `x = b` (`return 1`) is a constant.

So the last comparison moves INTO the chain (`&& maxY >= point[1]`) and the
then-arm becomes `return 0`. The preset puts `v0 = 1` ahead of the NULL test
(it lands in the `beqz` delay slot), local-alloc's temps then take
`$v1`/`$a0`, `bounds` is pushed off `$a0` into `$a2` (retail's entry
`move a2,a0`), and the remaining `if (!(maxY >= p1)) v0 = 0` is folded back
into the store-flag `slt v0,v1,v0` that falls into `jr ra`.

**Measured shapes this session** (isolated compiles through the pinned
pipeline, then the full oracle for the winner):

| shape | result |
| --- | --- |
| as given (four `return 1;`, `return maxY < p1`) | `$v0` temps, three `li v0,1` |
| `if (in range && ...) return maxY < p1; return 1;` (and with explicit `else`) | same as given |
| `if (A \|\| ... \|\| maxY < p1) return 1; return 0;` | no preset; `move v0,zero` tail, 29 words |
| five separate `if (...) return 1;` then `return 0;` | store-flag fold happens (26 words) but no preset, `$v0` temps |
| **`if (!A && ... && maxY >= p1) return 0; return 1;`** | **27/27, byte-exact** |

The prompt's parameter-list hypothesis (a structural cause for the entry
`move`) was checked and is not the cause: the only caller passes
`(self->bounds, buf.point)` and the signature is unchanged in the match.
No permuter search was run (not needed); Gate 3 therefore not run.

### Proposed learning

**When retail presets a return constant in `$v0` before the first test and
every block temp avoids `$v0` (often visible as an entry `move aN,a0` in a
leaf), write the test as the NEGATED `&&` chain with `return 0` in the
then-arm and `return <const>` after it.** jump.c's `x = b; if (...) x = a;`
preset fires on the hard return register only when `x = a` is a single insn
(a constant) and every test jumps to the else label; a trailing computed
comparison goes into the chain, and store-flag folds it back into `slt`.
Returning the comparison, `||` chains and `result` variables are all
negative (`IsPointOutOfBounds`, 2/27 -> 27/27, round 77).

---

## Earlier history (the title below was the stall title before round 77)

Former title: IsPointOutOfBounds — STALL, 27 words (exact length, zero drift), 2/27 raw word-match (round 71 re-measure), first diff at vram 0x8004CD38 (retail's `move a2,a0`, which every C shape omits)




## Round 71 (delta) — revisit

**Rebuilt as given first.** The `#ifdef NON_MATCHING` body made live: **2/27,
exact length, zero drift, funcdiff `insertions 6 / deletions 6`, positional
skeleton diffs 25.** `plan.py`'s `len-off` tag is stale: length is exact.

**The cause, narrowed to one decision.** Read register by register, every
difference follows from a single fact: in retail the block-local temporaries
never use `$v0` (left comparison operand -> `$v1`, right -> `$a0`), so the
constant 1 put in `$v0` by the first delay slot survives to every exit and
reorg drops the three later `li v0,1`s; and because a block-local took `$a0`,
the global `bounds` pseudo is pushed to `$a2`, which is the `move a2,a0` at
the top. In every C shape tried, local-alloc hands the first temp `$v0`,
`bounds` keeps `$a0`, and the constant is re-materialised in each `bnez`
delay slot. For a block-local to avoid `$v0`, the hard register `$v0` has to
be live across the check blocks in retail's RTL (set before the NULL test,
read at the exit). No C spelling that produces that was found. Since
local-alloc runs before global-alloc, a `result` variable cannot win `$v0`
from the temps; it lands in `$a3` (shape below).

| shape (full oracle each) | score | ins/del |
| --- | --- | --- |
| as given (four `return 1;` + `return b8 < p1`) | 2/27 | 6/6 |
| `(s8)((u8 *)point)[i]` casts | 2/27 (identical) | 6/6 |
| `return a \|\| b \|\| ...` single expression | 0/27 | 4/4 |
| `result = 1; if (all in range) result = b8 < p1; return result;` | 0/27 | **1/1** (result in `$a3` + a final `move v0,a3`; otherwise retail's instruction stream) |
| the same, `goto out` form / nested-if form / else-if chain assigning `result` / `\|\|` then `result = 1` else | 0/27 | 1/1 each |
| two named scratch locals reused per comparison | 2/27 | 3/3 |
| per-point named locals `x`, `y` | 0/27 | 12/12 |
| `?:` return, either polarity; single `if (\|\|) return 1;` | 2/27 or 0/27 | 6/6 or 1/1 |
| `result = point[1]; result = b8 < result;` | 0/27 | 3/3 |
| comparisons flipped (`p0 <= b4`, `p1 > b8`) | 0/27 | 3/3 |
| return type `u8`/`s8`/`s16`/`u16` | 5/27, 0/0 in range, but **97 bytes differ in the caller** (`Class866E8__SetFootprintFromQuery`): rejected |

**Bounded search (one).** Gate 3, all three checks, on the 1/1 result-variable
shape: (1) scaffold compiles, base 360; (2) `--debug --stack-diffs`: 1
insertion / 1 deletion; (3) funcdiff in-tree: `insertions 1 / deletions 1`:
AGREE. `permuter.py -j 6 --stop-on-zero --best-only --stack-diffs`, `timeout
1800`, **exit 124 (timeout) after 376,191 iterations, no zero.** Best
candidates 220 and 240 both add a dereference of `bounds` BEFORE the NULL test
(`if (point || bounds->unk0)`), which changes behaviour; rejected unmeasured
as UB-class. Together with round 18's 71,363 on the other shape, this
function has now had ~447k search iterations across two structurally
different bases.

Residue class: register identity (HARD RULE 6 territory). Discriminator: with
the 1/1 shape, the only non-register difference left is the copy of the
result into `$v0`, and that copy exists because the result lives in a pseudo
that local-alloc's temps have already shut out of `$v0`.

### Closest-structure body (1/1, 0/27 words; the 2/27 body in `src/` stays the NON_MATCHING body because the metric is words)

```c
#if 0
/* needs: common.h, class_3bb8c.h (Bounds866E8_3bb8c_b) */
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    s32 result;

    result = 1;
    if (bounds != NULL && point[0] >= bounds->unk0 && bounds->unk4 >= point[0]
        && point[1] >= bounds->unk2) {
        result = bounds->unk8 < point[1];
    }
    return result;
}
#endif
```

### Proposed learning

**`IsPointOutOfBounds`: when retail keeps a constant return value in `$v0` across
several exits and your build re-materialises it in each delay slot, the
decision was register allocation, not reorg**: retail's block-local temps
avoided `$v0`. Check which register the FIRST temp of the first block gets
before trying return-statement shapes; a `result` variable does not help,
because local-alloc runs first.

**Scoring trap met this round (not new, way 2):** splicing a candidate into
a `#ifdef NON_MATCHING` block that has been restored by `git checkout` gives
`build exit=0` and 27/27, since `INCLUDE_ASM` is compiled. Re-toggle the
block live after any checkout.

---


> **ROUND 46 (charlie): re-verified 2/27 with no drift; a fresh permuter
> scaffold built specifically to run the longer/differently-seeded search
> this report's own history calls for -- and it fails this round's Gate 3
> check even harder than round 18's did, so no search was launched.**
> Rebuilt the round-19 `goto`-form best body (Shape A, reproduced below)
> from a clean `INCLUDE_ASM` baseline first per Gate 1b: temporarily
> enabled it in `src/class_3bb8c_b.c` and confirmed **2/27, exact length,
> no outside-range drift** -- identical to round 19's own recorded figure.
> Reverted immediately (`git checkout -- src/class_3bb8c_b.c`; clean
> `OK: build matches retail` confirmed afterward).
>
> Built a brand-new scaffold from scratch (`tools/setup-permuter.sh
> IsPointOutOfBounds <seed>`, seed = the same round-19 `goto` body just
> verified) specifically to check whether a longer or differently-seeded
> search -- the "remaining lever" this report's round-18 entry names --
> would be worth running. `--debug --stack-diffs`: **base score 2900 -- 14
> insertions, 14 deletions, 0 reorderings, 20 register differences.** The
> REAL in-context build (just confirmed above, same session) shows **ZERO**
> insertions or deletions -- 2/27 is a pure register-identity cascade at
> the correct length, nothing missing or extra anywhere. An isolated
> scaffold reporting 14/14 against a target the real build reproduces with
> 0/0 is this round's Gate 3 failure mode, and it is a LARGER mismatch than
> round 18's own scaffold (which showed 4 insertions/5 deletions against
> the same real-build 0/0 -- also technically a Gate-3 failure, just a
> smaller one, from before Gate 3 was a named check). **Per Gate 3, a
> disagreement means STOP: no search launched.**
>
> This also explains, after the fact, why round 18's 71363-iteration
> search never reached zero despite 600 real seconds of compute: it was
> never scoring this function's actual residue (a whole-parameter register
> identity choice, `bounds` in `$a2` vs `$a0`) at all, only the isolated
> scaffold's own different structural gap. **Disposition unchanged: 2/27,
> `INCLUDE_ASM` (no preserved body currently in `src/`, per existing
> convention -- see "Preserved best-attempt body" below), not
> re-attempted further by hand this round.** The permuter route for this
> function needs a scaffold with more real surrounding-file context before
> it can be trusted, which is head-scale work, not a bounded runner
> attempt -- consistent with three other functions in this unit
> (`Class866E8__SplitFootprintSlot`, `Class866E8__ComputeFootprintDescriptor`, `Class866E8__ApplyRateEntries`, `Class866E8__BuildFootprintSlots`)
> reaching the identical verdict this round and last.
>
> ### Proposed learning
>
> **A permuter search's own non-zero result ("best 770, down from base
> 1040") does not certify that the scaffold was scoring the right thing --
> only a Gate-3 comparison against the real in-tree build does, and it can
> fail by a WORSE margin on a re-built scaffold than the original.** Round
> 18's search looked like ordinary "search ran, didn't reach zero" until
> this round's Gate 3 discipline was applied retroactively; the 600
> real-second, 71363-iteration budget it spent was measuring a program
> nobody is building. For a function this small (27 words), the
> single-function isolation itself may be the dominant source of
> mismatch -- worth someone checking whether a scaffold built with more of
> this file's other functions included changes the picture, rather than
> retrying with fresh RNG on the same isolated seed.

Not a vtable slot (confirmed absent from `gClass866E8Methods` via
`tools/classtable.py`) — a plain, non-virtual bounding-box test. Only
caller: `Class866E8__SetFootprintFromQuery` (`IsPointOutOfBounds(self->unk1DC, &stackBuf[0x12])`),
which established `Obj866E8::unk1DC`'s type (see `Class866E8__SetBounds`'s
report).

## What the function does (not in doubt)

Four early exits, each returning the literal `1`, sharing a null check and
two axis range tests against an `[x, y]` signed-byte point; falls through
to a genuine computed comparison for the final axis:

```
addu $a2, $a0, zero            ; a2 = self/bounds (retail copies a0 into a2 up front)
beqz $a2, FAIL                  ; bounds == NULL -> 1        (v0=1 set in the delay slot, shared by ALL exits below)
 ori  $v0, zero, 1
lbu  $v1, 0(a1)                  ; point[0]
lh   $a0, 0(a2)                   ; bounds->unk0
sll/sra ...                        ; sign-extend point[0] (s8)
slt  $v1, $v1, $a0
bnez $v1, FAIL                       ; point[0] < bounds->unk0 -> 1
 sra $a0, $a3, 24                     ; delay slot: RE-EXTRACT point[0] from the shifted value
                                       ; saved earlier (a3), reusing it for the NEXT check
lw   $v1, 4(a2)                        ; bounds->unk4
slt  $v1, $v1, $a0
bnez $v1, FAIL                          ; bounds->unk4 < point[0] -> 1
 nop
lbu  $v1, 1(a1)                          ; point[1]
lh   $a0, 2(a2)                           ; bounds->unk2
sll/sra ...                                ; sign-extend point[1]
slt  $v1, $v1, $a0
bnez $v1, FAIL                              ; point[1] < bounds->unk2 -> 1
 nop
lw   $v1, 8(a2)                              ; bounds->unk8
sra  $v0, $a1, 24                             ; re-extract point[1]
slt  $v0, $v1, $v0                             ; ACTUAL final comparison, falls straight into jr ra
FAIL:
jr ra
```

This maps directly onto:

```c
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    if (bounds == NULL) return 1;
    if (point[0] < bounds->unk0) return 1;
    if (bounds->unk4 < point[0]) return 1;
    if (point[1] < bounds->unk2) return 1;
    return bounds->unk8 < point[1];
}
```

and the CFG/branch-target structure is not in question — every attempt
below reproduced the SAME four-exit-plus-fallthrough shape retail has. The
residue is purely in TWO adjacent instructions:

1. Retail copies `$a0` into `$a2` unconditionally, in its very first
   instruction, before the null check. Every attempt below has this
   register copy folded away entirely (no instruction emitted for it) —
   GCC just keeps using `$a0` throughout instead of moving to `$a2`.
2. Retail's delay slot after the FIRST early-exit branch
   (`bnez $v1, FAIL`) is `sra $a0, $a3, 24` — re-deriving the
   sign-extended `point[0]` value from the shifted-but-not-yet-extracted
   form (`$a3`) it saved earlier, for reuse in the SECOND comparison.
   Every attempt below either drops this instruction outright (constant-
   folding the branch target to reuse the value already resident from
   check 1) or, in one variant, keeps an equivalent-looking recompute but
   one word later, netting the same total size with a DIFFERENT internal
   layout (see "Two structurally distinct outputs" below).

## Attempts (6, well under the 30 cap)

All six are semantically identical restatements of the same four-early-
exit-then-compute shape; every one compiled to one of exactly TWO possible
outputs (never mid-way), regardless of surface syntax:

1. **Four separate `if (cond) return 1;` statements.** GCC did NOT unify
   the four literal-`1` return blocks into one shared exit target: only
   the FIRST reaches the shared `jr ra`; the LAST duplicates its own
   `li v0,1` immediately before a SEPARATE, adjacent `jr ra`. 0/27 (by
   raw word position; the actual CFG is right for 3 of 4 exits, wrong for
   the last).
2. **Single chained `||` expression**
   (`if (bounds==NULL || point[0]<bounds->unk0 || ... ) return 1;`).
   Unified 3 of 4 exits onto the shared target (matching retail's own
   sharing for those three); the FOURTH check's exit still uses its own
   adjacent `li v0,1` block, with the true final computation reached via
   an explicit `j` around it. Same net word count as retail (27), so no
   "outside range" drift — but 2/27 internal match, one instruction short
   in the wrong place and one instruction long in the wrong place (see
   "Two structurally distinct outputs" below for the exact shape).
3. **Explicit `goto fail;` / `fail: return 1;`.** Byte-identical output to
   attempt 1 (separate ifs) — `goto` to a shared label is apparently
   lowered identically to repeated `return 1;` statements by this
   compiler for this shape, at least here.
4. **`goto fail;` with `bounds` copied into an explicit local first**
   (`Bounds866E8_3bb8c_b *bounds = arg0;`), attempting to force retail's
   `move a2,a0`. Byte-identical to attempt 3 — the redundant local was
   optimized away with no effect on codegen.
5. **`goto fail;` with the byte values cached into explicit `s8 x, y;`
   locals** (`x = point[0]; if (x < bounds->unk0) ...; if (bounds->unk4 <
   x) ...;`), attempting to force retail's re-extraction-from-a-register
   pattern. Byte-identical to attempts 3/4 — the locals were likewise
   optimized away.

## Two structurally distinct outputs (this is the actual finding)

Every attempt above collapsed to one of exactly two shapes:

- **Shape A** (attempts 1, 3, 4, 5): matches retail's FIRST TWO exits
  exactly (`beqz`->shared target, first `bnez`->shared target), but is
  missing retail's `move a2,a0` (word 0) AND its `sra a0,a3,24` re-extract
  (right after check 1), and its LAST check/exit uses its own separate
  `li v0,1` block instead of reaching the shared target. Net: one
  instruction fewer than retail in the front, one instruction more at the
  tail — same total length, wrong internal split.
- **Shape B** (attempt 2): all four exits correctly aggregate branch
  TARGETS the same way retail's do for three of them, but the shared exit
  is positioned ONE WORD EARLIER than retail's (missing the `move a2,a0`
  and the `sra` re-extract, same two missing instructions as Shape A),
  and the true final computation reaches it via an explicit `j` (one word
  retail does not have) landing next to a duplicated `li v0,1` (also one
  word retail does not have) for the last check alone.

Both shapes are missing the exact SAME two instructions (`move a2,a0` at
the top; `sra a0,a3,24` after check 1) and have the exact same net word
count as retail by adding back exactly two OTHER instructions elsewhere
(`j` + `li` in Shape B; a duplicated `li` in Shape A). No syntactic
variation tried moved either of the two missing instructions into
existence.

## Broadcast levers checked, neither changed the read

- **Delay-slot-belongs-to-target lever:** re-audited both delay slots
  above. The `beqz`'s delay slot (`v0=1`) and check-1's `bnez`'s delay
  slot (`sra a0,a3,24`) were already read as always-executing in every
  attempt (that is precisely why `v0=1` reaching the shared `jr ra`
  without being re-set was already the expected/attempted shape). Neither
  delay slot was misattributed to only one path.
- **Lowering-transcription lever:** the repeated `sll`/`sra`-by-24 pairs
  are the ordinary `s8` sign-extension idiom (already modeled correctly —
  my C uses `s8 *point`, and my own builds reproduce the identical
  `sll`/`sra` pairs). No `xori`+`sltiu`, `sltu`, or `sltiu`-vs-1 pattern
  is present anywhere in this function to mistranscribe.
- **Redundant-move lever (broadcasts #1/#2):** does not fit either
  direction. This isn't one extra/missing `move` of an ALREADY-COMPUTED
  value into `$v0` — it's a *register* retail uses (`$a2` instead of
  `$a0`) that no variant tried reproduces, plus a re-EXTRACTION (not a
  restated mention) of a value from an intermediate shifted form. Three
  different ways of writing the two comparisons (separate statements,
  `||`, cached locals) all produced identical machine code, which argues
  AGAINST this being reachable by counting source mentions of the value —
  the compiler is doing the same CSE regardless of how many times the
  byte is textually written.

## Preserved best-attempt body

```c
#if 0
/* Shape B (attempt 2) -- matches retail's branch TARGETS for 3 of 4 early
 * exits, closest structural match found, still not byte-exact. */
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    if (bounds == NULL || point[0] < bounds->unk0 || bounds->unk4 < point[0] ||
        point[1] < bounds->unk2) {
        return 1;
    }
    return bounds->unk8 < point[1];
}
#endif
```

## New struct knowledge (established despite the stall)

- New type `Bounds866E8_3bb8c_b` (`unk0`/`unk2` s16, `unk4`/`unk8` s32) —
  derived from reading THIS function's own body, independent of whether a
  byte-exact source form for it has been found. Used by `Obj866E8::unk1DC`
  (see `Class866E8__SetBounds`'s report).

## Attempts

6.

### Proposed learning

**A residue can be pinned down to exactly two missing/misplaced
instructions (a register copy and a value re-extraction) while remaining
completely unreachable across five independent, semantically-identical
source restatements — number of textual mentions of a value does not
predict whether GCC re-derives or reuses it.** This is a DIFFERENT
residue class from the project's existing "shared-literal early-exit
delay-slot placement" entry (`DreamSys__TryInstantTeleportLink`) in one respect worth
recording: there, the CFG already matched and only scheduling differed;
here, THREE of the four early exits correctly unify onto retail's shared
target under one source shape (attempt 2) while the fourth does not, and
no combination tried gets all four AND recovers the two missing
instructions simultaneously. Flagging as a permuter candidate per this
round's broadcast — a genuine 2-of-27-words-off near-miss with the CFG
settled — rather than spending further hand-reshaping attempts.

---

## Head verification, round 8 (2026-09-02)

**Classification CONFIRMED, and the disposition is right.** Residue 1 is a
register-identity difference — retail keeps `bounds` in `$a2` and ours keeps
it in `$a0` — which CLAUDE.md rule 6 makes a STALL by definition, not a
judgement call. It must not be closed with `register T v asm("$N")` or an
operand constraint, and this report correctly did not try.

**One thing to add, because the number is misleading.** 2/27 reads like
"nowhere near", and it is not. MIPS encodes register numbers inside the
instruction word, so a single differing register assignment in the FIRST
instruction cascades: every later instruction that touches `bounds` encodes a
different word even where the opcode, the operand order and the control flow
are all identical. The report's "correct size, correct CFG, two adjacent
instructions of real residue" and the score of 2/27 are therefore perfectly
consistent, and a future reader should not downgrade this from a near-miss on
the strength of the number.

**The check that would settle whether it is really two instructions**:
`.venv/bin/python3 tools/asm-differ/diff.py IsPointOutOfBounds` on the preserved
body reads the diff structurally rather than by word equality. Do that before
spending a permuter run, so the run is aimed at a residue whose size you know.

Head broadcasts #1–#3 were checked against this function by the runner and
none applied; I agree. In particular broadcast #3's delay-slot rule does NOT
crack residue 2: the `sra $a0, $a3, 24` in the branch's delay slot is a
re-extraction feeding the NEXT comparison, and the runner read its ownership
correctly.

Still a genuine permuter candidate — better posed than `Entity__UpdateActivationState`,
because the residue is local and the CFG is settled.

---

## Round 18 (echo) — permuter run, no zero, best 770 (down from base 1040)

`--debug` base score against the report's own preserved Shape-B body
(the `||`-chained form): **1040** (28 register diffs, 4 insertions, 5
deletions) -- NOT the clean pure-register-diff signature the smaller
`ItemList__LoadResources`/`TaskObjF__WriteMemcardSaveFile` scaffolds showed, consistent with the
head's own note that this function's "2/27" reads misleadingly and the
real residue cascades widely.

Bounded search (`timeout 600`, `-j 6 --stop-on-zero`): **71363
iterations**, best score **770** (down from 1040, never reached zero).
`timeout`'s own exit code line was not captured (same outer-Bash/inner-
`timeout` race as `Class866E8__ConfigureRateEntry`/`strcpy` this round -- the trailing
`echo "permuter exit=$?"` never lands in the log); treated as an ordinary
self-stop given the clean iteration count and no crash signature.

**Two intermediate leads inspected, both hand-tested against the real
oracle, both negative:**

1. `output-910-1`: introduces a `Bounds866E8_3bb8c_b *new_var = bounds;`
   used for ONLY the first (`->unk0`) and last (`->unk8`) struct accesses,
   leaving the null check and the two middle accesses (`->unk4`, `->unk2`)
   on the original `bounds`. Hand-translated to a CONSISTENT form (a
   single `b = bounds;` used for every access including the null check,
   matching retail's asm where literally every instruction -- including
   the null check itself -- reads through the copy register `$a2`) --
   **0/27 with size drift, dramatically worse than the 2/27 baseline**.
   Reverted (`build exit=0` confirmed clean afterward). This rules out
   "introduce one consistent alias for `bounds`, used everywhere" as the
   lever, which is somewhat surprising given retail's own asm reads as
   exactly that shape -- worth flagging as a case where transcribing
   retail's REGISTER usage literally into a "the whole function reads
   through one renamed pointer" C form does not reproduce it.
2. `output-770-1`: introduces an unused `int new_var = 0;` replacing the
   `NULL`/`0` literal in the guard, plus a dead `do { } while (new_var);`
   -- textbook permuter noise (unreachable dead code), not inspected
   further as a translation candidate.

**Disposition: still `INCLUDE_ASM`, unchanged.** Neither the permuter nor
the one hand-derived hypothesis from its leads closed this. Not
permuter-exhausted in the guide's technical sense (zero was never
reached); flagged as a genuine open lead for a future round given the
`new_var`-for-first-and-last-access asymmetry in `output-910-1` is still
unexplained (why THOSE two accesses specifically, and not the other two)
and was not fully run to ground.

---

## Round 19 (bravo) — verified the exhaustion claim, one more negative, class confirmed genuine

Per this round's brief ("report claims exhausted somewhere — verify that
claim, do not inherit it"): re-read. **The claim is accurately stated as
written** — round 18's own text already says "Not permuter-exhausted in the
guide's technical sense" and disposition is "flagged as a genuine open
lead", not "exhausted". No overclaim to correct here.

**Attempt 7**, testing this round's axis list against the residue: a
`goto`-based consistent alias (`bounds = arg0;` used for the null check AND
all four field accesses, matching retail's literal register usage where
even the null-check reads through the copied register) —

```c
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *arg0, s8 *point) {
    Bounds866E8_3bb8c_b *bounds;
    bounds = arg0;
    if (bounds == NULL) goto fail;
    if (point[0] < bounds->unk0) goto fail;
    if (bounds->unk4 < point[0]) goto fail;
    if (point[1] < bounds->unk2) goto fail;
    return bounds->unk8 < point[1];
fail:
    return 1;
}
```

Result: **2/27, no size drift** (unlike round 18's `||`-form version of the
same idea, which the permuter section above reports as "0/27 with size
drift, dramatically worse"). This `goto` form's score and size match Shape
A exactly (attempts 1/3/4/5) — the alias is still optimized away with no
codegen effect, confirming the fold is independent of whether the
consistent-alias idea is expressed as a flat `||` or as sequential `goto`s.
None of this round's four promoted axes (retype-to-real-width,
whole-struct-assignment, split-combined-expression, `return` on
wrongly-`void`) apply here: there is no masked/narrow parameter to retype
(the `s8` sign-extension is already confirmed correct), no struct copy,
no single combined read-modify-write expression to split, and the function
is already correctly `s32`-returning. **Genuinely a different residue
shape from what those axes address** — this one is a register/alias
identity choice on a PARAMETER with no arithmetic width or struct-copy
component at all.

**Disposition unchanged: `INCLUDE_ASM`, stall confirmed again, still a
permuter target for a future round with fresh compute budget** (round 18's
600s bounded search reached 770/1040, not zero, in 71363 iterations — a
longer or differently-seeded run is the remaining lever, not more hand
reshaping).

---

## Round 53 (bravo) — CALIBRATION attempt, one fresh shape, negative

Assigned as one of three functions in a round-53 Sonnet calibration slot for
the track-1 stop rule (`docs/FINISHING-PLAN.md`); this report's own text is
the reason it was picked over the plan's higher-ranked but
levers-measurably-spent `code_2cc8c_e` job — round 18 and round 46 both say
explicitly this is NOT permuter-exhausted, so a real attempt was owed before
any further stop-rule conclusion.

**Re-checked the round-46 Gate 3 verdict first, per this round's brief.**
Nothing has changed the scaffold-vs-real-build mismatch it found (no maspsx
flag, no toolchain change, no struct edit to this function's types since
round 46) — re-running the scaffold would reproduce the same disagreement,
so no search was launched, consistent with "no CHANGED state, do not
re-spend."

**Attempt 8** (a new shape, not among the seven above): a single nested
ternary collapsing all five outcomes into one `return` expression, on the
theory that a fundamentally different EXPRESSION FORM — not another
if/`||`/`goto` restatement of the same four-early-exit CFG — might make
cc1's RTL expansion allocate differently:

```c
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    return (bounds == NULL) ? 1
        : (point[0] < bounds->unk0) ? 1
        : (bounds->unk4 < point[0]) ? 1
        : (point[1] < bounds->unk2) ? 1
        : (bounds->unk8 < point[1]);
}
```

**Result: 2/27, exact length (0x6C, no drift) — but a THIRD, distinct
compiled shape, not a repeat of Shape A or B.** The raw `funcdiff` output
shows a different instruction sequence throughout (not just a different
score composition); it does not recover either missing instruction (`move
a2,a0`; the `sra a0,a3,24` re-extract). No better than the existing best.
Reverted (`git checkout -- src/class_3bb8c_b.c`; clean `OK: build matches
retail` confirmed immediately after).

**Disposition unchanged: `INCLUDE_ASM`, still 2/27, still not
permuter-exhausted in the technical sense (Gate 3 still disagrees so no
search was run), still a register-identity residue per CLAUDE.md HARD RULE
6** — not something a `register`/asm-constraint fix is permitted to close.
Eight independent source restatements (six from rounds 18/19, the round-19
`goto`-with-consistent-alias as a seventh, this round's nested ternary as an
eighth) have now converged on only three possible compiled shapes, none
retail's. This is the honest negative half of this round's Sonnet track-1
calibration measurement: no padding, no new lever found, the class's cheap
levers read as spent for this function.

### Proposed learning

**"Restate the CFG" (if-chain / `||` / `goto`) and "restate the whole thing
as ONE expression" (nested ternary) are different axes, and testing every
combination of the first without trying the second leaves a real gap** — but
here they converged on the same conclusion (2/27, the same two missing
instructions) rather than the ternary opening new ground. Recorded so a
future round does not re-spend an attempt re-deriving "does a
single-expression form change anything" for this function: it does not, at
least for this specific nested-ternary spelling.

---

## Round 58 (bravo) — round 46's Gate 3 verdict is VOID (same unit error as `Class866E8__SplitFootprintSlot`), which RE-VALIDATES round 18's 71,363-iteration search as a genuine negative; four fresh source shapes, all inert at 2/27

Rebuilt Shape A (the round-19 `goto` body) from a clean `INCLUDE_ASM`
baseline first, per Gate 1b: **2/27, exact length, zero outside-range drift**
— identical to rounds 19 and 46.

### The changed state, and the report line it contradicts

Round 46's entry above says, of a freshly built scaffold:

> base score 2900 -- 14 insertions, 14 deletions … The REAL in-context build
> … shows **ZERO** insertions or deletions … An isolated scaffold reporting
> 14/14 against a target the real build reproduces with 0/0 is this round's
> Gate 3 failure mode … **Per Gate 3, a disagreement means STOP: no search
> launched.**

**That comparison is invalid, and in the same specific way round 58 found on
`Class866E8__SplitFootprintSlot`: `funcdiff.py` does not report insertion or deletion counts
at all.** The "real build shows 0/0" half was never measured — it was
inferred from "2/27, exact length, no outside-range drift". Equal length does
not mean zero insertions and zero deletions; it means they **cancel**. This
function is 27 words with 2 matching, so 14 insertions against 14 deletions
is not merely consistent with the real build, it is exactly what the real
build looks like.

**Gate 3 check 3 run the way it is defined — on BYTES:**

```sh
tools/setup-permuter.sh IsPointOutOfBounds <Shape-A seed> permuter-work/cd38
tools/binutils/bin/mipsel-linux-gnu-objdump -d permuter-work/cd38/base.o
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/class_3bb8c_b.c.o
```

The two disassemblies of `IsPointOutOfBounds` are **identical instruction for
instruction**. The only differing lines are the absolute targets of the four
`beqz`/`bnez` and the one `j` (`beqz a0,64` vs `beqz a0,814`) — which every
standalone object shows, because those are section-relative until link time.
**The scaffold is faithful. It always was.**

### The consequence is the opposite of "unblock a search"

Round 46's reasoning did two things, and voiding it undoes both:

1. It refused to launch a search. That refusal was unfounded.
2. It retroactively **discredited round 18's search** — "why round 18's
   71363-iteration search never reached zero … it was never scoring this
   function's actual residue at all, only the isolated scaffold's own
   different structural gap."

Since the scaffold is faithful, **round 18's 71,363-iteration, 600-second
search was scoring this function's real residue, and its failure to reach
zero is a genuine negative.** So the correct queue status for this function
is **permuter lever SPENT, not permuter lever BLOCKED** — and round 46's
follow-on suggestion ("worth someone checking whether a scaffold built with
more of this file's other functions included changes the picture … head-scale
work") is **unnecessary**: single-function isolation is not the problem,
because the isolated compile reproduces the real one byte for byte.

No new search was run here for exactly that reason: re-running round 18's
search with fresh RNG on the same seed is a recorded attempt, not a result.

### Four fresh source shapes, all inert

The residue, re-read with `asm-differ`, is sharper than "a whole-parameter
register identity choice":

```
retail                          ours (2/27)
3d538: move a2,a0               (absent)
3d53c: beqz a2,3d59c            3d538: beqz a0,3d59c
3d540: li   v0,0x1              3d53c: li   v0,0x1
3d544: lbu  v1,0(a1)            3d540: lbu  v0,0(a1)
3d548: lh   a0,0(a2)            3d544: lh   v1,0(a0)
...                             ...
                                3d558: li   v0,0x1      <- re-materialised
                                3d56c: li   v0,0x1      <- re-materialised
                                3d598: li   v0,0x1      <- re-materialised
```

**Retail dedicates `$v0` to the constant `1` for the whole body** — it is set
once in the `beqz`'s delay slot and survives to the shared `jr ra`, because
retail's scratch is `$v1`/`$a0`/`$a2`/`$a3` and it bought `$a0` as scratch
with the `move a2,a0`. Ours uses `$v0` itself as the scratch register, so it
must re-materialise `li v0,1` before three of the four exits. That is one
decision, not two: the missing `move a2,a0` and the three duplicated `li`s
are the same choice seen at both ends.

That reading suggested a shape the attempts list above does not contain —
**naming the returned constant as a variable assigned once at the top**, so
that the source itself carries a value live across the entire body. Attempts
3, 4 and 5 named `bounds` and named the `s8` bytes; none named the `1`.

| # | shape | result |
| --- | --- | --- |
| 8 | `s32 result; result = 1;` then `return result;` at each of the four exits | **2/27, inert** |
| 9 | the same, with `goto fail; … fail: return result;` | **2/27, inert** |
| 10 | `do { … break; … } while (0);` around the four guards, `return 1;` after | **2/27, inert** |
| 11 | shape 10 plus both the `result` variable and the `bounds` alias | **2/27, inert** |

Shapes 10 and 11 are round 58's own new lever, the one that moved
`Class866E8__SplitFootprintSlot` from 55/97 to 62/97 (a `do { } while (0)` is a real loop to
GCC 2.6.3's loop pass where a plain brace block is nothing). **It does not
apply here**, which is the useful half of the negative: that lever addresses
scheduling and delay-slot placement, and this function's residue is a
register-class assignment decision taken well before scheduling. Shapes 8 and
9 join attempts 4 and 5 in the same bucket the report already names — the
compiler folds the alias away and the source-level naming has no codegen
effect.

**Disposition: unchanged. `INCLUDE_ASM`, 2/27, exact length, zero drift.**
What changes is the queue metadata, not the score: the permuter lever is
spent rather than blocked, and the head-scale "build a scaffold with more
file context" item round 46 raised can be closed.

### Proposed learning

**Gate 3 check 3 is a check on BYTES, and comparing two tools' summary
numbers is not it.** Three separate rounds across two functions (19 and 33 on
`Class866E8__SplitFootprintSlot`, 46 here) read a permuter `--debug` structural summary against
a figure they believed `funcdiff.py` had reported, concluded the scaffold was
unfaithful, and stopped. `funcdiff.py` reports exactly three things: words
equal at the same offset, bytes differing outside the function's range, and
staleness warnings. **It never reports insertions or deletions**, so any
sentence of the form "the real build shows N insertions" is an inference from
"length exact" — and equal length is the case where insertions and deletions
CANCEL, which is precisely the case a structural diff is worth running.
Run `objdump` on `base.o` and on `build/src/<unit>.c.o` and diff the two;
expect the absolute `j`/branch targets to differ and nothing else. When they
do, the scaffold is faithful whatever its score says.

**And a scaffold verdict is load-bearing in both directions.** Round 46's
finding did not only stop its own search; it discredited an earlier round's
71,363-iteration negative, which would have sent a later round to re-spend
that budget. A wrong verdict about a TOOL propagates further than a wrong
score, because every report that cites it inherits it silently — the same
shape as CLAUDE.md's "a wrong SCORE is corrected the next time anyone
measures; a wrong CAUSE is what the next round acts on."

---

## Round 60 (charlie) — NON_MATCHING body promoted, round 60

Track 1b promotion. Score re-verified unchanged (2/27, exact length, zero
drift) before promoting. Placed Shape A — the plain four-early-return form
(`This maps directly onto:` above; byte-identical to attempts 1/3/4/5 per
rounds 19/46/58) — in `src/class_3bb8c_b.c` inside `#ifdef NON_MATCHING`,
with `INCLUDE_ASM` in the `#else`. Chosen over the goto/alias form because
it is the plainer restatement of the same compiled shape and no more or
less byte-exact. Both oracles green: `./build-and-verify.sh` (exit 0, `OK:
build matches retail`) and `tools/check-nonmatching.sh` (exit 0). No bytes
changed; disposition otherwise unchanged (still `INCLUDE_ASM` in the
verified build, still a HARD RULE 6 register-identity stall, not
re-attempted by hand this round).

## Naming

**Tier A.** Confirmed NOT a vtable slot (`tools/classtable.py gClass866E8Methods`
has no entry at this address) -- a plain, non-virtual helper, so it takes
no `self` and gets no `Class866E8__` prefix. Still `INCLUDE_ASM` (a
documented STALL), named per this round's brief since the evidence for
its mechanics is solid: four comparisons of an `[x,y]` point against a
`Bounds866E8_3bb8c_b`'s four edges, returning nonzero when the point is
outside any of them. Mechanics-is-purpose leaf.
