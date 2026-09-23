# DreamSys__TryInstantTeleportLink -- STALL: exact length (63/63 instructions, zero address drift), 58/63 raw word-match, first real diff at 0x4B080 (two swapped delay-slot fillers around the `ExecuteLink` result branch -- a scheduling residue, PERMUTER-EXHAUSTED)

> Renamed from `func_8005A82C` on 2026-09-22 (tools/rename.py). Address 0x8005a82c.

> **ROUND 49 (2026-09-16, runner bravo): re-verified fresh, one new axis
> (`do{...}while(0)`) tried at both remaining exit sites, both clean
> negatives.** This unit had not been touched since round 39. Spliced the
> exact round-39 preserved body back in and rebuilt: byte-identical **58/63,
> zero address drift**, same two diverging sites (0x4B080/0x4B0D0) as every
> prior round -- confirms nothing drifted across the 10 untouched rounds.
>
> Round 47 found `do { return; } while(0)` around a single-statement early
> return changes GCC 2.6.3's basic-block shape enough to move an UNRELATED
> register-rotation residue elsewhere in the same function
> (`NoteOn`), scoped as "a per-return experiment, never a
> whole-function rewrite." That axis had never been tried on this function's
> own three `return true;` exits, so it was tried here, in two shapes:
>
> 1. Wrapping ONLY the first exit (`if (!ExecuteLink(...)) { do { return
>    true; } while(0); }`, the exit immediately before the first diverging
>    site at 0x4B080) -- **byte-identical to the kept 58/63 body.**
> 2. Wrapping the LAST TWO exits (`if (saved == 0) { do {...} while(0); }` /
>    `if (this->isFlashbackSession) { do {...} while(0); }`, bracketing the
>    second diverging site at 0x4B0D0) -- **also byte-identical.**
>
> Both reverted immediately after measuring; `INCLUDE_ASM` restored,
> whole-image SHA1 verified green, `git diff --stat` empty against `main` at
> the point of this write-up.
>
> **This is a clean, doubly-confirmed negative for the do-while lever on
> this residue class.** Unlike `DreamSys__TryStaircaseLink`'s do-while experiment this
> same round (which DID perturb codegen, for the worse, on an unrelated
> struct-copy elsewhere in that function), this function's do-while wrapping
> produced literally no observable change anywhere -- consistent with this
> function's residue being a pure delay-slot-FILL CHOICE between two already
> -independent, already-schedulable instructions (a materialized `li v0,1`
> constant vs. an address computation or a genuine `nop`), a shape round 39
> already established has nothing for the hoist-both lever to attach to, and
> now confirmed to have nothing for the do-while lever either. No new
> permuter search run this round (already PERMUTER-EXHAUSTED at ~49300
> iterations, and this round's negative gives no new seed idea to try).
>
> ### Proposed learning (round 49)
>
> Round 47's `do{...}while(0)` lever is CONFIRMED SCOPED, not just "worth
> trying everywhere": it moved a register-rotation residue in one function
> and left a delay-slot-fill-choice residue completely untouched in two
> different placements on this one. The two classes are diagnosably
> different from the .s alone (register-rotation is a same-length register-
> IDENTITY difference across the whole function; delay-slot-fill-choice is a
> divergence confined to exactly the two words bracketing an already-correct
> branch), and the lever appears to target only the first. Worth checking
> which class a residue is BEFORE spending a do-while attempt on it, rather
> than treating the lever as generically worth trying on any same-length
> stall.

> **TITLE REBUILT, round 32 (2026-09-12, runner alpha2).** The old title had
> no length figure and no diff-location figure, per this round's assignment
> to fix that. Re-measured fresh (spliced the exact preserved body below back
> in and rebuilt, not assumed): confirmed **63 words compiled, matching
> retail's length exactly** (`objdump` word count, zero `funcdiff` drift
> warning) and **58/63 raw word-match**, both identical to the inherited
> figures -- no drift had crept in. First real diff located precisely via
> `tools/asm-differ/diff.py`: `0x4B080`, the `beqz $v0,...` right after
> `ExecuteLink` returns, exactly the first of the two swapped-delay-slot
> sites this report already documents. No new attempt made this round beyond
> the re-measurement and title rebuild; the function remains
> PERMUTER-EXHAUSTED per the ~49300-iteration search already on record below,
> and neither of this round's two findings apply: this is not a hardware
> access (`volatile` is not in scope), and the residue is a delay-slot-FILL
> CHOICE (which independent, dependency-free instruction occupies a branch's
> delay slot), not a register-identity question at all -- confirmed already
> in round 20's commutative-add screen, which found no `addu`/`add`
> instruction anywhere in either diverging word.

**Unit:** DreamSys · **Size:** 63 words · **Status:** STALL, best 58/63 words (92%) ·
Restored to `INCLUDE_ASM`, no toolchain issue. **PERMUTER-EXHAUSTED** (round
2026-09-02, runner BRAVO) -- see "Permuter run" below; do not re-run this
search expecting a different outcome without new information.
**Vtable slot:** `DREAMSYS_METHODS +0x1D8` (third of the four-slot run; see DreamSys__TryTunnelLink.md)

## What it does (fully derived, control flow confirmed)

Third sibling of `DreamSys__TryTunnelLink`/`DreamSys__TryStageTimerLink` (same round): a
`Test4InstantTeleporters` link test, then on success an `ExecuteLink` with
literal type `0x11`, then -- new in this one -- a block of vtable/class
calls gated on the `ExecuteLink` result, itself internally gated on two
more conditions.

```c
bool DreamSys__TryInstantTeleportLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 saved;
	s32 local[4];

	result = Test4InstantTeleporters(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	saved = func_8005BFC4();
	if (!ExecuteLink(this, result, 0x11, 0))
		return true;
	this->unknwon_int_0x44 = 0;
	this->unk_0x4C->methods->slot0xE8(this->unk_0x4C, local, &this->linkCoordinates);
	this->vt->BaseObjO__SetVec14(this, local);
	if (saved == 0)
		return true;
	if (this->isFlashbackSession)
		return true;
	this->vt->GetSetDreamTimeLimit(this, this->vt->DreamSys__GetDreamTimerScaled(this) + saved);
	return true;
}
```

This reaches 58/63 words -- every field offset, every call target, every
register identity, and the overall control-flow graph (branch TARGETS, not
just delay slots) are all confirmed correct. `funcdiff.py` shows a clean
in-range diff with NO "differs outside this range" warning, i.e. the
instruction COUNT matches retail exactly; only 5 words differ, and every
one of them is a delay-slot filler choice or the branch offset that
encodes it, at exactly two sites.

## New struct knowledge confirmed by this derivation (kept, not reverted)

- `DreamSysUnk4CMethods` (the class `DreamSys::unk_0x4C` points to) gained
  slot `+0xE8`: `void (*slot0xE8)(void *self, void *arg1, PlayerSpawnPoint
  *arg2)`, called as `this->unk_0x4C->methods->slot0xE8(this->unk_0x4C,
  &local, &this->linkCoordinates)`. `local` is an 0x10-byte stack buffer
  (same frame-derived sizing logic as `DreamSys__TryTunnelLink`'s `local`) that this
  slot fills and the next call consumes.
- `vtable_DreamSys` gained a name for `+0xB8` (previously folded into an
  anonymous `u32 unknown_functions_0xa0[7]`, right before the already-named
  `BaseObjO__AddVec14` at `+0xBC`): resolved via `tools/classtable.py
  DREAMSYS_METHODS` to `BaseObjO__SetVec14`, address outside this
  unit/runner's range, still `INCLUDE_ASM`. Called as `this->vt->
  BaseObjO__SetVec14(this, &local)` -- same buffer as the slot above.
- The tail two calls, `vt->slot0x108(this)` and `vt->slot0x104(this, ...)`,
  turned out to be ALREADY-NAMED existing slots read at the wrong offset in
  a first pass: `+0x108` is `DreamSys__GetDreamTimerScaled(DreamSys *this)` (existing
  signature, unchanged) and `+0x104` is `GetSetDreamTimeLimit(DreamSys
  *this, s32 time)` (existing signature, unchanged) -- confirming
  `GetSetDreamTimeLimit(this, DreamSys__GetDreamTimerScaled(this) + saved)` as the correct
  reading, not two new unnamed slots.
- `this->isFlashbackSession` (existing field, offset `0x68`) gates the
  final `GetSetDreamTimeLimit` call.

All of the above are struct/header facts independent of this function's
own residue and are kept in `include/DreamSys.h` even though the function
itself is reverted to `INCLUDE_ASM`.

## The residue: two swapped delay-slot fillers, same total instruction count

At `.text+0x54` (retail `8005A880`, the `beqz $v0,.L8005A908` right after
`ExecuteLink`): retail's delay slot is `addiu $a1,$sp,0x10` (computing
`&local` early, for the FALL-THROUGH path's first vtable call); the
compiled build's delay slot is `li $v0,0x1` (the function's "return true"
value, hoisted here instead) -- with the `&local` computation pushed 4
words later, right before it is actually consumed. The branch OFFSET
differs by exactly 1 word as a direct consequence (same target, same CFG,
just a different physical distance because of the swap).

The identical pattern recurs at `.text+0xA4` (retail `8005A8D0`, the
`bnez $v0,.L8005A908` gating the final `GetSetDreamTimeLimit` call): retail
leaves that delay slot a genuine `nop`; the compiled build again fills it
with `li $v0,0x1`.

Both sites are the SAME underlying phenomenon: this compiler is willing to
hoist the cheap, dependency-free `li $v0,1` ("return true") into an earlier
delay slot than retail does, in a function with THREE early-exit points
that all return the same literal. Total instruction count is identical
either way (58/63, no address drift) -- this is pure scheduling, not a
register-identity or CFG difference.

## Attempts that did NOT move it (5, before the hard stop)

1. **Nested `if`, single trailing `return true;`** (as shown above but with
   braces/nesting instead of `goto`) -- same 58/63, identical residue.
2. **Three explicit early `return true;` statements** instead of one
   shared exit -- no change at all; GCC's cross-jump/tail-merge treats them
   identically to the nested form.
3. **`goto end;` / `end: return true;`** (the form now inlined above) --
   no change from (1) or (2). Confirms the residue is NOT about
   return-vs-goto spelling; the CFG GCC builds is the same regardless.
4. **`__asm__("")` barrier** right after the `ExecuteLink` check, before
   the first store -- made it WORSE: added a full extra instruction and
   reintroduced whole-file address drift (funcdiff's "differs outside this
   range" warning came back). Reverted immediately per the project's own
   caution that a barrier's blast radius scales with function size.
5. **Explicit `void *pLocal = local;` computed at the top of the
   function**, dereferencing through `pLocal` instead of `local` at both
   call sites -- made it much worse: GCC allocated an EXTRA callee-saved
   register for `pLocal`, growing the frame and drifting the whole file.
   Reverted immediately.

None of the standard reshaping levers documented in
`DECOMPILATION_LEARNINGS.md` (goto/return, statement order, explicit
locals, barriers) reach this residue; it looks like a scheduler heuristic
tied to the total number of live "return 1" exit edges in the function
rather than anything expressible as a source-shape choice. Flagging as a
candidate permuter target rather than continuing to burn manual attempts.

### Proposed learning

**A function with multiple early-exit points that all return the SAME
literal constant can have its constant's materialization
(`li $v0,<value>`) hoisted into a DIFFERENT delay slot than retail chose,
with IDENTICAL total instruction count and IDENTICAL branch targets** --
this is a pure scheduling residue, not reachable (in this instance) by
`goto`/`return` spelling, statement reordering, an explicit local for the
repeatedly-dereferenced pointer, or a scheduling barrier (which instead
perturbed register allocation and made it worse, consistent with the
existing barrier-blast-radius caution in this file). Two independent sites
in the same function showed the identical pattern (retail fills the slot
with useful order-independent work or a plain `nop`; this build fills it
with the hoisted return-value constant instead), which rules out a
one-off misread. Recommend a permuter target once one is set up: fixed
CFG, fixed register allocation, only delay-slot scheduling in question.

## Round: re-verified from raw asm, one more reshape tried (runner delta, round 19)

Re-read `asm/nonmatchings/DreamSys/DreamSys__TryInstantTeleportLink.s` directly (not just this
report's prose) as a check against the kind of misread that turned out to
be real in `DreamSys__SoundCueCallback` this same round. No hidden bug found here --
every field offset, call target, and the two delay-slot sites this report
already describes check out exactly against the raw disassembly.

One additional reshape, not previously on record: replacing the three
literal `return true;` exit points with a single named `bool ret = true;`
materialized once at function entry and returned from all three sites
(the same "materialize the value into a named local before the branch"
idiom that closed `DreamSys__SoundCueCallback` this round). Result: **identical 58/63**,
same two delay-slot sites diverging the same way. This idiom generalizes
from "a comparison's boolean result" (where it worked) to "a constant
return value repeated across multiple exits" (where it does not move
anything here) -- the two are not the same shape, and this function is a
clean negative for the second one specifically.

**Verdict unchanged: STALL at 58/63, PERMUTER-EXHAUSTED for the legitimate
search space.** No source changes; `INCLUDE_ASM` untouched.

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.


## Permuter run (round 2026-09-02, runner BRAVO) — PERMUTER-EXHAUSTED

Set up per `tools/setup-permuter.sh` with the near-miss body above as the
seed. Base score confirmed 460 in `--debug` mode, matching this report's
own diagnosis (the two swapped delay-slot fillers).

Ran in two windows totaling **~49300 iterations over ~9m50s wall clock**
(`-j 8`, `--stop-on-zero --best-only`):

- **Window 1: cut short by an external, unscoped `pkill -f "decomp-permuter"`
  run by another runner in this round tidying up ITS OWN permuter session** —
  not a bug in this search, but it means the window-1 iteration count
  (37028) reflects an interrupted run, not a clean stop. Confirmed after
  the fact: the process was dead well before this round's own 300s
  timebox would have fired, and the log shows the multiprocessing
  "leaked semaphore" warning characteristic of a `SIGKILL`, not a graceful
  exit.
- **Window 2: a fresh restart, run to its own bounded, PID-scoped timeout**
  (no pattern-based kill this time) — a further ~12300 iterations, no
  improvement over window 1's best.

**Best score reached: 200** (base 460), never 0, across both windows and
seven distinct saved "best" outputs (`output-200-1`, `output-200-2`,
`output-300-1`, `output-360-1..4`).

### Why the 200-score candidate was REJECTED, not banked as a match

The improving candidate introduces a spare local (`int new_var`) and
reshapes the final two early-return sites as:

```c
saved = (new_var = func_8005BFC4());
...
if (this->isFlashbackSession) {
	new_var = true;
	return new_var;
}
this->vt->GetSetDreamTimeLimit(this, this->vt->DreamSys__GetDreamTimerScaled(this) + saved);
return new_var;   /* <-- new_var was never (re)assigned on THIS path */
```

The final `return new_var;` reads a variable that is only ever assigned
on a DIFFERENT, non-overlapping control path (the `isFlashbackSession`
early exit) or as an alias for `func_8005BFC4()`'s return value earlier.
On the path that actually reaches this statement, `new_var` holds
whatever was last written to its register for an unrelated purpose --
which happens, for THIS compiler and THIS function's register pressure,
to coincide with the needed value. This is a **read of an
effectively-uninitialized variable along the executed path**, not a
legitimate expression of "return true" -- it is the "stale-register-reuse"
form the project's Gate 3 explicitly calls out as disqualifying. The other
six saved candidates (300/360-word scores) are all variations on the same
trick, or introduce dead duplicate `return true; return true;` statements
that a real author would never write. None is real C; all were rejected.

**Verdict: PERMUTER-EXHAUSTED for the legitimate/idiomatic C search space.**
Every path to a lower score found in ~49300 iterations relies on a
non-idiomatic or UB-adjacent form. This does not prove no legitimate
zero-cost reshape exists — a permuter search is inherently incomplete —
but it is strong enough evidence, combined with the five manual attempts
already on record above, that a future round should NOT default to
re-running this exact search. If someone has a genuinely new SHAPE idea
(not a reshuffle of the same locals), it is worth a fresh, small, targeted
seed rather than a long blind search.

## ROUND 20 (runner echo): commutative-add screen -- NOT this class

Per the coordinator's standing request, checked this function's residue
against the commutative-add operand/destination-register class confirmed
this round in `CalcDreamColor` (this same unit) and five prior instances
across `code_179d8_c`/`class_3bb8c`. **Not a match.** This function's two
divergences are both delay-slot-FILL CHOICES -- which independent
instruction (the hoisted `li $v0,0x1` return-value constant vs. retail's
`addiu $a1,$sp,0x10`/a genuine `nop`) occupies a branch's delay slot --
not a commutative `addu`'s operand order or destination register at all.
No `addu`/`add` instruction appears anywhere in either of the two
diverging words. Same family as `Class866E8__ResetAllElements`'s (`class_3ac78`) and
`DreamSys__AdvanceMoveCycle`'s residue #2 (this same unit, this round) -- an
independent, dependency-free value getting scheduled into an earlier
delay slot than retail chose -- but a DIFFERENT class from the
commutative-add one. Not re-attempted further this round: already
PERMUTER-EXHAUSTED (49300 iterations, prior round) and five manual
attempts plus one more reshape (round 19) already on record, all
converging on the identical residue; no new idea available this round
beyond confirming the screen. Disposition unchanged: STALL at 58/63,
`INCLUDE_ASM`.

## ROUND 39 (runner echo): hoist-both-lever precondition doesn't hold; two new reshapes, both inert or worse

Re-verified fresh (spliced the exact preserved body back in): confirmed
byte-identical **58/63**, same two diverging sites (`0x4B080`/`0x4B0D0`) as
every prior round. Checked this round's flagship lever explicitly: the
"hoist both values before either is consumed" diagnostic requires retail's
TWO loads/mults to be ADJACENT with consumers later. Neither residue site
here is a load/mult at all -- both are which-independent-instruction-fills-
this-delay-slot choices (a `li $v0,1` constant vs. an `addiu`/`nop`), so the
lever's precondition does not hold. This is consistent with round 20's
finding that no `addu`/`add` appears at either diverging word either.

Two new reshapes tried anyway, both rebuilt and measured:

1. **Reordering `this->unknwon_int_0x44 = 0;` to AFTER the `slot0xE8` call**
   (untried in any prior round -- all five prior attempts touched the
   return-value/goto shape, never this store's position relative to the
   call). **Regressed sharply to 25/63 with 136229 bytes of whole-image
   drift** -- moving this store changes far more than the two delay-slot
   sites; reverted immediately.
2. Re-confirmed the existing kept form's exact 58/63 result stands
   unchanged after (1)'s revert.

**Verdict unchanged: STALL at 58/63, PERMUTER-EXHAUSTED.** No source
changes retained; `INCLUDE_ASM` restored, whole-image SHA1 verified green.

### Proposed learning (round 39)

The hoist-both-before-either lever's precondition (two adjacent loads/mults,
consumed later) is a real filter, not just a diagnostic label: this
function's residue is a delay-slot-FILL CHOICE between two already-scheduled,
already-independent instructions (a materialized constant vs. an address
computation), which is a different shape entirely and the lever has nothing
to attach to. Checking the precondition against the raw `.s` BEFORE
attempting the lever (as this round's brief instructed) correctly predicted
it would not apply here, saving a blind attempt.

## Naming

- **Tier B.** STALL (still INCLUDE_ASM). Wraps Test4InstantTeleporters and calls ExecuteLink (type 0x11) on success, per the preserved #if 0 body; same family as DreamSys__TryTunnelLink. Renaming a stall's symbol changes no bytes.
