# IntermediateBase__SetState — MATCHED round 72 (32/32, length exact; was STALL 23/32 "register identity")

> Renamed from `Obj86B60__NotifyParents` on 2026-09-25 (tools/rename.py). Address 0x8003e4b8.

REVISITED, round 72: MATCHED 32/32, whole image green; names/types not relevant (the lever was call structure, not a type or a name).

## ROUND 72 (runner bravo): MATCHED -- the "register identity" was a global-alloc PRIORITY, set by how many times `self` is referenced

**Rebuilt as given first** (the round-46/62 NON_MATCHING body, barrier
variant, spliced live): **23/32, insertions 2 / deletions 2**, positional
skeleton diffs 9, no drift -- the title's figures reproduce; plan.py's
`len-off` tag is stale (length was already exact).

**Discriminator.** `cc1 -dl` on a standalone reproducer (pinned
pipeline, same output as the image) prints the pseudo stats global-alloc
ranks by, priority = `floor_log2(refs) * refs / live_length`:

```
Register 71 (self)    used 6 times across 14 insns  -> 2*6/14 = 0.857
Register 72 (arg1)    used 4 times across 10 insns  -> 2*4/10 = 0.800
Register 73 (methods) used 4 times across 12 insns  -> 2*4/12 = 0.667
```

Highest priority takes the first free callee-saved register, so `self`
-> `$s0`, `arg1` -> `$s1`, `methods` -> `$s2`. Retail has `arg1` in
`$s0`, so in retail `self` has FEWER references. Six refs = the copy
from `$a0`, the `methods` load, the `unk20` store, and one `move a0`
per call -- THREE calls. But retail's two mode calls end in one shared
`jalr v0; move a0,$s1` (what looked like cross-jumping). If the source
itself has one call there, `self` has 5 refs, 2*5/14 = 0.714 < 0.800,
and the order flips.

**The match:**

```c
void IntermediateBase__SetState(Obj86B60 *self, s32 arg1)
{
    Obj86B60Methods *methods;
    void (*fn)(Obj86B60 *);

    methods = self->methods;
    __asm__("" ::: "memory");
    self->unk20 = arg1;
    methods->slot30(self);
    if (arg1 == 2) {
        fn = methods->slot64;
    } else if (arg1 == 3) {
        fn = methods->slot68;
    } else {
        return;
    }
    fn(self);
}
```

Without the barrier the register identity is already right
(`self`->`$s1`, `arg1`->`$s0`), but `move s0,a1` lands late, after the
`lw s2,0(s1)`. The round-46 barrier puts it back in the prologue. That
is instruction ORDER only (it is the same barrier the stall body already
carried), so HARD RULE 6 allows it. Two barrier-free spellings were tried
standalone and both left the move late: `self->unk20 = arg1;` before the
`methods` load, and `methods` initialised at its declaration. So the
barrier stays.

Builds this round: 5 image builds, 7 standalone. No permuter search (the
first structural lever matched), so Gate 3 was not needed.

The round-13/46 conclusion that this signature (a full swap, 0
insertions/deletions, frame exact) is "outside source-mutation reach" was
WRONG. The swap came from the source's CALL COUNT, which a permuter
rarely changes. That is also why round 46's 82k-iteration search found
nothing.

### Proposed learning (round 72)

**A full callee-saved swap between two parameters, with the frame exact,
is a global-alloc priority tie-break. Measure it with `cc1 -dl`; don't
reason about it.** The `.lreg` dump gives each pseudo's refs and live
length, and priority `floor_log2(refs)*refs/live_length` decides who gets
`$s0`. So the question to ask is "which value does retail reference more
or less often". A shared `jalr; move a0,sN` tail across two `if` arms,
where each arm only differs in the slot loaded, is the tell that the
source had ONE call through a function pointer chosen per arm. Two calls
merged by cross-jumping still count as two references.


> Renamed from `func_8003E4B8` on 2026-09-19 (tools/rename.py). Address 0x8003e4b8.

## ROUND 46 (runner delta): FIRST PERMUTER SEARCH on this function -- 279 base score, ~82,702 iterations under a 600s bound, no real zero; one heuristic-tempting sub-baseline candidate verified WORSE for real

This function had never been permuter-searched (per this round's own
assignment brief). Ran all three mandated checks first.

**1. Correctness / drift:** the preserved body (byte-identical to the one
already on file) was spliced into `src/code_2cc8c_c.c` in isolation
(every other INCLUDE_ASM across all four of this runner's units
confirmed still wrapped) and rebuilt through the full oracle. Reproduces
**exactly 21/32, no outside-range drift** -- matches this report's own
inherited figure precisely; not stale.

**2. Cost (`--debug --stack-diffs`):** base score **279** = 24 stack
differences, 0 branch differences, 11 register differences, 0
reorderings, **1 insertion, 1 deletion**. This is a materially different
signature than the report's own prose ("only the register NUMBER holding
each parameter is swapped throughout... every instruction, branch, offset
match") suggested -- `asm-differ`, re-run this round, shows the mismatch
is not a pure whole-function register-bank swap after all:

```
retail                          this build
sw   s1,0x14(sp)                sw   s0,0x10(sp)
move s1,a0    ; self -> s1      move s0,a0     ; self -> s0
sw   s0,0x10(sp)                sw   ra,0x1c(sp)
move s0,a1    ; arg1 -> s0      sw   s2,0x18(sp)
sw   ra,0x1c(sp)                sw   s1,0x14(sp)
sw   s2,0x18(sp)                lw   s2,0(s0)
lw   s2,0(s1)                   move s1,a1     ; arg1 -> s1, DELAYED
sw   s0,0x20(s1)                sw   s1,0x20(s0)
```

Retail materialises BOTH parameters into s-registers immediately,
back-to-back in the prologue (interleaved with their own save
instructions) -- the classic documented "prologue callee-save order"
shape. This build's `self` is likewise moved into an s-register
immediately, but `arg1`'s move into `$s1` is DEFERRED until right before
its first real use, rather than happening eagerly alongside `self`'s. So
the residue is register identity (both parameters land in the opposite
s-register NUMBER from retail) **plus** a genuine scheduling difference
in WHEN `arg1` gets promoted to a callee-saved register -- not the pure,
single-mechanism swap the original round-13 classification described.

**3. Base-score agreement with the real build:** the scaffold's 279
reproduces the identical 21/32 in-tree residue (same instructions
differing at the same offsets); no scaffold/real-build disagreement.
Proceeded to search.

**Two cheap hand levers tried first** (not permuter, just to rule out the
obvious before spending the search budget): a bare
`__asm__("" ::: "memory")` right after `self->unk20 = arg1;` (no
change, identical 21/32) and swapping the `if (arg1==2)`/`else if
(arg1==3)` comparison order (regressed hard to 17/32 -- confirms retail
genuinely checks `==2` before `==3`, not a symmetric choice).

**Search:** `-j 6 --stack-diffs --stop-on-zero --best-only`, bounded
`timeout 600`. Ran the full 600s bound (ended via the wall-clock bound,
not `--stop-on-zero`) for **~82,702 iterations**. No candidate reached a
real zero. Three sub-baseline-scoring candidates were saved
(`output-58-1` at heuristic score 58, `output-239-1`/`output-239-2` at
239, `output-261-1` at 261) -- per this project's own standing warning
that a permuter score is a heuristic, not a word count, all three were
diffed against the seed and checked, and the most tempting one
(`output-58-1`, well below the 279 baseline) was additionally translated
to idiomatic C and rebuilt through the REAL pinned toolchain:

```c
void IntermediateBase__SetState(Obj86B60 *self, s32 arg1)
{
    Obj86B60Methods *methods;
    Obj86B60Methods *methods2;

    methods = self->methods;
    methods2 = methods;
    self->unk20 = arg1;
    methods->slot30(self);
    if (arg1 == 2) {
        methods->slot64(self);
    } else if (arg1 == 3) {
        methods2->slot68(self);
    }
}
```

**Real oracle result: 2/32, with a large outside-range drift warning --
markedly WORSE than the 21/32 baseline**, despite scoring better on the
permuter's own heuristic. The extra alias apparently gives cc1 enough
freedom to allocate a genuinely different (larger) frame. This is the
same class of permuter false-lead this project's other reports already
document (`Unk18Obj__InitOt.md` round 36, `SsUtChangePitch.md`): a heuristic
score below baseline is a LEAD to verify, never a result to adopt
un-translated.

The other two saved candidates are spurious for different, simpler
reasons, confirmed by inspection alone (no rebuild needed):
- `output-239-1`/`-2`: retypes `arg1` from `s32` to `unsigned short` --
  a real truncation bug (this call site's actual argument range is
  unknown), the same "narrows a parameter's width" artifact class
  documented in `Unk18Obj__InitOt.md`.
- `output-261-1`: reads `new_var` (an aliased copy of `methods`) BEFORE
  `methods` itself is ever assigned -- literal undefined behaviour (use
  of an uninitialised value), exactly the "reject UB, a candidate
  branching on a value read before its first assignment is exploiting
  the scorer" case this round's own instructions call out by name. Not
  translated or tested; rejected on inspection.

The search itself did not close it. But the refined classification above
pointed at a concrete, un-tried lever -- see the next section, which
tried it and got real progress.

## ROUND 46 (runner delta), continued: the eager-materialisation barrier IS the lever -- 21/32 -> 23/32, and the residue is now a CLEAN, pure register-identity stall

Per the "Proposed learning" reasoning above (written before trying it):
if `arg1`'s DEFERRED promotion to a callee-saved register is a real,
separate defect from the register-NUMBER swap, then a bare memory-clobber
barrier placed to force it EARLY -- before `self->unk20 = arg1;`, not
after (the position this report's round-19 attempt used, and got no
change) -- should collapse the scheduling half of the residue even if it
cannot fix the register-identity half.

```c
void IntermediateBase__SetState(Obj86B60 *self, s32 arg1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    __asm__("" ::: "memory");
    self->unk20 = arg1;
    methods->slot30(self);
    if (arg1 == 2) {
        methods->slot64(self);
    } else if (arg1 == 3) {
        methods->slot68(self);
    }
}
```

**Result: 23/32, no outside-range drift -- a real, verified improvement
over the 21/32 that has stood since round 12/13.** `asm-differ` confirms
the mechanism directly: both parameters now move into their s-registers
immediately, back-to-back with their own save instructions, in exactly
retail's OWN prologue instruction order --

```
sw   s0,0x10(sp)   sw   s1,0x14(sp)   move s0,a0
sw   s1,0x14(sp)   move s1,a1         sw   ra,0x1c(sp)  sw s2,0x18(sp)
```

(retail's own order, reproduced exactly except for which s-register
number each move targets) -- and the ONLY remaining difference anywhere
in the function is `self`/`arg1` occupying `$s0`/`$s1` here vs retail's
`$s1`/`$s0`, consistently through every later use (both branch
comparisons, the final `move a0,sN` before `slot68`). Re-ran
`permuter.py --debug --stack-diffs` on this exact body (fresh scaffold,
`permuter-work/func_8003E4B8_v2`): base score **58**, decomposing as
**10 register differences, 0 branch differences, 0 reorderings, 0
insertions, 0 deletions** (the 8 "stack differences" the tool also
reports are its own internal stack-slot-numbering bookkeeping, not a
real structural defect -- there is no length or instruction-order
mismatch left anywhere in the function).

**This is now the textbook 0/0-insertion/deletion, pure-register-diff
signature round 45's `ServiceSoundCueSet` report established as OUTSIDE a
source-mutation search's reach entirely** -- per that finding, did NOT
spend a search on this new, cleaner base score; it would not find
anything a barrier or statement reorder can reach; only a banned
mechanism (`register T v asm("$N")`, an extended-asm operand constraint)
could pin the two registers, and CLAUDE.md forbids both regardless of
outcome.

**Verdict: STALL, IMPROVED to 23/32 (from 21/32).** The improved body
above is the new best-reached and is the one to hand to any future
attempt. Restored to `INCLUDE_ASM`; full oracle re-confirmed green
(`build exit=0`, `OK: build matches retail`) before moving on.

### Proposed learning (round 46, final)

**A residue diagnosed as "pure register-bank swap" can still be hiding a
real, separately-fixable SCHEDULING defect underneath it, and finding
that defect can require nothing more than re-running `asm-differ` with
fresh eyes and trying the barrier at the position the ORIGINAL
diagnosis's own mental model didn't consider (before the store the
value feeds, not after).** This function carried an unchanged 21/32
verdict across rounds 12/13 and 19 because every prior attempt treated
"self and arg1 swap s-registers" as one indivisible fact. It is two
facts: an eager-vs-deferred SCHEDULING choice (fixable, one barrier) and
a register-NUMBER choice (not fixable, CLAUDE.md HARD RULE 6). Closing
the fixable half first is what exposed the second half cleanly enough to
recognise it matches round 45's already-documented unreachable
signature, rather than continuing to look for a single lever that
explains both.

## Round 19: one more attempt, negative

Re-examined per this round's brief (register-shaped verdicts are the
least reliable class). Tried wrapping the whole body in a
`do { ... } while (0)` (the lever that closed `Obj6EAC0__SetChar` this round
for a similar-looking symptom): this REGRESSED to 0/32 with the function
one word LONGER (33 vs retail's 32) and full whole-image drift. Reverted
immediately.

This is now the SECOND confirmed instance this round where the
`do/while(0)` lever (see `Obj6EAC0__SetChar.md`) does not generalise --
`TaskCore__CommitElementScroll.md` also tried it this round and got the same kind of
regression. The lever appears specific to a narrow shape (a single
unconditional call-plus-field-writes block with no branches of its own
inside the wrapped region); this function's own `if/else if` branch
structure disqualifies it the same way `TaskCore__CommitElementScroll`'s loop did.
Verdict unchanged: STALL at 21/32 (full swap of `self`/`arg1` into
`$s1`/`$s0` vs the natural `$s0`/`$s1`), restored to `INCLUDE_ASM`, no
compile errors in a fresh build.

## Round-12 history

**Unit:** code_2cc8c_c · **Size:** 32 instructions · **Attempts:** 5

## Blocker screen (mandatory, round 13 head broadcast)

```
grep -nE 'gp_rel|addiu *\$at, *\$at, *%lo' asm/nonmatchings/code_2cc8c_c/IntermediateBase__SetState.s
```

No hits. NOT toolchain-blocked -- this is a genuine register-identity
residue, verified against the actual instructions below, not assumed.

## What it does

`gIntermediateBaseMethods+0x060` (and `gClass86B60Methods`'s own verbatim-inherited `+0x060`):
records `arg1` into `self->unk20`, dispatches `self->methods->slot30`
(inherited BasicClass slot, `BasicClass__NotifyParents`) unconditionally, then
`self->methods->slot64` (`IntermediateBase__OnState2`, already matched) if `arg1 == 2`,
or `self->methods->slot68` (`IntermediateBase__OnState3`, next in this queue) if
`arg1 == 3`. Fully understood -- the control flow, every field, and every
slot identity all check out and are not in question.

## The C (best reached, does NOT match -- restored to INCLUDE_ASM)

```c
#if 0
void IntermediateBase__SetState(Obj86B60 *self, s32 arg1)
{
    Obj86B60Methods *methods;

    methods = self->methods;
    self->unk20 = arg1;
    methods->slot30(self);
    if (arg1 == 2) {
        methods->slot64(self);
    } else if (arg1 == 3) {
        methods->slot68(self);
    }
}
#endif
```

## The residue

21/32 words match. `tools/asm-differ/diff.py IntermediateBase__SetState` shows the ENTIRE
diff is one thing: retail assigns `self` (the `a0` parameter) to `$s1` and
`arg1` (`a1`) to `$s0`; every build of mine assigns `self`->`$s0` and
`arg1`->`$s1` -- the "natural" ascending parameter-index order. Every
instruction, branch, offset, and the frame size (`-0x20`, 3 saved registers
`s0`/`s1`/`s2` either way) are otherwise identical; only the register NUMBER
holding each parameter is swapped throughout the function body, including
inside the two comparison branches and the final `move a0,sN` before the
`slot68` call.

This is a register-IDENTITY mismatch by CLAUDE.md's own test ("if removing
[a barrier] changes WHICH REGISTER holds a value, it is banned") -- not a
scheduling/order artifact. It does NOT match the project's documented
"prologue callee-save stores in the wrong order" class (`DECOMPILATION_
LEARNINGS.md`): that class has the SAME final register-to-value mapping and
only the STORE INSTRUCTION order in the prologue differs; here the final
mapping itself is swapped (self ends up in a different physical register
than retail), which is exactly what CLAUDE.md's hard rule 6 defines as
unfixable from plain C and explicitly bans `register T v asm("$N")` /
extended-asm operand constraints for.

## Attempts (5, well under the 30 cap; stopped because every variant

reproduced the SAME residue in the same place -- see "Why this reads as a
plateau, not a partial derivation" below)

1. `methods` cached, `self->unk20 = arg1;` BEFORE `methods = self->methods;`
   (guessing retail's literal instruction order backwards from the `sw`
   position) -- 21/32, register swap only.
2. Same but with `methods = self->methods;` FIRST (matches retail's actual
   `lw`-before-`sw` instruction order) -- identical 21/32, same swap. This
   confirms STATEMENT ORDER is not the lever; both permutations produce
   byte-identical output apart from the swap.
3. No `methods` local at all, `self->methods->slotNN(self)` repeated three
   times -- regressed hard (1/32, 178KB outside-range drift): the compiler
   no longer CSEs the `self->methods` load, and a THIRD call reloads it,
   changing the function's own length. Confirms the `methods` local is
   necessary to reach the 21/32 plateau at all, not merely stylistic.
4. `s32 mode = arg1;` extra local, used throughout in place of `arg1` --
   regressed (3/32): adds a 4th saved register instead of reproducing the
   swap, confirms the extra local is not what retail did.
5. `switch (arg1) { case 2: ...; case 3: ...; }` instead of `if/else if` --
   regressed (8/32, longer body): GCC 2.6.3's switch lowering for a 2-case
   dense switch is structurally different from the `if/else` retail clearly
   used (confirmed by the UNCHANGED branch instructions in the passing
   baseline), not a red herring worth re-trying.

## Why this reads as a plateau, not a partial derivation

DECOMPILATION_LEARNINGS' own caution -- "a report with many attempts along
one axis may have explored one branch of the space" -- was checked here:
attempts 1-2 varied STATEMENT ORDER (the axis this project's own precedent
suggests first), attempt 3 varied WHETHER `methods` is cached at all,
attempt 4 varied whether `arg1` is ALSO cached into a fresh local, attempt 5
varied the CONTROL-FLOW CONSTRUCT (`switch` vs `if`/`else`). Four different
axes, one converged plateau (21/32, same swap) and two divergent
regressions (3, 4, 5 all made frame size worse, which is orthogonal
evidence the 21/32 shape is the closest reachable one, not an
under-explored branch). The one axis NOT tried is the banned one (pinning a
parameter to a specific hard register), which the hard rules forbid
regardless of outcome.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no negation of any kind
  in this function.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk; every access is a single struct dereference or vtable dispatch.

### Proposed learning

A register-identity swap where BOTH parameters are simple scalars/pointers
(no aggregate, no array), the swap is FULL (every use of both registers
throughout the whole function body, not just the prologue), and the frame
size matches retail exactly is a strong signature for this specific stall
class -- distinguish it from `TaskCore__RefreshSlotView`'s round-12 "callee-saved file
saturated" class (`grep -oE 'sw +\$s[0-9]' ... | sort -u | wc -l`, which
here is only 3 registers, nowhere near saturating `s0`-`s7`) and from the
documented "prologue store order" class (which keeps the SAME final
mapping). This is neither -- just two hard-register parameters landing in
the opposite pseudo-register numbering than GCC 2.6.3 chose for this
specific queued function, for a reason not visible from the C source shape.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. 5 attempts,
`INCLUDE_ASM` left in place (never removed from the built source).

## Naming (round 55, runner alpha)

**IntermediateBase__SetState** (renamed from `func_8003E4B8`; still a STALL,
23/32 -- naming applies to NON_MATCHING/stalled functions exactly as to
matched ones, per track 3). Tier A: `Obj86B60Methods::slot30` (this
function's own occupant, "IS IntermediateBase__SetState" per the header) is
documented as "inherited BasicClass slot (`BasicClass__NotifyParents`)" --
this function overrides `NotifyParents`, forwarding to the base slot
unconditionally (`methods->slot30(self)`, itself still `slot30` since the
occupant/dispatcher pair sits one level up this project doesn't rename
without a clearer base-vs-derived split) and then, based on its own `arg1`
parameter (stored into `self->unk20`), forwarding again to
`IntermediateBase__OnState2` (mode 2) or `IntermediateBase__OnState3`
(mode 3) -- fully understood control flow per the report's own "What it
does" section above, independent of the register-identity residue that
stalls the byte match.

## NON_MATCHING body promoted, round 62

Placed the round-46 best-reached body (the `__asm__("" ::: "memory")`
barrier variant, 23/32, zero drift) in `src/code_2cc8c_c.c` under
`#ifdef NON_MATCHING`/`#else INCLUDE_ASM`, per track 1b. Confirmed
hand-derived from this report before promoting: round 46's own text
tried the permuter's tempting sub-baseline candidate (`output-58-1`)
translated to C and it scored a real 2/32 with outside-range drift --
worse than baseline -- and was explicitly rejected un-promoted; the body
promoted here is the separate hand lever (a memory-clobber barrier
placed before `self->unk20 = arg1;`) that round 46 tried next and
verified at 23/32 with no outside-range drift, not a permuter output.
`./build-and-verify.sh` green (`build exit=0`, `OK: build matches retail
SLPS_015.56` -- no bytes changed, the verified build never compiles the
`#ifdef NON_MATCHING` half) and `tools/check-nonmatching.sh` green
(`OK: 10 NON_MATCHING body(ies) in 4 unit(s) compile and reference only
linked symbols`). The report's recorded 23/32 figure and residue class
(pure register identity, self/arg1 swapped `$s0`/`$s1` vs retail's
`$s1`/`$s0`) matched the body exactly; no discrepancy to flag.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__NotifyParents. The round-55 name rested on the header's claim that this function is `slot30`'s occupant; it occupies +0x060 (`classtable.py gIntermediateBaseMethods`), and +0x030 is BasicClass__NotifyParents, which it CALLS. It stores its argument in +0x020 (now `state`), passes it to notifyParents, and runs +0x064 (`onState2`) on 2 and +0x068 (`onState3`) on 3. The overrides of +0x060 are named SetState (TaskCore__SetState, Class86B60__SetState) and forward to this base, so the slot is `setState` and so is this occupant. Tier B. The base call is now the inherited `methods->notifyParents(self, state)` rather than a one-argument `slot30(self)`: byte-identical, the state is already in $a1.
