# CalcDreamColor — STALL: exact length (35/35 instructions, zero address drift), 28/35 raw word-match, first real diff at 0x4BD78 (retail computes the table base address early; this build loads `upper` early instead -- the commutative-add operand-order class, sixth confirmed instance project-wide)

> **TITLE REBUILT, round 32 (2026-09-12, runner alpha2).** The old title
> carried no length figure at all, per this round's assignment to fix that.
> Re-measured fresh (spliced the exact preserved body below back in and
> rebuilt) rather than trusting the inherited "28/35 words" figure blind:
> confirmed the compiled function is genuinely **35 words, matching retail's
> length exactly (zero address drift)** -- `funcdiff.py` reports no
> outside-range warning and a direct `objdump` word count confirms 35 words
> compiled. First real diff located via `tools/asm-differ/diff.py`: `0x4BD78`,
> where retail computes the `D_80087E14` table-base address (`lui a0,0x8008`)
> immediately after reading `dynamic`, while this build instead loads `upper`
> early (`lb a0,1(sp)`) — exactly the round-20-confirmed "commutative-add
> operand-order" class this function was already filed under (sixth instance
> project-wide, third unrelated unit). No new attempt made this round beyond
> the re-measurement; the function remains genuinely **PERMUTER-EXHAUSTED**
> per the twelve-thousand-plus-iteration search already on record below, and
> nothing in this round's two findings (the `volatile` scheduling instrument,
> the register-identity-as-hypothesis discriminator) applies here — this is
> neither a hardware-observed access nor a case where any reshape (including
> the operand-order reversal already tried in round 20) discriminates order
> from allocation; both directions of the commutative add were tried and
> produced the identical wrong result, which is the signature CLAUDE.md/
> DECOMPILATION_LEARNINGS already classifies as this specific compiler-level
> class, not a source-reachable one.

**Unit:** DreamSys · **Size:** 35 instructions · **Best reached:** 28/35 words. **PERMUTER-EXHAUSTED** for the legitimate search space (round 2026-09-02, runner BRAVO) -- see "Permuter run" below; one untried legitimate lead is flagged there for a future attempt.

## What it does

Already documented: `@brief Calculates the DreamColor for a given mood.`
Classifies each of the mood's two axis bytes into `{0,1,2}` (via
thresholds `<-3`, `[-3,4)`, `>=4`) in a local copy, then indexes a 3x3
lookup table `D_80087E14[dynamicClass*3 + upperClass]`.

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
DreamColors CalcDreamColor(MoodGraphPoint *mood)
{
	MoodGraphPoint local;
	s8 *p;
	s32 i;
	s8 val;

	local.value = mood->value;
	p = (s8 *)&local;
	for (i = 0; i < 2; i++, p++) {
		val = *p;
		if (val >= 4) {
			*p = 2;
		} else if (val < -3) {
			*p = 0;
		} else {
			*p = 1;
		}
	}
	{
		s32 index;
		s8 *entry;

		index = local.axis.dynamic * 3;
		entry = &D_80087E14[index];
		return entry[local.axis.upper];
	}
}
#endif
```

(`D_80087E14`'s `extern s8 D_80087E14[9];` declaration is kept live.)

## Two real fixes landed; one residue didn't move

**Fix 1 (worked): the classification loop's if/else arm order.** The
"obvious" nested reading (`if (val<4) { if(val<-3) 0; else 1; } else 2;`)
compiled to a genuinely different branch layout than retail's (inverted
outer condition sense, an extra `j`, wrong fallthrough) -- same
"arm-order-must-match-retail's-fallthrough" class as
`Test4StaircaseNodes` and `func_800590E8` (round 2026-08-30-c). Rewriting
as a flat `if (val>=4) 2; else if (val<-3) 0; else 1;` (the ">=4" case
FIRST, matching retail's actual branch-taken/fallthrough split) fixed the
whole first half of the function (word 0-22 all match) on one try.

**Fix 2 (worked): splitting the double-indexed table lookup.** The single
expression `D_80087E14[dynamicClass*3 + upperClass]` computed the FULL
index before adding the array base, one instruction shorter and 5 words
off from retail. Splitting into an intermediate `s8 *entry =
&D_80087E14[dynamicClass*3];` then `entry[upperClass]` matched the total
instruction COUNT (28/35 -> correct 35-word size, no more outside-range
drift) and got 5 more words matching.

**Residue (did not move): which register holds the table-base address vs.
the `upper` byte in the final two `addu`s.** Retail computes the array
base (`lui`/`addiu 0x8008.../0x7e14`) into one register EARLY (right after
reading `dynamic`, before reading `upper`), then adds `upper` last. This
body's natural codegen reads `upper` first instead, and the base-address
computation lands in the OTHER register -- both `addu`s end up register-
swapped relative to retail, with no value or branch-target difference at
all (28/35, same size, same total instruction count).

Reshapes tried on JUST this residue, all four producing the identical
28/35 result:
1. `s8 *entry = &D_80087E14[idx]; return entry[upper];` (shown above).
2. Same, with `upper` pulled into its own named local, assigned AFTER
   `entry` (to force the read to happen later in source order).
3. `dynamic*3` pulled into its own named `index` local before computing
   `entry` (shown above -- this is what's kept live).
4. Two independent named index locals (`idx1 = dynamic*3; idx2 = upper;
   return D_80087E14[idx1+idx2];`) -- this one actually regressed to the
   single-expression form's 23/35, confirming the intermediate-pointer
   split (attempts 1-3) is the right general shape, just not fully
   reachable.

**Argument-register test:** no calls anywhere after the loop (the function
ends with one `lb` and returns) -- `$a0`/`$v1` here are never live into a
subsequent call. Confirmed register-identity, not a missing parameter.

### Proposed learning

Splitting a double-indexed array access (`arr[a*N + b]`) into an
intermediate one-indexed pointer (`&arr[a*N]`) THEN adding the second
index closes both a size gap AND most of a register-identity residue in
one move -- but the LAST piece (which of the two final operands loads
first) can still resist further reshaping. Worth trying as a first move on
any `table[f(x) + g(y)]` residue before assuming it's unreachable; expect
it to get you most of the way, not necessarily all the way.

## Permuter run (round 2026-09-02, runner BRAVO) — PERMUTER-EXHAUSTED

Set up per `tools/setup-permuter.sh` with the kept near-miss body above as
the seed. Base score confirmed 610 in `--debug` mode, matching this
report's own diagnosis (the register-swapped final `addu` pair).

Ran in two windows totaling **~40400 iterations over ~8m55s wall clock**
(`-j 8`, `--stop-on-zero --best-only`):

- **Window 1: cut short by an external, unscoped `pkill -f "decomp-permuter"`
  run by another runner in this round tidying up ITS OWN permuter session**
  -- not a bug in this search; confirmed after the fact by the process
  dying well before this round's own timebox and the log's
  multiprocessing "leaked semaphore" warning (the `SIGKILL` signature).
  27708 iterations before the external kill landed.
- **Window 2: a fresh restart, run to its own bounded, PID-scoped timeout**
  (no pattern-based kill) -- a further ~12700 iterations, no improvement
  over window 1's best.

**Best score reached: 120** (base 610), never 0, across twelve distinct
saved "best" outputs.

### The 120-score candidate was REJECTED

It retypes the function's return as `volatile unsigned int` in place of
the real `DreamColors` enum return type:

```c
volatile unsigned int CalcDreamColor(MoodGraphPoint *mood)
{
	...
	return entry[local.axis.upper];
}
```

`volatile` forces the compiler to treat every access as having an
observable side effect, which changes scheduling/reload behavior enough
to move the residue -- but it is not a claim about the retail source; the
real declaration (`typedef enum DreamColors {...} DreamColors;`, used as
this function's return type everywhere else it's referenced, e.g.
`src/DreamSys.c`'s two call sites) has no `volatile` anywhere in this
codebase. Rejected as a different function, not a match.

### One candidate at 135 is legitimate C and UNTRIED in the real build

A separate saved output (`output-135-1`), score 135, keeps the correct
`DreamColors` return type and makes no UB-adjacent move -- it only routes
the table-base pointer through the loop's own `s8 *p` variable before
assigning it to `entry`:

```c
{
	s32 index;
	s8 *entry;

	index = local.axis.dynamic * 3;
	p = &D_80087E14[index];
	entry = p;
	return entry[local.axis.upper];
}
```

This is a real, legitimate C reshape (reusing `p` as scratch rather than
computing `entry` directly) that was NOT tried by hand in the four manual
attempts already on record above, and it scored better (135 vs. the kept
attempt's 610... note: attempt 3's kept form above is actually the
610-scoring BASE for this permuter run, i.e. the intermediate-pointer
split alone was not enough; THIS candidate adds the `p`-then-`entry`
indirection on top of it). Per this round's "do not start anything new"
instruction, it was NOT applied to `src/DreamSys.c` or verified against
`build-and-verify.sh` this round -- flagging it here as the concrete next
manual attempt rather than a fresh blind permuter search.

**Verdict: PERMUTER-EXHAUSTED for the volatile/UB region of the search
space** (twelve candidates, all either `volatile`-typed or otherwise
non-idiomatic, none reaching 0) -- **but NOT exhausted overall**, because
the 135-score candidate above is genuine C that was found but not yet
manually verified. A future round should try that ONE specific reshape by
hand (three lines) before reaching for the permuter again.

## Round: verified the untried 135-candidate and a new ordering lever (runner delta, round 19) -- both REGRESS against the real oracle

Re-read `asm/nonmatchings/DreamSys/CalcDreamColor.s` directly (the same
"verify from raw asm, not from the report" discipline that found a real
misread bug in `func_8005A1F4` this round). No hidden semantic bug found
here -- the classification loop and the table lookup are exactly what this
report already describes. Two concrete follow-ups, both tested against the
real oracle (`./build-and-verify.sh` + `funcdiff.py`), not just reasoned
about:

1. **The BRAVO-flagged 135-score permuter candidate (`p` as scratch before
   assigning to `entry`) is a REGRESSION, not a lead.** Built it verbatim
   (renamed `entry`->`base` for local style only):
   ```c
   index = local.axis.dynamic * 3;
   p = &D_80087E14[index];
   base = p;
   return base[local.axis.upper];
   ```
   Result: **17/35, with a genuine 13-byte outside-range drift** (confirmed
   via `funcdiff.py`'s own warning) -- i.e. this candidate is not just a
   worse register match, it compiles to the WRONG SIZE. The permuter's
   "135 < 610" ordering does not imply "closer to a real match" once a
   candidate changes the instruction count; this is exactly the trap
   CLAUDE.md/DECOMPILATION_LEARNINGS.md warn about for permuter scores in
   general, now with a concrete instance for this function. Do not try this
   shape again without a different translation.
2. **Forcing the table-base pointer into its own statement BEFORE `index`
   is computed also regresses (23/35, 13-byte drift), even though retail's
   own disassembly computes the `lui`/`addiu` base address immediately
   after loading `dynamic` and BEFORE the `sll`/`addu` that forms
   `dynamic*3`:**
   ```c
   base = D_80087E14;
   index = local.axis.dynamic * 3;
   return base[index + local.axis.upper];
   ```
   Matching retail's OBSERVED instruction order by hand, statement-for-
   statement, is not sufficient here -- GCC 2.6.3 schedules the two
   differently depending on how the base and index are FUSED in the source
   expression, not just what order they're written in. The known-good
   28/35 form (`entry = &D_80087E14[index]; return entry[upper];`, base and
   index summed in one fused address-of expression rather than as two
   separate prior statements) remains the best C reached; this round did
   not find anything better.

**Verdict unchanged: STALL at 28/35, PERMUTER-EXHAUSTED for the legitimate
search space, and the one previously "untried" lead is now a confirmed
negative** (item 1 above) rather than an open thread. No source changes;
`INCLUDE_ASM` untouched throughout this round's testing.

### Proposed learning

A permuter candidate's SCORE ORDERING (135 better than 610) is not evidence
about the REAL oracle unless it is re-verified end to end, including
`funcdiff`'s outside-range drift check -- a candidate can score better on
the permuter's internal metric while being a different, WRONG size. This is
the same caution CLAUDE.md's "four ways a score lies" already states for
`funcdiff` itself; it applies with equal force to a permuter's own score,
and this function is a second confirmed instance (after `func_8005A1F4`'s
round-18 "50-point candidate" scare, which was structurally different
rather than size-wrong, but the same "read it back against the disassembly
before trusting it" discipline caught both).

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
Restored to `INCLUDE_ASM`. Permuter run added round 2026-09-02, runner
BRAVO; still `INCLUDE_ASM`.

## ROUND 20 (runner echo): confirmed as the commutative-add operand-order class -- SIXTH instance, THIRD unrelated unit

Per the coordinator's standing request to apply this screen to every
residue hit this round: re-read this function's residue against the raw
disassembly directly. Retail's tail:

```
lb   v1,0(sp)        ; dynamic
lui  a0,0x8008        ; base address (computed EARLY, right after dynamic)
addiu a0,a0,0x7e14
sll  v0,v1,1
addu v0,v0,v1          ; index = dynamic*3
lb   v1,1(sp)          ; upper (loaded LATE, only after index+base are ready)
addu v0,v0,a0           ; += base
addu v0,v0,v1            ; += upper
```

This build's compiled tail (same source, rebuilt fresh this round to
confirm rather than trusting the report's prior prose):

```
lb   v1,0(sp)        ; dynamic
lb   a0,1(sp)          ; upper (loaded EARLY instead -- right after dynamic)
sll  v0,v1,1
addu v0,v0,v1           ; index = dynamic*3
lui  v1,0x8008           ; base (computed LATE, reusing v1 once dynamic is dead)
addiu v1,v1,0x7e14
addu v0,v0,v1             ; += base   (retail: += a0 here)
addu v0,v0,a0              ; += upper  (retail: += v1 here)
```

Same two operand registers (`a0`, `v1`) feed the final two `addu`s in
both, just with `base`/`upper` holding the OPPOSITE register each time,
and the two `addu`s consequently swapped in which operand each consumes.

**Applied the coordinator's decisive test directly: reversed the C-level
operand order of the final addition** (`return *(local.axis.upper +
entry);` instead of `return entry[local.axis.upper];`, i.e. `upper +
entry` instead of `entry[upper]` -- an addition-order reversal, not just
a subscript-notation change) and rebuilt. **Byte-identical to the
un-reversed form: 28/35, same two words differing, same diff.** Both C
operand orders produce the identical wrong register/operand assignment
-- exactly the datum the coordinator used to confirm charlie's two
`class_3bb8c` instances as the same phenomenon.

**This makes CalcDreamColor a sixth confirmed instance of the class, and
the third unrelated unit** (after three in `code_179d8_c` and two in
`class_3bb8c`), independently found without knowing charlie's result in
advance -- this report's own residue description ("both addus end up
register-swapped relative to retail... reshapes... all four producing
the identical result") already matches the class's signature exactly; it
just wasn't cross-referenced against the class until this round. The
one nuance worth recording: unlike `func_80032BB8`'s instance (a single
`addu` with a genuine destination-register choice), here the swap
presents across TWO separate `addu`s and is entangled with an
independent load (`upper`) being scheduled early -- but the decisive
test (operand-order reversal producing an identical wrong result) is the
same, and that is the test the coordinator is treating as authoritative
for class membership, not the surface shape.

**Disposition unchanged: STALL at 28/35, PERMUTER-EXHAUSTED** (prior
rounds' 40400-iteration search already covers this exact residue).
`INCLUDE_ASM` restored; the operand-order-reversal test was reverted
immediately after confirming the identical result (`git diff --stat`
empty before continuing).

## ROUND 39 (runner echo): the combination corollary tested explicitly -- both levers, alone and combined, are inert

Per this round's flagship question (does the hoist-both-before-either lever,
or its combination with an already-tried lever, reach residues elsewhere
immune to each alone), re-verified fresh (28/35, byte-identical) and ran
two additional experiments on top of round 20's already-confirmed
operand-order-reversal negative:

1. **Hoist both bytes (`dynamic`, `upper`) into named locals, read before
   either is consumed**, i.e. `s8 dynamic = local.axis.dynamic; s8 upper =
   local.axis.upper;` before computing `index`/`entry` -- **byte-identical
   to the kept 28/35 form.** Confirms this residue is not the
   "value-computed-early-consumed-late" shape the lever targets: it is
   already a single fused index expression, not two independently
   producible values with divergent consumption timing.
2. **The SAME hoist combined with the operand-order reversal**
   (`return *(upper + entry);` instead of `entry[upper]`, on top of the
   hoisted locals) -- **also byte-identical.** The combination corollary
   (two individually-inert levers reaching a residue together, as with
   `func_8004C470`) does NOT hold here: both together produce the exact
   same wrong register assignment as either alone or as the baseline.

**This is a genuine, doubly-confirmed clean negative for both this round's
levers on a register-identity/commutative-add residue** (the sixth
confirmed instance of that project-wide class, per round 20) -- consistent
with the three other negatives reported this round on register-identity
ground elsewhere in the project. `INCLUDE_ASM` restored; whole-image SHA1
verified green; both experiments reverted immediately after measuring.

### Proposed learning (round 39)

The combination corollary (`func_8004C470`: two inert levers together
reaching 0) is real but not universal -- it depends on the two levers
actually touching independent DEGREES OF FREEDOM in the scheduler/allocator
decision. Here, hoisting the two byte reads and reversing the addition
operand order are both attempts to influence the SAME single fused
address computation, so combining them tests the same thing twice rather
than two different axes. Worth checking, before combining two levers, that
each targets a distinct choice the compiler makes -- otherwise the
combination is not a new experiment.
