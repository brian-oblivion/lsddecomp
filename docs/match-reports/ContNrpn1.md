# ContNrpn1 -- STALL: length ONE WORD SHORT (76/77); 54/77 raw word-match; first real diff at word 55 (vram 0x80034F38)

> Renamed from `func_80034E5C` on 2026-09-23 (tools/rename.py). Address 0x80034e5c.

`asm/nonmatchings/code_179d8_k/ContNrpn1.s`, vram `0x80034E5C`, unit
`code_179d8_k`. Round 25, runner alpha. Round 31, runner bravo
(re-verified, permuter search negative, still stalled).

## Round 31 update (runner bravo): re-verified, decomp-permuter searched, no zero within budget

Re-verified round 27's baseline reproduces exactly: 54/77 raw word-match,
compiled length 76/77 (one short), same fused sign-extend-and-multiply
residue.

Set up `tools/decomp-permuter` against the preserved near-miss body (base
score 115 confirmed via `--debug --stack-diffs`: 1 deletion + 3 register
differences, matching this report's single-missing-instruction
description). Ran a 16-way `-j 16` search for the full 240s budget
(`timeout 240`, confirmed the process exited on the timeout rather than
being killed externally) with `--stop-on-zero --best-only`: **no zero
found in ~36000 iterations.** The single best candidate found (score 5,
reproduced 8 separate times across the run) was inspected and rejected as
a spurious near-miss rather than a real lead:

```c
if (fn != 0) {
    if (a1) {
        fn(ch, a2 & 0xFF);
    } else {
        fn(ch, a2 & 0xFF);
    }
}
```

Both arms of the introduced `if (a1)` are IDENTICAL calls -- this is the
permuter inventing a dead branch that happens to nudge the scheduler
without changing behaviour, not a genuine structural difference from
retail. Per `docs/MATCHING-GUIDE.md`'s explicit caution ("a permuter score
drop is a LEAD, never a RESULT, until the real oracle confirms it"), this
was not carried into `src/` -- a duplicated-arm branch that adds no real
computation is exactly the shape that scores well against the permuter's
own local diff metric while teaching nothing about retail's actual
codegen.

**Not exhausted, just not closed within one bounded search.** Per this
project's rule against upgrading a negative into "permuter-exhausted,"
this remains a live near-miss for a future round's search, not a settled
STALL beyond what round 27 already established.

## Round 32 update (runner bravo): re-verified; narrow volatile tried, NEGATIVE and worse than the round-27 barrier

Re-verified round 27/31's baseline reproduces exactly: 54/77, compiled length
76/77 (one short), same fused sign-extend-and-multiply residue on `sl`.

**Lever 1 (narrow `volatile`), tried on the exact access this report's own
"untested axis" note names, and NEGATIVE -- worse than the barrier round 27
already ruled out.** Round 27 tried a bare `__asm__("")` between `sl`'s
definition and its use and found it inert (byte-identical, 54/77) because the
fusion happens within one basic block during instruction *selection*, before
there is a second instruction on the other side of a barrier to schedule
around. This round tried the narrower instrument instead: `volatile s16 sl =
a1;`. Unlike the barrier, this is NOT neutral: it dropped the score to
43/77 with 261078 bytes of whole-image drift (the length changed). The
mechanism is the same one recorded in `SeqPlay.md` and in
`DECOMPILATION_LEARNINGS.md`'s "narrow a `volatile`" entry for a *different*
fold (div/mod fusion) -- `sl` is a sub-word (`s16`) value, so forcing a
`volatile` access on it changes retail's implicit widening path, not just
whether the fusion happens. Reverted immediately.

This closes out the "untested axis" this report's original derivation
flagged without asserting: both the strong (barrier) and narrow (volatile)
scheduling-fence instruments are now confirmed inert-or-worse for this
specific sign-extend/multiply-fusion residue, which is consistent with the
report's own conclusion that the fusion is an instruction-*selection*
decision (made once, per-expression, before scheduling), not an
instruction-*ordering* one -- exactly the class of residue this project's
`__asm__("")`/`volatile` levers are documented to be unable to reach.

### Round 32 lever checklist (this function)

- **Lever 1 (narrow `volatile`)**: tried, **NEGATIVE** (worse than the
  already-inert barrier -- causes whole-image drift via a widening-path
  change on the sub-word field, not just a missed opportunity).
- **Lever 2 (register-identity verdict is a hypothesis)**: not applicable --
  this residue was never filed as register-identity; it is a peephole-fusion
  residue (fewer instructions, not swapped registers).
- **Lever 3 (emission order != source order)**: not newly tested; round 27
  already ruled out declaration-order variants for the participating locals.
- **Lever 4 (permuter negative is evidence about one search)**: not
  re-run this round -- round 31's 36000-iteration search stands as the
  live data point; not re-attempted given the budget went to functions with
  no permuter history yet (`SeqPlay`, `NoteOn`).
- **Lever 5 (asm-differ/permuter compare text)**: not implicated; the
  residue is an instruction-count difference (fused vs. two-instruction
  shift/sign-extend), not an opcode-vs-immediate text ambiguity.

## What it is

A per-(channel, slot) "note event" dispatcher. It reads two mode flags out
of the record (`unk27`/`unk10` for a one-shot "arm the latch" path, `unk16`
for a small closed set of event kinds) to decide which byte field to
stamp with `a2`, and then -- **unconditionally of which path was taken** --
checks whether `rec->unk16 == 0x28` (freshly re-read from memory) and, if
so, looks up a function pointer in a per-(channel,slot) dispatch table
(`D_80090368`, 0x40-byte-stride rows of 16 pointers) and calls it with
`(channel, a2 & 0xFF)`. It finishes by caching
`ReadDeltaValue(channel, slot)` into `rec->unk88`, the same tail every
sibling function in this file has.

```c
void ContNrpn1(s16 a0, s16 a1, u8 a2)
{
    Entry90902E8 *rec = &D_800902E8[a0][a1];
    u8 kind;
    Fn80090368 fn;

    if (rec->unk27 == 1) {
        if (rec->unk10 == 0) {
            rec->unk28 = a2;
            rec->unk10 = 1;
            goto check;
        }
    }
    kind = rec->unk16;
    if (kind != 0x1E && kind != 0x14) {
        rec->unk15 = a2;
        rec->unk2A = rec->unk2A + 1;
    }
check:
    if (rec->unk16 != 0x28) {
        goto skip_call;
    }
    {
        s16 ch = a0;
        s16 sl = a1;
        fn = D_80090368[ch][sl];
        if (fn != NULL) {
            fn(ch, a2 & 0xFF);
        }
    }
skip_call:
    rec->unk88 = ReadDeltaValue(a0, a1);
}
```

## New struct fields (added to this unit's own `Entry90902E8`, safe -- see below)

This function is the first in the unit to touch `+0x10`, `+0x15`, `+0x16`,
`+0x27`, `+0x28` and `+0x2A`, all of which were previously anonymous padding
in this unit's own copy of the 172-byte record struct. Named them by
splitting the existing `pad8`/`pad15`/`pad2A` arrays without moving any
other field's offset or changing `sizeof(Entry90902E8)` -- verified safe by
rebuilding after the struct edit alone (before writing this function's body)
and confirming `build-and-verify.sh` still reports a byte-exact whole-image
match:

- `+0x10 unk10`: one-shot latch, set to 1 once the "mode 1" branch fires once
- `+0x15 unk15`: cached byte, written from the "default kind" side-effect path
- `+0x16 unk16`: the event-kind selector this function switches on (values
  seen: `0x14`, `0x1E`, `0x28`, and "anything else")
- `+0x27 unk27`: a dispatch-mode selector, compared against `1`
- `+0x28 unk28`: cached byte, written from the "mode 1" latch path
- `+0x2A unk2A`: a second, independent retrigger/step counter (distinct from
  the already-named `unk29`)

## The actual control-flow finding (worth more than the residue itself)

The first read of this function's disassembly suggests three DIFFERENT
call-or-no-call decisions gated by `unk27`/`unk10`/`unk16 == 0x1E`/
`unk16 == 0x14`. **They collapse to one.** Tracing every branch target by
hand (not by intuition -- retail's own encoding buries this) shows the call
only ever happens when `rec->unk16 == 0x28`, checked via a single shared
re-read at label `.L80034F14` that every path funnels into:

- The `unk27==1 && unk10==0` path jumps straight to the reload+compare
  (skipping the `unk16` chain entirely) -- so a `0x1E`/`0x14` value of
  `unk16` is irrelevant on this path; only the freshly-reloaded `unk16==0x28`
  matters.
- On the `unk16` chain, `0x1E` and `0x14` are special-cased **only to skip
  the "default" side-effect write** (`unk15`/`unk2A`) -- they do NOT
  independently decide whether the call happens. Both land back at the exact
  same shared `unk16==0x28` re-check as the fallthrough/default path (the
  0x1E/0x14 cases just happen to always fail that final check, since `0x1E`
  and `0x14` are trivially `!= 0x28`).

So the earlier draft of this function (three nested gotos threading through
`reload:`/`do_call:`/`skip_call:` labels, one per retail label) was
semantically right but needlessly complex; collapsing it to a single
trailing `if (rec->unk16 != 0x28)` after the two mutually-exclusive
side-effect branches is both simpler AND scores higher (39/77 -> 51/77 ->
54/77 as the goto-tangle was progressively simplified toward this reading).
**Proposed learning:** when a retail branch chain reloads the SAME struct
field via a fresh `lbu`/`lw` at a shared merge label reached from multiple
predecessors, check whether the reload is doing all the real decision work
and the earlier branches are just deciding which SIDE EFFECT ran, before
modeling each predecessor as an independently-gated call site.

## The residue that did not close (1 word, ripples through the tail)

```
25738: TARGET sra a1,v0,0x10   CURRENT sra v0,v0,0xe    <- real diff
2573c: TARGET sll v0,a1,0x2    CURRENT (missing)         <- retail's extra word
25740: TARGET addu v0,v0,v1    CURRENT addu v0,v0,v1     (shifted into 2573c)
...                                                       (everything after
                                                           ripples by 1 word:
                                                           andi target reg,
                                                           jal target address,
                                                           etc. -- NOT
                                                           independent diffs)
```

Retail computes the slot (`a1`/`sl`) index into `D_80090368[ch][sl]` in TWO
steps: sign-extend into a real register (`sra a1,v0,0x10`), then separately
multiply by 4 (`sll v0,a1,0x2`). This project's compiler, given the same
source shape, FUSES those two steps into one `sll`+`sra`-by-14 pair (the
same fusion trick retail itself uses for `D_800902E8`'s index in this
function's own preamble), producing one fewer instruction. The channel index
(`ch`, row multiply by `0x40`) does NOT get this treatment on EITHER side,
because `ch` is reused later as the call's first argument -- so a full
sign-extended register value must exist regardless, and the multiply-by-64
is a genuinely separate step in both retail and this build. The slot value
`sl`, by contrast, is used exactly ONCE (only for the index), which is
exactly the condition under which this build's compiler applies the fusion
peephole and retail's apparently does not.

**Axes tried, all inert (same 54/77, same missing word) or regressive:**
- `s16 ch = a0; s16 sl = a1;` named locals before indexing (inert vs. using
  `a0`/`a1` directly -- no change).
- Declaring `sl` as `s32` instead of `s16` (regressed to 47/77 -- extra
  divergences appeared in the row-pointer computation too).
- `Fn80090368 *row = D_80090368[a0]; fn = row[a1];` splitting row-then-column
  into two statements (regressed to 51/77).
- Reordering the `sl`/`ch` local declarations (inert).
- Passing `ch` vs `a0` directly as the call's first argument (inert).

**Untested axis, flagged rather than asserted:** an `__asm__("")` bare
scheduling barrier between `sl`'s definition and its use might block the
fusion peephole without pinning a register (CLAUDE.md's allowed form), but
adding an OPERAND to it (e.g. `"+r"(sl)`) to actually force materialization
reads as exactly the kind of extended-asm operand constraint the register
rule is written to forbid outside the COP2 exception, so this report leaves
it untried rather than reach for it under attempt pressure.

## Round 27 update (runner alpha): the untested axis, tried, negative

Re-verified round 25's baseline reproduces exactly (54/77, 76/77 compiled
length). This round's explicit ask was to try the missing-word cause found
on any one function of the one-word-short cluster against the other three.
Before doing that cross-check, closed out this report's own flagged
"untested axis": a bare `__asm__("")` scheduling barrier placed between
`sl`'s definition and its use (`s16 sl = a1; __asm__(""); fn =
D_80090368[ch][sl];`). **Inert -- byte-for-byte identical output, 54/77
unchanged.** The barrier blocks cross-block code motion (as documented
elsewhere in this project), but the fused shift here happens entirely
WITHIN one basic block during instruction selection for the index
expression itself, before there is a second instruction on the other side
of the barrier for the scheduler to move anything across.

**Cross-cluster answer:** this function's cause (a sign-extend/multiply
fusion peephole triggered by `sl` being used exactly once) does **NOT**
explain `ContModulation`/`ContPortaTime`/`ContPortamento`'s shared residue
(a `move`-based register rescue before a loop counter reclaims a register)
-- confirmed by inspection, not just by absence of a shared fix: this
function has no loop and no register reuse for a counter at all, so the
rescue mechanism the other three share cannot apply here even in
principle. The one-word-short cluster is genuinely **two independent
causes**, not one: this function stands alone, and the other three share
a real (but still unresolved) class among themselves.

## Verification

`./build-and-verify.sh` build exit=2 (clean compile, expected -- this
function restored to `INCLUDE_ASM`). `funcdiff.py ContNrpn1`: 54/77
words match at the point the residue was last measured (before reverting),
compiled length 76/77 words (one short). The struct-field additions above
were verified independently safe via a whole-image byte-exact rebuild before
this function's body was attempted.

## Round 39 update (runner alpha): re-verified; hoist-both-before-either checked (not applicable); one new lever tried, negative

Rebuilt the preserved near-miss body: reproduces exactly, 54/77, compiled
length 76/77 (one short), same fused sign-extend-and-multiply residue on
`sl`.

**Hoist-both-before-either: not applicable.** The residue is a peephole
FUSION (retail keeps `sl`'s widen and its `*4` scale as two separate
instructions; this build's compiler fuses them into one because `sl` is
used exactly once) -- an instruction-SELECTION difference, not a scheduling
question about two adjacent loads/multiplies with a later shared consumer.
There is only one load/index expression here, not a pair to hoist.

**New lever tried: force a second, dead reference to `sl` after the call**
(`if (0) { rec->unk88 = sl; }`), on the theory that giving the compiler a
second (even if unreachable) use might raise `sl`'s apparent use-count
above the single-use threshold that triggers the fusion peephole.
**Negative**: measured against the real oracle, the object was
byte-IDENTICAL to the baseline (54/77, same missing word, same 260994
bytes of whole-image drift from the pre-existing length gap). Unlike this
project's established `if (0) { dead[0]=0; }` idiom for forcing FRAME SIZE
(which works because a declared local ARRAY reserves stack space regardless
of reachability), a dead reference to an already-live SCALAR inside
unreachable code contributes nothing GCC 2.6.3 needs to keep: the whole
`if (0)` block is eliminated before the fusion decision is made, so it
cannot be used to inflate a use-count. Reverted immediately (no net change
either way).

This closes out the "second consumer" axis this residue's fusion-avoidance
question raises; combined with the already-on-file negatives (declaration
order, `s32` widening, row/column split, barrier, narrow `volatile`, and
round 31's permuter search), every lever this project has for defeating an
instruction-selection peephole (as opposed to a scheduling decision) has
now been tried against this specific fusion. `INCLUDE_ASM` unchanged,
`build-and-verify.sh` confirmed byte-exact.

## Round 48 update (runner charlie): re-verified; check 3 re-run, confirms AGREE but already-exhausted -- not re-searched

Rebuilt the preserved near-miss body in isolation: reproduces exactly,
54/77, compiled length 76/77 (one short), first real diff at word 55, same
fused sign-extend-and-multiply residue on `sl`.

**Ran check 3 fresh, per this round's broadcast discriminator** (build the
permuter scaffold, `--debug --stack-diffs`, compare against the real
build's own residue): scaffold **base score 115** (1 deletion, 3 register
differences) -- this is an EXACT match to round 31's own scaffold base
score and penalty breakdown, i.e. the AGREE case (scaffold residue ==
real build's residue). Per this round's discriminator, AGREE means a
search would be meaningful in principle.

**Disposition: NOT re-searched.** Round 31 already spent a full 240s/~36000
iteration `-j 16` search against this exact scaffold shape and found no
zero (one spurious duplicated-dead-branch candidate at score 5, rejected as
not a real lead). Every manual lever this project has for an
instruction-*selection* peephole fusion (as opposed to a scheduling
question) has also already been tried and is NEGATIVE on file: bare
`__asm__("")` barrier (round 27), narrow `volatile` (round 32, actively
regressive), declaration order, `s32` widening, row/column split, a
second dead reference to inflate use-count (round 39). Re-running an
already-searched scaffold with no new seed shape or new axis is not a
good use of this round's one-search-at-a-time budget while
`ContDataEntry`'s newly-discovered frame-size lever (this round, see that
report) and `NoteOn`'s freshly re-scaffolded 62/70 body (also this
round) are live. Treating this function as **SPENT** for this round;
flagging for the next round that a genuinely different seed framing (not
just a re-run) would be needed to learn anything new here.

`INCLUDE_ASM` unchanged, `build-and-verify.sh` confirmed byte-exact
(no source edit made this round).
