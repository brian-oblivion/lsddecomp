# TodActor__SetLightMode -- MATCHED (40/40, round 75): lever = LOOP KIND (plain `for`, no barrier, no frame filler)

> Renamed from `Class65650__SetLightMode` on 2026-09-26 (tools/rename.py). Address 0x80065ae0.

> Renamed from `func_80065AE0` on 2026-09-24 (tools/rename.py). Address 0x80065ae0.

REVISITED, round 75: MATCHED 40/40, whole-image `OK: build matches retail`; names/types not relevant (control-flow shape only; the `base` temporary was also dropped for a direct call, not measured separately).

## Round 75 (charlie): matched

**Preserved body rebuilt first** (the guard + `do/while` body with
`u8 unused[8]`, identical to the `#ifdef NON_MATCHING` body that stood in
`src/`): 33/40, `insertions 2 / deletions 2`, positional skeleton diffs 7 --
reproduces the recorded figure.

Applied the lever that closed its sibling `TodActor__SetDisplay` earlier this round:

| build | variant | result |
| --- | --- | --- |
| 1 | preserved body | 33/40, ins/del 2/2, skeleton 7 |
| 2 | `for (i = 0; i < self->unk6C; p++) { i++; call; }` (the `TodActor__SetDisplay` shape) | 38/40, ins/del 0/0: prologue, frame and guard exact; `addiu $s0` and `move $a1,$s3` swapped around the vtable load / `jalr` slot |
| 3 | `for (i = 0; i < self->unk6C; p++) { call; i++; }` | **40/40** |
| 4 | `for (i = 0; i < self->unk6C; i++, p++) { call; }` | **40/40, OK: build matches retail** (kept: the plainest spelling) |

```c
void TodActor__SetLightMode(TodActor *self, void *arg)
{
    Unk70ElemObj **p;
    s32 i;

    p = self->unk70;
    for (i = 0; i < self->unk6C; i++, p++) {
        (*p)->methods->slot70(*p, arg);
    }
    GetActorMethods()->slot70(self, arg);
}
```

The recorded compound residue (three-way callee-save store permutation plus
the `arg` copy deferred past the `blez`) and the `u8 unused[8]` filler were
all artefacts of writing the loop as guard + `do/while`; a `for` loop
produces retail's prologue order, early parameter copy and 0x30 frame by
itself. Where `i++` sits in the source picks where its `addiu` lands:
at the body top it is scheduled before the vtable-slot load (retail
`TodActor__SetDisplay`); after the call or in the increment clause it fills the
`jalr` delay slot (retail here).

### Proposed learning

See `TodActor__SetDisplay.md` round 75: a guard + `do/while` stall carrying a leading
`__asm__("")` and a `u8 unused[N]` frame filler is the wrong loop kind;
rewrite as a `for` and drop both compensations. Three of three in this unit.

## History before round 75 (superseded title: "TodActor__SetLightMode -- STALL: length EXACT (40/40 words, no drift); 33/40 raw word-match; first real diff at file 0x0562F0 / vram 0x80065AF0")

**Unit:** code_55dd4 · **Size:** 40 words (0xA0 bytes) · **Status:** STALL —
**LENGTH exact (40/40 words, no drift); RAW WORD-MATCH 33/40; FIRST REAL DIFF
at file 0x0562F0 / vram 0x80065AF0** (a compound: the same 3-way callee-save
store-order permutation as `TodActor__SetDisplay`, immediately followed by the
`arg` parameter's deferred copy into `$s3`). Whole-image red. Restored to
`INCLUDE_ASM`.

## What it does (fully derived, and it is — everything past the prologue byte-matches)

The same for-each-then-base-call shape as `TodActor__SetDisplay`'s STALLED loop
(same `self->unk70`/`self->unk6C` array), but through a DIFFERENT vtable
slot (`+0x070`, not `+0x060`) on each element, and followed by one more
call to the shared intermediate base class's OWN `+0x070` slot after the
loop:

```c
void TodActor__SetLightMode(TodActor *self, void *arg)
{
    Unk70ElemObj **p;
    s32 i;
    D800878D4Methods *base;

    p = self->unk70;
    if (self->unk6C > 0) {
        i = 0;
        do {
            (*p)->methods->slot70(*p, arg);
            i++;
            p++;
        } while (i < self->unk6C);
    }
    base = GetActorMethods();
    base->slot70(self, arg);
}
```

This resolves a new slot on BOTH `Unk70ElemMethods` (`+0x070`, alongside
the already-known `+0x060`) and `D800878D4Methods` (`+0x070`, extending
the base class beyond the previously-known `+0x050`/`+0x038`) —
coincidentally the same offset on both unrelated classes, confirmed
independently from each call site's own bytes, not assumed from the
coincidence.

**Loop body, epilogue, and the trailing base-class call are all
byte-identical to retail** in every attempt below — the only residue is in
the 7-word prologue.

## The residue: the `arg` parameter's copy into its callee-saved register is deferred

Retail's prologue copies both parameters into callee-saved registers
*before* the `blez` that guards the loop:

```
addiu $sp, $sp, -0x30
sw    $s2, 0x20($sp)  ;  move $s2, $a0(self)
sw    $s3, 0x24($sp)  ;  move $s3, $a1(arg)      <-- happens HERE, early
sw    $s0, 0x18($sp)
sw    $ra, 0x28($sp)
sw    $s1, 0x1c($sp)
lw    $v0, 0x6c($s2)   ; count
lw    $s1, 0x70($s2)   ; array pointer
blez  $v0, TAIL / delay: $s0 = 0
```

Every attempt here instead defers `move $s3, $a1` to right AFTER the
`blez`, as an extra standalone instruction in the not-taken fallthrough —
one word longer, which is exactly the `TodActor__FindPartIndex`/`TodActor__SetDisplay`
"parameter copy deferred into/past the branch" residue class. Two
sub-issues bundled together, addressed separately:

1. **Frame size.** A straight translation (4 meaningful locals: `p`, `i`,
   `base`, no filler) produced a `$sp,0x28` frame instead of retail's
   `$sp,0x30` — an 8-byte shortfall, not the usual 0-byte one. Adding a
   fifth, genuinely unused `u8 unused[8];` local (the same lever as
   `TodActor__FindPartIndex`/`TodActor__SetDisplay`) fixed the frame to `$sp,0x30` exactly
   and, as a side effect, also fixed the callee-save STORE ORDER (which
   had independently come out wrong — `s2,s3,ra,s1,s0` instead of
   retail's `s2,s3,s0,ra,s1` — before the padding local was added; after
   adding it, the order matched with no further work). This is new: on
   `TodActor__SetDisplay` the store-order residue needed a barrier and still
   never fully closed; here the padding local alone fixed it for free.
2. **The `arg` copy deferral, 33/40 ceiling.** With the frame fixed, this
   is the ONLY remaining residue, and it is the mirror image of
   `TodActor__SetDisplay`'s own first fix: there, `__asm__("")` as the function's
   first statement closed an identical-looking deferral. **Here it does
   the opposite** — it not only fails to move the `$s3` copy earlier, it
   ALSO breaks something that was already correct (the `blez` delay
   slot's `$s0 = 0` fill), reintroducing an extra `nop` and a full 1-word
   size drift on top. Confirmed by placing the barrier in three positions
   (function's first statement before `unused` is "used", between
   `p = self->unk70;` and the `if`, and — implicitly — its total absence),
   and only its ABSENCE gives the trustworthy, non-drifted 33/40.

| attempt | result |
| --- | --- |
| straightforward translation (`p`, `i`, `base`, no filler) | 4/40, frame 8 bytes short, drift |
| add `u8 unused[8]` | **33/40, no drift** — fixes frame AND store order in one step; only the `$s3` deferral (7 words) remains |
| same + `__asm__("")` as the very first statement | worse: 7/40, drift reappears (delay-slot fill breaks too) |
| same + `__asm__("")` between `p = self->unk70;` and the `if` | worse: 5/40, drift |
| add a `s32 count = self->unk6C;` local (matching retail's own count-before-pointer read order) instead of re-reading `self->unk6C` inline for the guard | 33/40, unchanged — no effect either way |
| declare `unused` first vs. last among the locals (declaration-order permutation) | 33/40, unchanged |

Six real attempts. The frame-size/store-order half of this residue is
fully closed (a first for this unit's "prologue permutation" residue
class — `TodActor__SetDisplay` never got its store order to close even with a
barrier). Only the parameter-copy deferral remains, and unlike its two
prior instances in this unit (`TodActor__FindPartIndex`, `TodActor__SetDisplay`), the
established `__asm__("")` lever actively regresses it here rather than
fixing it.

## Preserved body (best attempt, 33/40, no size drift)

```c
#if 0
void TodActor__SetLightMode(TodActor *self, void *arg)
{
    u8 unused[8];
    Unk70ElemObj **p;
    s32 i;
    D800878D4Methods *base;

    p = self->unk70;
    if (self->unk6C > 0) {
        i = 0;
        do {
            (*p)->methods->slot70(*p, arg);
            i++;
            p++;
        } while (i < self->unk6C);
    }
    base = GetActorMethods();
    base->slot70(self, arg);
}
#endif
```

(`Unk70ElemMethods::slot70` and `D800878D4Methods::slot70` are kept live
in `src/world/TodActor.c` — both are confirmed correct by the
byte-identical loop body, call sequence, and trailing call, independent
of this stall.)

### Proposed learning

**The `__asm__("")`-as-first-statement lever for a deferred parameter copy
is not safe to apply by pattern-match alone — verify each instance.**
It fixed this exact-looking residue in `TodActor__FindPartIndex` and (partially) in
`TodActor__SetDisplay`, but on `TodActor__SetLightMode` it actively regresses an
ALREADY-correct delay-slot fill and reintroduces a full-word drift on top
of failing to move the copy. Three instances of "a parameter copy is
deferred past a branch" in one unit, two different correct responses (add
a barrier / add nothing at all) — the shape of the residue does not
predict which lever closes it. Always rebuild and re-diff after applying
this lever; do not assume it from the previous function's report.

Separately, confirmed positively this round: **an 8-byte unused padding
local can fix a callee-save STORE ORDER for free, with no barrier at
all**, when it's also needed to correct the frame size. `TodActor__SetDisplay`
needed a barrier for its store-order residue and never fully closed it;
this function's store order was wrong before the padding local and
correct after, with nothing else changed.

## Round 18 (permuter pass, charlie)

**Important methodology finding, discovered here first:** the first bounded
search (`permuter.py --debug`/search WITHOUT `--stack-diffs`) reported a
"zero" at iteration 197. **The literal winning candidate** (from
`permuter-work/TodActor__SetLightMode/output-0-1/source.c`, function body only):

```c
#if 0
void TodActor__SetLightMode(TodActor *self, void *arg)
{
    Unk70ElemObj **new_var;
    u8 unused[8];
    Unk70ElemObj **p;
    s32 i;
    D800878D4Methods *base;

    p = self->unk70;
    i = 0;
    if (i < self->unk6C) {
        i = i;
        do {
            (*p)->methods->slot70(*(new_var = p), arg);
            i++;
            p++;
        } while (i < self->unk6C);
    }
    base = GetActorMethods();
    base->slot70(self, arg);
}
#endif
```

This is what a false permuter zero looks like: a plausible-reading but
UB-adjacent form (`new_var` assigned inside a dereference expression
purely to perturb liveness, a dead `i = i;` self-assignment, `i = 0;`
hoisted above the guard and the guard rewritten as `i < self->unk6C`
instead of `self->unk6C > 0`). **How the false positive was caught:**
translating it to `src/world/TodActor.c` in place of the `INCLUDE_ASM` and
running `./build-and-verify.sh` + `tools/funcdiff.py` -- the mandatory
"a permuter zero is a LEAD, not an answer" step -- showed it was
**NOT a real match**: 28/40 words, WITH an 8-byte frame-size drift
(`addiu $sp,$sp,-0x38` vs retail's `-0x30`), i.e. the same class of false
positive documented in `ApplyMatrixToLVArray`'s report this round: without
`--stack-diffs`, permuter's scorer cannot see a uniformly-shifted
`$sp`-relative offset and will call a frame-broken candidate a "zero".
Confirmed directly: `permuter.py --debug --stack-diffs` on the unmodified
seed gives base score 107 (32 stack-difference points + others), not the
75 recorded from the earlier non-`--stack-diffs` `--debug` check.

Relaunched properly with `--stack-diffs`. Ran to its bound:
**`permuter exit=124`** (timeout fired), 26,892 iterations. Best score
reached: 58 (down from 107), via `p = self->unk70; i = 0; if
(self->unk6C > 0) { do {...} while (...); }` -- i.e. hoisting `i = 0;`
to before the guard, dropping it from inside the `if` block. **This is
NOT an improvement in real terms**: translating this exact shape to
`src/` and checking against the real oracle (done manually, since 58 is
not 0) gives **28/40 with an 8-byte frame OVERSHOOT** (`-0x38` instead of
retail's `-0x30`) -- worse than the existing preserved 33/40 body. So the
permuter's own relative score (58 < 107) does not imply a real-oracle
improvement here; it optimizes register/reordering penalties that
partially trade against a frame-size regression the `--stack-diffs`
scoring only partially weights. Reverted immediately, `INCLUDE_ASM`
untouched throughout (all testing done via direct `src/` edit + rebuild +
revert, never left uncommitted).

No zero reached by either search. Per the head's standing instruction,
**not marked permuter-exhausted** -- two runs (600s equivalent total)
under heavy contention explored two different scoring configurations
without finding the actual residue's fix; the parameter-copy-deferral
axis documented in this report's original analysis (the `$s3`/`arg`
copy timing) was never independently reproduced as the blocker by either
search, so it remains the best-supported theory for what is actually
missing. Remains a STALL at 33/40 (the original preserved body, still the
best REAL score found for this function this round).

### Proposed learning

**A lower permuter score is not evidence of a real-oracle improvement
when the candidate also changes frame size.** `--stack-diffs` makes
frame-size differences visible to the scorer, but its linear per-word
penalty does not necessarily rank a frame-broken-but-locally-better
candidate below a frame-correct-but-locally-worse one the way
`funcdiff.py`'s in-range word count does. Any permuter candidate that
touches a local's live range enough to plausibly move frame size should
be verified against the real oracle before being reported as progress,
even under `--stack-diffs` -- lower permuter score alone is not
sufficient, only `build-and-verify.sh` + `funcdiff.py` settle it.

## Round 19 (echo): checked against ApplyMatrixToLVArray's dead-code-arity finding
## (does not apply here), one more lever tried (negative)

The head flagged this function as possibly sharing a cause with
`ApplyMatrixToLVArray`'s frame-size gap (both were frame-size-adjacent stalls).
**Checked directly: this is NOT the same class.** `ApplyMatrixToLVArray`'s
signature was "the frame reserves MORE outgoing-argument space than any
live call site needs" (a pure `$sp`-relative reservation gap, with the
live call site's own bytes already byte-perfect). Here the frame size is
ALREADY CORRECT (`-0x30`, matching retail exactly, confirmed again this
round) once the padding local is present -- the residue is entirely
about WHEN an already-correctly-sized callee-saved register (`$s3`,
holding `arg`) gets its value, not about how much stack is reserved.
There is no unexplained reservation to hunt for a dead call site here;
the two functions' stalls are only superficially similar (both mention
"frame"), not causally related.

Re-verified the 33/40 claim first (matches exactly, no drift). Tried one
lever not in the original attempt table: indirecting BOTH `self` and
`arg` through freshly-assigned locals (`s = self; a = arg;`) right at
function entry, on the theory that giving `arg` an early, otherwise-inert
reference (mirroring `self`'s early use via `p = self->unk70;`) might
persuade GCC to materialize its callee-saved copy at the same point.
Result: **33/40, IDENTICAL residue, no change whatsoever** (same 7-word
diff, same values, no drift either way) -- this axis, which helped
close/narrow register-identity residues on `SceneNode__AddToActorParents` and
`StageMap__Finalize` earlier this round, does nothing for a parameter-copy-
TIMING residue. Reverted immediately.

This is now 8 real attempts (6 from round 14 originally, plus this
round's indirection try, plus -- counting separately since it is a
distinct axis from manual attempts -- 2 permuter runs with proper
`--stack-diffs` scoring) without closing the `$s3`/`arg` deferral.
Filing unchanged as STALL at 33/40, `INCLUDE_ASM` restored.

### Proposed learning

**"Both stalls mention frame size" is not evidence of a shared cause --
check whether the frame size itself is wrong (an arity/reservation
question, `ApplyMatrixToLVArray`'s class) or already correct (a register-
materialization-TIMING question, this function's class) before assuming
one function's fix generalizes to the other.** Separately: the
early-alias/indirection lever (assign a parameter to a fresh local
immediately at entry) that helped two OTHER register-identity residues
this round did nothing for this parameter-copy-deferral residue --
another data point that a lever's success on one residue SHAPE (register
identity/rotation) does not transfer to a superficially adjacent but
mechanically different shape (deferred materialization past a branch).

## Round 20 (alpha): CORRECTION -- round 14's "store order also fixed for free" claim was wrong; the residue is a compound of two issues, not one

Re-verified by dropping the preserved 33/40 body in verbatim and reading
the real diff with `tools/asm-differ/diff.py`, not just `funcdiff.py`'s
word count (the round-19-documented "preserved body claims must be
re-verified" discipline, applied here to a PROSE claim rather than a
word count). The score itself reproduces exactly (33/40, same 7-word
range `0x0562F0`-`0x056308`), so there is **no drift and no
contamination** -- but round 14's own narrative about what that 33/40
consists of does not hold up.

**Round 14 claimed: "the padding local alone fixed the frame to 0x30
AND, as a side effect, fixed the callee-save STORE ORDER... after adding
it, the order matched with no further work" -- leaving only the `arg`
parameter-copy deferral as the residue.** Reading the actual diff:
retail's callee-save store order is `s0 (0x18), ra (0x28), s1 (0x1C)`;
this build's is `ra (0x28), s1 (0x1C), s0 (0x18)` -- the **exact same
three-way permutation** documented as unfixed in `TodActor__SetDisplay`'s own
report, still present, not fixed. The confusion is understandable: the
missing/deferred `arg` copy (retail's word 4) shifts every subsequent
word's file OFFSET by one position, so a naive "does word N match word
N" comparison at the wrong granularity can make an already-wrong store
order look coincidentally offset-aligned with something else. Reading
the actual register/operand content (not just position) shows the
permutation is real and unchanged.

**So this function's 7-word residue is a COMPOUND of two of this unit's
known independent issues stacked together, not the arg-deferral alone:**

1. The same `s0/ra/s1` callee-save store-order permutation that
   `TodActor__SetDisplay` has never closed (3 words: `0x0562F4`-`0x0562FC`), and
2. The `arg`-parameter-copy deferred into the `blez` delay slot instead
   of materializing early (the remaining words, cascading from the
   missing word 4 onward).

This reframes why every barrier position tried here (round 14: function
start, between `p=...` and the `if`) made things WORSE rather than
fixing the deferral cleanly -- they were perturbing a function that
already carried an independent, currently-unfixable residue, not a
function with one clean, isolated problem to solve.

**One more barrier position tried this round, inside the `if` block
before `i = 0;`** (a position not in round 14's or 19's tables -- both
prior tries were BEFORE the branch; this one is AFTER it, inside the
taken path): result **byte-identical, 33/40, no change whatsoever**. A
barrier placed after the branch decision cannot influence what fills the
branch's own delay slot or reorders the preheader before it, which is
consistent with why it's inert rather than harmful (unlike the two
before-the-branch positions, which actively regressed things). Reverted;
confirmed clean rebuild.

Given `TodActor__SetDisplay`'s own report -- with more attempts (15+),
independent permuter search (47,952 iterations), AND this round's
isolated-cc1 confirmation that the permutation is decoupled from frame
composition -- has never closed this exact store-order permutation, it
is very unlikely a lever exists here that would close it either. Filing
the corrected understanding rather than a new fix; remains a STALL at
33/40, `INCLUDE_ASM` restored, `src/` confirmed clean (`git diff --stat`
empty).

### Proposed learning

**A report's PROSE claim about mechanism ("X also fixed Y for free") needs
the same re-verification discipline as its WORD-COUNT claim -- reading
`funcdiff.py`'s score is not enough when the claim is about which
specific instructions changed, only `asm-differ`'s actual operand-level
diff settles it.** The word count here (33/40) was never wrong across
three rounds; the NARRATIVE about what made up that 33/40 was wrong from
round 14 onward, and nothing in the intervening two rounds' re-checks
caught it because they re-verified the SCORE, not the diff's content.
When a report claims a specific mechanism closed (not just "the total
matches"), spend the one extra `asm-differ` call to confirm the actual
instructions match before building on that claim -- especially for
"fixed as a side effect, no further work" claims, which are exactly the
ones nobody has reason to re-derive from scratch.

## Round 24 (echo): re-verified, title corrected to the three-figure format, no new axis found

Re-verified by dropping the preserved 33/40 body in verbatim. `funcdiff.py`
reproduces exactly **33/40**, no drift. `tools/asm-differ/diff.py` confirms
the first real diff (after realignment) is at file offset **0x0562F0** /
vram **0x80065AF0** -- retail's `move s3,a1` (the `arg` copy) is simply
missing at that position in ours, consistent with round 20's corrected
understanding that this residue is the SAME store-order permutation as
`TodActor__SetDisplay` plus the deferred-copy cascade. Corrected this report's
title to the three-figure LENGTH/RAW-WORD-MATCH/FIRST-DIFF format.

Screened against the same three round-21/22/23 levers as `TodActor__SetDisplay`
this round (see that report's round-24 entry for the full reasoning): no
dead parameter to substitute, no register-pair-swap shape for the
pointer-elimination lever to apply to (this is a store-order/deferred-copy
residue, not an identity swap), and the function currently carries no
lever at all (the preserved body has neither a barrier nor any other
addition, so there is nothing to audit for the "levers do not commute"
rule). None applicable -- no new build attempt spent.

Remains a STALL at 33/40, `INCLUDE_ASM` restored, `src/world/TodActor.c`
confirmed clean.

### Proposed learning

Same as `TodActor__SetDisplay`'s round-24 entry: this unit's two store-order-flavor
stalls sit outside every lever the project has found since round 20, and
that is worth recording explicitly rather than re-screening them fresh next
round.

## Round 25 (delta): block-order check (negative)

Per the head's round-25 block-order broadcast: grepped for a bare
unconditional `j` (not `beq`/`bne`/`bgez`) whose target is a join with real
work in its delay slot.

```
grep -nE '\*/\s+j\s' asm/nonmatchings/TodActor/TodActor__SetLightMode.s
```

**No bare `j` mnemonic anywhere in this function's disassembly.** Same
shape as its sibling `TodActor__SetDisplay`: linear prologue, one `do`/`while`
loop guarded by `blez`/`bnez` (both conditional), linear epilogue, one
trailing unconditional call. There is no candidate site for a
basic-block-layout residue. This is consistent with — and further
supports — round 20's corrected understanding that this function's 7-word
residue is a compound of `TodActor__SetDisplay`'s own unclosed store-order
permutation plus a deferred parameter copy, both genuinely scheduling
questions, not a layout one.

Also re-verified this round by temporarily un-wrapping the `#if 0` body
already embedded in `src/world/TodActor.c` and rebuilding: `funcdiff.py`
reproduces **33/40** exactly (file `0x0562E0-0x056380`), no drift.
`INCLUDE_ASM` restored immediately after; `git diff --stat` confirmed
empty before moving on. This function is next in this round's priority
order (after `TodActor__ApplyTodFrame`'s search) — its own permuter scaffold will
be provisioned fresh at that point, since round 18's `permuter-work/` did
not survive between rounds/worktrees.

## Round 31 (alpha): re-verified (no drift), discriminator confirmed, second permuter campaign (negative)

Re-verified first: dropped the preserved 33/40 body in live, rebuilt.
`funcdiff.py` reproduces exactly **33/40**, file range `0x562E0-0x56380`
(0xA0 bytes = 40 words, no drift) — same 7-word residue documented since
round 14. `INCLUDE_ASM` restored, `git diff --stat` confirmed clean.
Blocker screens (`gp_rel`, `mflo`/`mfhi`-into-`mult`/`div`) both clean.

**Applied round 27's callee-saved-register discriminator**: retail and the
preserved body's own compiled prologue both save the identical set
`$ra,$s0,$s1,$s2,$s3` (`grep -oE 'sw +\$(s[0-7]|fp|ra),' ... | sort -u` on
both). Confirms — independently of round 20's own diff-level correction —
that this is genuine order/timing, not a promotion/demotion the address-taken
lever could fix.

**Provisioned the permuter scaffold fresh** (round 18's `permuter-work/` did
not survive), seeding with the exact preserved 33/40 body. `--debug
--stack-diffs` reproduced base score **107** exactly, matching round 18's own
documented figure for this seed (32 stack-difference points + others) —
confirms the scaffold targets the same residue before spending search time.

Ran the bounded search:

```
nohup timeout 600 env PATH=$PWD/permuter-work/bin:$PATH .venv/bin/python3 \
  tools/decomp-permuter/permuter.py -j 6 --stack-diffs --stop-on-zero \
  --best-only permuter-work/TodActor__SetLightMode > /tmp/alpha_permuter_80065AE0.log 2>&1 &
```

**80,272 iterations. Best score reached 58 (down from the 107 base), the
SAME candidate round 18 already found and disqualified** — hoisting `i = 0;`
above the guard, out of the `if (self->unk6C > 0)` block. Confirmed by
inspection this round rather than re-verified against the real oracle a
second time (round 18 already did that verification: 28/40 with an 8-byte
frame OVERSHOOT, `-0x38` instead of retail's `-0x30` — worse than the
existing preserved body, not a real improvement; `--stack-diffs`' scoring
does not fully weight a frame-size regression against the local-reordering
points it recovers). No score below 58 was found across the entire run; no
zero was reached.

Exit status inferred from evidence (nohup detached the process, so no
parent shell captured `$?` directly, the same ambiguity round 18/25 already
documented for this project's background permuter launches): wall-clock
from launch to completion was ~600-670s (bounded 600s timeout plus the
sleep-loop's own 5s polling granularity), and the log's tail carries the
same graceful `resource_tracker: leaked semaphore objects to clean up at
shutdown` warning round 25 identified as the SIGTERM-permitting shutdown
path, not an unclean kill. Points to the bound firing on its own (**exit
124**), not an external kill.

**Combined with round 18's 26,892 iterations, this is now 107,164 iterations
across two independent bounded campaigns, both converging on the identical
58-score candidate and never finding anything better.** Per the project's
standing instruction this is **NOT marked permuter-exhausted** — both runs
executed under multi-runner contention (this round: up to 38 permuter-related
processes visible in `ps aux` at points during the run, consistent with other
worktrees' own searches sharing the machine). Correct framing: **not closed
in 107,164 iterations under load across two campaigns; the one candidate
found below the base score is a confirmed non-improvement (frame-size
regression), not an untested lead.** Remains a STALL at 33/40, `INCLUDE_ASM`
restored throughout (permuter runs happen entirely inside `permuter-work/`,
`src/` was only touched for the drift-reverification step above, immediately
reverted).


---

## HEAD NOTE, round 31 (2026-09-11) -- orphaned workers, negative UNAFFECTED

At 21:29:40 the head `kill -9`-ed every permuter process cwd-ed into this
runner's worktree (9 of them), during a sweep run on the mistaken belief that
this runner had died. **This did not truncate the campaign described above.**

This runner's searches were bounded at `timeout 600`; its last permuter output
artifact is timestamped 21:20, so the campaigns had already completed on their
own bounds roughly ten minutes before the kill. The 9 surviving processes were
**orphaned workers** -- the round-26 phenomenon recorded in PARALLEL-RUNS.md
2c, where `permuter.py -j N` runs a multiprocessing forkserver and killing or
exiting the root leaves workers reparented to init, still holding cores.

So this is a clean independent confirmation of that documented behaviour:
workers outlived a *normally completed* run, not just a killed one. The
iteration counts and exit codes recorded above stand.

## Round 33 (alpha): re-verified (no drift), round-32 levers screened, `volatile` filler tried (inert)

Re-verified first: dropped the preserved 33/40 body in live, rebuilt.
`funcdiff.py` reproduces exactly **33/40**, same 7-word residue at
`0x0562F0-0x056308`. `INCLUDE_ASM` restored, `git diff --stat` clean.

**Round-32 lever 1, tried directly:** `volatile u8 unused[8];` in place of
the plain `u8 unused[8];` filler (this function's preserved body carries
no barrier — round 14 already established any barrier position here
actively regresses it). Result: **33/40, IDENTICAL residue** — the
volatile qualifier on a filler local that is never read or written is
inert here, same as the plain filler. No improvement, no regression.

**Levers 2-5 screened:** (2) register-identity skepticism does not
apply — round 20's corrected understanding (this residue is a compound of
`TodActor__SetDisplay`'s own unclosed store-order permutation plus a deferred
`arg` copy) was independently re-confirmed by round 31's callee-saved-set
discriminator (`$ra,$s0,$s1,$s2,$s3` both sides); re-checked again this
round, unchanged. (3) not applicable — no attempt this round derived
order from emission order. (4) no new permuter run — round 31's own
campaign (its second) already ran to its bound with no zero; a third
blind search adds nothing new. (5) the residue's first diff (`move
s3,a1` present in retail, absent here) is a genuine missing/deferred
instruction, not an addiu/ori encoding artifact — confirmed via
`asm-differ`'s operand-level read, unaffected by this round's checks.

Remains a STALL at 33/40, `INCLUDE_ASM` restored, `src/world/TodActor.c`
confirmed clean before and after.

### Proposed learning

A third data point (after `TodActor__SetDisplay` and `TodActor__ApplyTodPacket` this same
round) that the round-32 volatile lever has no purchase on this unit's
callee-save-ordering-flavored residues: it is inert on an unused filler
local and actively regressive as a barrier substitute, but never an
improvement. The lever's validated use case (accepting an
already-authored body unchanged where a *different* runner's *different*
barrier attempts had regressed it) does not generalize to "add volatile
to whatever local currently supplies the frame padding."

## Round 49 (alpha): re-verified (no drift); fresh open-ended permuter search finds a ZERO, but it is round 18/31's already-rejected candidate under a different name -- CHECK 3's "perfect zero, real build does not match" outcome, confirmed a third time

This unit has been unstaffed since round 33 (per the head's round-49
opening: stale-verdict ground, every lever discovered since never applied
here). Re-verified first: dropped the preserved 33/40 body in live,
rebuilt. `funcdiff.py` reproduces exactly **33/40**, file range
`0x562E0-0x56380` (0xA0 bytes = 40 words, no drift). `INCLUDE_ASM`
restored, `git diff --stat` confirmed clean before proceeding.

Re-provisioned the permuter scaffold fresh against the preserved 33/40
body (`tools/setup-permuter.sh TodActor__SetLightMode <seed>`). `--debug
--stack-diffs` reproduced base score **107** exactly, matching round 18
and round 31's own documented figure -- **CHECK 3: AGREE**, scaffold
targets the same residue as the real build before any search time spent.

Launched a genuinely open (not manually-enumerated) bounded search under
LOW machine contention (`uptime` load average 3.85/32 at launch, a much
quieter box than round 18/31's "up to 38 permuter-related processes"):

```
timeout 900 permuter.py -j 6 --stop-on-zero --best-only permuter-work/TodActor__SetLightMode
```

**Found a score-0 candidate at iteration 501** (rc=0, `--stop-on-zero`
fired). The candidate:

```c
p = self->unk70;
i = 0;
if (self->unk6C > i) {
    i = i;
    do {
        (*p)->methods->slot70(*p, arg);
        i++;
        p++;
    } while (i < self->unk6C);
}
```

i.e. hoist `i = 0;` above the guard and rewrite the guard as
`self->unk6C > i` instead of `> 0` (plus an inert `i = i;` self-assignment
the permuter left in). This is **exactly round 18's and round 31's
already-found-and-rejected candidate** (round 31: "the SAME candidate
round 18 already found and disqualified — hoisting `i = 0;` above the
guard, out of the `if (self->unk6C > 0)` block"), rediscovered from a
fresh random seed in 501 iterations instead of the 26,892 + 80,272 it took
across two campaigns to surface previously — the search space clearly
finds this shape readily; it is the SAME shape every time, not new
ground.

Translated to the real build anyway (round 40's rule: an un-zeroed score
is worth reading, and a `--stack-diffs` **zero** demands checking even
harder before trusting the report's inherited verdict rather than
independently re-confirming it). **Result: 28/40, WITH an 8-byte frame
OVERSHOOT** (`-0x38` instead of retail's `-0x30`) — confirms round 18/31's
figure exactly, from a from-scratch translation rather than a citation.
Reverted immediately; confirmed the reversion rebuilds `build exit=0`,
whole-image green, `git status --porcelain` empty.

**This is round 47's third CHECK-3 outcome, textbook: "scaffold scores a
PERFECT ZERO but real build does not match -> WHOLE-FILE effect, DECLINE
and stop looking inside the function."** `--stack-diffs`' linear penalty
model does not weight a whole-function frame-size regression (the extra
8 bytes ripple through every stack-relative offset in the function, but
each individual instruction-level diff it produces scores as a small,
uniform, everywhere-the-same delta rather than the single large
structural fact it actually is) heavily enough against the local
reordering gain it recovers at the guard. Consistent with, not a
correction to, round 18's own original finding — this round is the
missing "found this again independently, with a low-load machine and a
formal check-3 label attached" confirmation the head's round-49 brief
asked for on stale ground.

No new search queued this round: this function's search space has now
demonstrably converged on the SAME single candidate from three
independent random seeds (round 18, round 31, round 49) at three very
different iteration counts (26,892 / 80,272 / 501), which is stronger
evidence of a narrow, well-explored space than three more separate runs
would add. Remains a STALL at 33/40, `INCLUDE_ASM` restored.

### Proposed learning

**A permuter score of exactly ZERO is not automatically Check-3 AGREE
territory just because the scaffold's own `--debug` base score matched
the real residue before the search started.** Check 3 as originally
written compares the scaffold's STARTING signature against the real
build's residue; it says nothing about whether an individual CANDIDATE
found during the search is representative once translated. Here the
scaffold was faithful (base 107 == real residue) and the search still
surfaced a candidate whose stack-diffs delta genuinely reaches 0 while
the real oracle regresses — the zero is real INSIDE the permuter's
weighting, not a scaffold artifact, but the weighting itself blind-spots
whole-frame-size effects. The fix is round 40's own rule applied
literally: translate and measure, every time, even for a zero — perhaps
*especially* for a zero, since a zero is exactly the score most likely to
be accepted without the oracle check that would catch this.

## Round 63 (charlie): NON_MATCHING body promoted

NON_MATCHING body promoted, round 63. The preserved 33/40 body (round 14's
hand analysis, re-verified without change across rounds 20/24/25/31/33/49,
never bettered by two independent permuter campaigns whose only sub-zero
candidate is a confirmed frame-size regression — see round 49's CHECK-3
entry above) is hand-derived, not permuter-sourced, so it qualifies for
track 1b promotion as-is. Placed in `src/world/TodActor.c` in the project's
`#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` shape, in ROM order,
immediately after `TodActor__SetDisplay`'s own NON_MATCHING block. Comment names
the score (33/40 words, length exact), the residue class (the compound of
`TodActor__SetDisplay`'s own unclosed 3-way callee-save store-order permutation
plus the `arg` parameter's copy into `$s3` deferred past the `blez` guard),
and this report. `./build-and-verify.sh` green (zero bytes changed — the
verified build never compiles the NON_MATCHING half) and
`tools/check-nonmatching.sh` green (11 NON_MATCHING bodies across 4 units,
up from 10/4 before this round's addition). No new build attempt made;
promotion only.

## Naming

Round 75 (charlie), track 3.

- `TodActor__SetLightMode` (was `func_80065AE0`), tier A. Occupies +0x070, overriding SceneNode__SetLightMode; forwards to every part's SetLightMode, then chains the base.

## Track 4 (2026-09-25, round 85, alpha)

The class (id 0x234, table `gTodActorMethods`) is unified as `TodActor` in `include/TodActor.h`: an Actor subclass (its ctor chains to Actor's first) and Entity's base. Any source block above is the pre-unification spelling (the local `TodActorMethods` of `include/code_55dd4.h`, `linkCompanion`/`unlinkCompanion`, `companion2`, `Unk5CObj`/`Unk70ElemObj`); the live body in `src/code_55dd4.c` takes the unified types and the inherited slot and field names (`addChild`/`removeChild`, Actor's `ticker`, `Actor *` parts, `ModelData *` modelData), byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
