# DreamSys__AdvanceMoveCycle -- MATCHED, 79/79 words, round 32 (2026-09-12, runner alpha2)

> Renamed from `func_80059BE0` on 2026-09-22 (tools/rename.py). Address 0x80059be0.

**This function now matches retail byte-for-byte.** Before touching anything,
this round re-measured fresh per the function's own standing instruction
("measure it yourself and rebuild the title unambiguously"): spliced the
preserved body back in verbatim, confirmed **80 words compiled vs retail's
79 (1 word LONG, not short — the round-20/25 correction was right)**, 19/79
raw in-range word-match, first real diff at `0x4A438` (the `bne
$unk_0xAC,0x4` branch's own delay slot, where retail computes `bit = count &
1` unconditionally and this build left a `nop`, instead recompiling the
boolean as a differently-shaped 2-instruction sequence later).

**One inherited-body drift caught in passing**: the report's own "Best-reached
C" section (below) shows a named `s32 bit = count & 1;` local, but the body
actually preserved in `src/DreamSys.c` had that inlined away
(`doCallback = ((count & 1) == 0);`, no `bit` local at all) — a genuine
divergence between the report's prose and what was actually banked. Rebuilt
BOTH forms fresh to check which was really best: the `bit`-local form scores
**14/79** (worse), confirming the inline form (already in `src/`) was
correctly the better of the two — but the report's own listed "best-reached
C" was not what was actually compiled. Read any report's own code block as a
claim to verify, not a transcription to trust, same lesson this file already
carries elsewhere for word-count claims.

**The fix, tried after the above verification**: collapse the nested
`if (this->unk_0xAC == 4) doCallback = ((count & 1) == 0);` into a single
short-circuit boolean expression:

```c
doCallback = (this->unk_0xAC == 4) && ((count & 1) == 0);
```

**Result: 79/79, whole-image SHA1 green.** This is the same family as
`CalcDreamColor`'s and other units' "flatten a nested if into one boolean
expression" idiom, applied here to a case none of the prior 20+ attempts
tried — every earlier attempt (named `bit` local, inlined ternary-like form,
a scheduling barrier at the branch) kept the NESTED `if`/assignment shape and
varied only the sub-expression inside it. Flattening the two conditions into
one `&&` expression let GCC compute `unk_0xAC == 4`'s test and the `count &
1` term in the SAME instruction ordering retail's compiler chose — where
before, the extra level of C-level nesting was itself forcing a different
lowering of the boolean.

### Proposed learning

**A same-length register/scheduling residue confined to a small boolean
computed inside a nested `if` can be closed by flattening the nesting into
one short-circuit `&&` expression, even after a named intermediate local, an
inlined ternary, and a scheduling barrier have all failed on the SAME
sub-expression.** The lesson generalizes past `CalcDreamColor`'s "split a
combined index expression" finding: here the axis that mattered was not the
arithmetic's grouping but the CONTROL-FLOW nesting depth the boolean was
computed under. Worth trying before filing a same-instruction-count boolean
residue as a scheduling-barrier-exhausted stall — flattening nested guards
into `&&`/`||` is a cheap, previously-undocumented axis distinct from
everything else already tried on this residue class.

## Final C (matches retail exactly)

```c
s32 DreamSys__AdvanceMoveCycle(DreamSys *this, s32 arg1)
{
	s32 doCallback = 0;
	s32 ret = 0;
	s32 count;
	DreamSysUnk5C *p;
	s32 delta;

	if (this->unk_0xA0 != 0) {
		ret = this->unk_0xA0;
		count = this->unk_0xB4 + 1;
		this->unk_0xB4 = count;
		if (count < 4) {
			doCallback = (this->unk_0xAC == 4) && ((count & 1) == 0);
		} else {
			this->unk_0xA0 = 0;
			doCallback = 1;
		}

		if (doCallback)
			this->vt->DreamSys__StartVoice(this);

		p = this->unk_0x5C;
		if (p != NULL && this->screenShakeOn != 0 && arg1 != 0) {
			delta = -50;
			if (this->unk_0xB4 >= 3)
				delta = 50;
			p->unk_0x18 += delta;
			p->unk_0x24 += delta;
		}

		if (this->unk_0xA0 == 0)
			this->unk_0xB4 = 0;
	}

	if (!doCallback)
		this->vt->DreamSys__StopVoice(this);
	return ret;
}
```

---

## History (pre-round-32, kept verbatim)

> **TITLE WAS STALE.** The original title line ("PLUS a real 2-word drift")
> predates round 20, which narrowed the drift to 1 word and fully closed
> the separate tail-merge defect that caused the second word. Round 25
> reproduced round 20's exact state and tried two NEW levers (a goto-based
> early return, and a scheduling barrier at the delay-slot-fill site);
> neither improved on round 20's 1-word-short/19-in-range result. Read
> "ROUND 20" and "ROUND 25" below; the sections in between predate both and
> describe the (larger, now-closed) 2-word state.

**Unit:** DreamSys · **Size:** 93 instructions (79 words in scope, `0x4A3E0`-
`0x4A51C`) · **Status:** STALL, best score **45/79 words**. **CORRECTION
(round 19): the build is NOT drift-free** -- it compiles to 81 words, 2
LONGER than retail's 79, confirmed both via `funcdiff.py`'s own
outside-range warning and directly via `objdump` word-count. The original
"clean (drift-free) build" claim just below was wrong; read the round-19
section near the end of this file before trusting the 45/79 in-range
number. Round 2026-09-02, runner BRAVO (original filing).

## What it does

Vtable slot `+0x164`. Called by `DreamSys__TickMoveFree`/`DreamSys__TickMoveForced` as
`DreamSys__AdvanceMoveCycle(this, 0)` or `DreamSys__AdvanceMoveCycle(this, 1)`. It:

1. Returns 0 immediately if `this->unk_0xA0 == 0` (no attempt/beat cycle in
   progress), still running the "else" tail call below first.
2. Otherwise saves `this->unk_0xA0` as the return value, increments and
   stores `this->unk_0xB4` (an attempt counter), and:
   - if the counter reaches 4, clears `this->unk_0xA0` and flags a callback;
   - else if `this->unk_0xAC == 4`, flags the callback when the counter is
     even.
3. If the callback flag is set, calls `this->vt->DreamSys__StartVoice(this)` (vtable
   slot `+0x168`, itself still `INCLUDE_ASM`/blocked — see that function's
   own stall report).
4. If `this->unk_0x5C` is non-NULL, `this->screenShakeOn` is nonzero, and
   `arg1` is nonzero: computes `delta = (this->unk_0xB4 < 3) ? -50 : 50` and
   nudges both `this->unk_0x5C->unk_0x18` and `->unk_0x24` (the y-components
   of the two `(x,y)` points documented on `DreamSysUnk5C`) by `delta`.
5. If `this->unk_0xA0` is now 0, resets `this->unk_0xB4` to 0.
6. If the callback flag was NOT set, calls `this->vt->DreamSys__StopVoice(this)`
   (slot `+0x16C`, already matched separately in this unit) instead of the
   slot-`0x168` call.
7. Returns the value saved in step 2 (or 0, from step 1).

## New struct knowledge (kept; header changes are NOT part of the stall)

Confirmed by this round and left in `include/DreamSys.h` even though the
function itself did not close:

- `DreamSysUnk5C::unk_0x18` — the first point's y (paired with the already-
  known `unk_0x24`, the second point's y). Both are nudged by the SAME delta
  in the SAME statement pair, which is what pinned the offset down.
- `DreamSys::unk_0xB4` (was `unknown_values_0xB4[8]`) — the attempt/beat
  counter described above, bounded to `[0,4)` while `unk_0xA0` is nonzero.
- `vtable_DreamSys::DreamSys__StartVoice` (was `unknown_functions_0x168[1]`) — named
  from `tools/classtable.py DREAMSYS_METHODS` (`+0x168`); the symbol already
  existed as an `INCLUDE_ASM` entry in `src/DreamSys.c`, just not yet wired
  into the vtable struct.
- `DreamSys::unk_0x678` is exactly `DreamSys::screenShakeOn` — confirmed by
  offset arithmetic (`unknown_values_0x604[116]` runs `0x604`-`0x677`,
  `screenShakeOn` starts at `0x678`) and by `bool` being `typedef int bool`
  (`include/types.h:27`), so the word-sized `lw`/`bnez` test in the
  disassembly is exactly a `bool` read. No header edit was needed for this
  one; it already existed under that name and this round just confirmed the
  call site uses it.

Confirmed the two blocker screens are clean for this function: no `gp_rel`
hit and no `addiu $at, $at, %lo` hit anywhere in `DreamSys__AdvanceMoveCycle.s`.

## Best-reached body (45/79 words, clean/drift-free build)

Two whole sub-blocks of the function compile BYTE-IDENTICAL to retail: the
`DreamSys__StartVoice`/`DreamSys__StopVoice` dispatch pair (the `if (doCallback) ... else
...` call shape, `0x4A44C`-`0x4A468` and the `unk_0x5C`/`screenShakeOn`/
`delta` block (`0x4A46C`-`0x4A4C4`) match exactly, including the `delta =
-50; if (this->unk_0xB4 >= 3) delta = 50;` idiom (an ordinary ternary
`(cond)?-50:50` does NOT reproduce retail's branch polarity/instruction
order here — this "unconditional-then-overwrite" spelling does) and the
`p = this->unk_0x5C;` single-load-reused-twice pattern (writing
`this->unk_0x5C->fieldA += d; this->unk_0x5C->fieldB += d;` as two direct
member accesses instead reloads the pointer a second time after the first
store, because GCC 2.6.3 has no alias proof that the intervening store
didn't change `this->unk_0x5C` itself).

What does NOT close, despite several structurally-different rewrites, all
of which reproduce the same two residues:

1. **Prologue callee-save STORE ORDER + the unconditional `doCallback = 0`
   init.** Retail's order is `sw s1 / li s1,0 / sw ra / sw s3`, with `s1=0`
   unconditional at function entry and `s3=0` (this function's `ret=0`, the
   `unk_0xA0==0` early-exit value) living in the entry branch's delay slot.
   Every C shape tried instead produces `sw ra / sw s3 / sw s1`, with
   `s1=0` occupying that delay slot and `s3=0` relegated to the branch
   target. This is exactly the class CLAUDE.md/DECOMPILATION_LEARNINGS.md
   already document ("Prologue callee-save store ORDER is not reachable
   from C... a bare `__asm__("")` as the function's FIRST statement is the
   lever") — and the lever was tried (see below) but did not generalise
   here the way it did for `func_80025D10`.
2. **`count`'s hardware register.** Retail keeps the incremented
   `this->unk_0xB4 + 1` value in `$a0` (freed up again right before the
   `DreamSys__StartVoice` call, which needs `$a0` for `this`); every rewrite tried
   here (a named `count` local, an inlined `this->unk_0xB4++` with no local
   at all, a hoisted-vs-nested `bit` sub-expression) instead allocates
   `$v1`. This drags along a second residue: retail computes `andi
   v0,a0,0x1` UNCONDITIONALLY, filling the `bne $unk_0xAC,4` branch's own
   delay slot (dead on the not-taken path), then `sltiu s1,v0,1` only on the
   taken/equal path. The register-allocation choice and the delay-slot
   choice move together in every attempt; never got both to align with
   retail simultaneously.

### `__asm__("")` experiment (does not generalise here)

Adding a bare `__asm__("");` as the function's first statement (after the
prologue's `doCallback = 0`) DID fix the callee-save order (all four `sw`
instructions plus the `li s1,1` position matched byte-for-byte). But it also
let GCC recognise that `this->unk_0xA0` (already loaded into `$v1` for the
entry test) is unconditionally correct for BOTH the true branch (`ret =
this->unk_0xA0`) and the false branch (`ret = 0`, since `$v1==0` exactly
when the branch is taken) — so it collapsed retail's two-instruction
`li s3,0` (delay slot) + `move s3,v1` (overwrite) into a single `move
s3,v1` in the delay slot, which is ONE INSTRUCTION SHORTER than retail and
therefore shifts every address after it (drift: outside-range diff jumped
from the drift-free baseline 91419 bytes to 147015 bytes). Score with the
barrier: 12/79 (and address-shifted, i.e. untrustworthy even at that).
Moving the barrier to other points (right after `ret = this->unk_0xA0;`,
combined with removing the `else` arm and writing `ret = 0;` unconditionally
first to mimic the `delta = -50; if (...) delta = 50;` idiom that worked for
`delta`) reproduced the same or worse drift (score as low as 5/79, still
drifted). The barrier is a real, banked lever for the ORDER problem alone,
but here it interacts with a DIFFERENT optimization (the `ret`
merge) that retail's compiler evidently did not take, and no placement
tried avoided triggering it while still filling the delay slot the way
retail does.

Per CLAUDE.md rule 6, neither residue is a candidate for
`register T v asm("$N")` or an operand constraint — both are register-
IDENTITY differences (WHICH register is chosen), explicitly the banned
class, not an instruction-ORDER difference a barrier is permitted to fix.

## Best-reached C (inlined for the next attempt; NOT compiled into `src/`)

```c
#if 0
s32 DreamSys__AdvanceMoveCycle(DreamSys *this, s32 arg1)
{
	s32 doCallback = 0;
	s32 ret;
	s32 delta;

	if (this->unk_0xA0 != 0) {
		s32 count;
		DreamSysUnk5C *p;

		ret = this->unk_0xA0;
		count = this->unk_0xB4 + 1;
		this->unk_0xB4 = count;
		if (count < 4) {
			s32 bit = count & 1;

			if (this->unk_0xAC == 4)
				doCallback = (bit == 0);
		} else {
			this->unk_0xA0 = 0;
			doCallback = 1;
		}

		if (doCallback)
			this->vt->DreamSys__StartVoice(this);

		p = this->unk_0x5C;
		if (p != NULL && this->screenShakeOn != 0 && arg1 != 0) {
			delta = -50;
			if (this->unk_0xB4 >= 3)
				delta = 50;
			p->unk_0x18 += delta;
			p->unk_0x24 += delta;
		}

		if (this->unk_0xA0 == 0)
			this->unk_0xB4 = 0;
	} else {
		ret = 0;
	}

	if (!doCallback)
		this->vt->DreamSys__StopVoice(this);
	return ret;
}
#endif
```

## For the next attempt

- Do not re-try plain declaration-order or nesting permutations of `count`/
  `bit`/`p` — six+ variants tried (see above), none moved the `count`
  register off `$v1`.
- The `__asm__("")` barrier IS worth re-trying, but only in combination with
  a `ret`-handling shape that does NOT give the compiler an opportunity to
  notice `$v1 == 0` on the early-exit path (i.e. something that keeps `ret`'s
  two assignments provably distinct to GCC 2.6.3's very limited alias/value
  analysis). Whatever that shape is, it wasn't found this round.
- The `DreamSys__StartVoice`/`unk_0x5C`/`delta` two-thirds of the function are
  solid — reuse this report's C for those blocks verbatim; they are not
  where the remaining risk is.

## Proposed learning

The `delta = -50; if (cond) delta = 50;` idiom (unconditional-then-overwrite,
NOT a ternary) generalises the existing "surplus value" reading in
DECOMPILATION_LEARNINGS.md to a value materialised in a branch's delay slot
that is later conditionally overwritten by the fallthrough — not just to
redundant `move`s. Confirmed byte-exact here for a `+50`/`-50` pick; worth
trying whenever a two-constant selection differs from a ternary's compiled
form only in operand/branch order.

## Round: hand analysis (runner delta), asm reading confirmed, no corrections found

Re-read `asm/nonmatchings/DreamSys/DreamSys__AdvanceMoveCycle.s` directly against this
report's two documented residues. Both check out exactly as described --
no transcription errors found (unlike `func_8002B3F4` this same round,
where the preserved C had a bare-array-vs-dereference bug): the prologue
really is `sw s0 / addu s0,a0,zero / sw s2 / addu s2,a1,zero / sw s1 /
addu s1,zero,zero / sw ra / sw s3`, with the entry-branch's delay slot
really is a standalone `addu s3,zero,zero` (unconditional `ret=0`)
immediately overwritten by `addu s3,v1,zero` on the fallthrough path --
two separate instructions, not merged, matching the report's `__asm__("")`
finding precisely. And `count` (`this->unk_0xB4 + 1`) really does live in
`$a0`, confirmed at `addiu $a0,$v0,0x1` immediately followed by
`slti $v0,$a0,0x4`. No new lead found by re-reading; the report's own
"for the next attempt" guidance stands. Seed already validated
(`--debug` base 640: 8 register differences, 4 insertions, 2 deletions,
0 reorderings/stack/branch). Queued behind `DreamSys__SoundCueCallback`, `func_8002B3F4`
and `func_80064E34` for a search slot. No source changes; `INCLUDE_ASM`
untouched.

## Round: coordinator-directed type-axis test (runner delta) -- negative, evidence attached

Directly tested the type/width lever against this function's own
disassembly (not re-derivation -- checked every relevant opcode).
Every field this function touches loads/stores at full word width:

```
lw   $v1, 0xA0($s0)     # unk_0xA0
lw/sw     0xB4($s0)     # unk_0xB4
lw   $v1, 0xAC($s0)     # unk_0xAC
lw   $v0, 0x678($s0)    # screenShakeOn
lw/sw     0x18($a1)     # DreamSysUnk5C::unk_0x18
lw/sw     0x24($a1)     # DreamSysUnk5C::unk_0x24
```

No `lb`/`lbu`/`lh`/`lhu`/`sb`/`sh` appears anywhere touching these six
fields -- a narrower declared type for any of them would change the
opcode itself, and none of the opcodes leave room for that. Rules out
the type/width axis end-to-end for both of this report's residues
(prologue store order and the `count` register). Verdict unchanged:
genuine register-identity / scheduling stall, now tested against
disassembly rather than assumed from the earlier read. No source
changes.

## Round: found a THIRD, previously undocumented residue -- a real 2-word drift (runner delta, round 19)

Rebuilt this report's exact preserved 45/79 body into `src/DreamSys.c`
directly (not just re-read) to verify the "clean (drift-free) build" claim,
per this round's "verify claims, do not inherit them" brief. **The claim is
wrong.** `funcdiff.py` prints the usual outside-range drift warning
(91419 bytes), and `objdump -d build/src/DreamSys.c.o` confirms the compiled
body is genuinely **81 words, 2 longer than retail's 79** -- not a
`funcdiff`-window artifact (unlike a similar-looking warning checked this
same round on `CalcDreamColor`, which turned out to be a false positive;
this one is real, confirmed by the next function's start address shifting
by 8 bytes in the object file).

**The extra 2 words are at the tail merge, not at either previously
documented residue.** `asm-differ` pinpoints it precisely:

```
retail:  bnez v0,.L80059CD8 / nop / sw zero,0xB4(s0) / .L80059CD8: bnez s1,...
mine:    bnez v0,X          / nop / j Y / sw zero,0xB4(s0) / Y: move s3,zero / bnez s1,...
```

Retail's `.L80059CD8` is ONE label reached from TWO places: the function
entry's `unk_0xA0==0` skip (which sets `ret=0` via a `move` in that
branch's own delay slot) AND the fallthrough after the inner
`if (unk_0xA0==0) unk_0xB4=0;` reset (which needs no `ret` assignment at
all, since `ret` was already set earlier in the same block). My compiled
body instead places the outer `else { ret = 0; }` block's code as a
physically separate block reached via its own extra `j`, because the
report's C writes the big `if (unk_0xA0 != 0) {...}` block FIRST and the
`else { ret = 0; }` SECOND in source order -- the inner block's own tail
then has to jump forward, over the `else` block's code, to reach the
shared "if (!doCallback)" statement.

**Two restructurings tried to force the single shared merge point retail's
disassembly shows, both regressed sharply (14/79, same 2-word drift plus
much more register damage):**

1. **`goto`-based early return**, matching retail's literal CFG
   (`if (unk_0xA0==0) { ret=0; goto tail; } ... tail: if (!doCallback) ...`),
   with `count`/`p` hoisted to function scope (required for the `goto` to
   skip cleanly past their block in C89). Made it dramatically worse.
2. **Swapping the `if`/`else` textual order** (`if (unk_0xA0==0) ret=0;
   else { big block }`, no `goto`) -- also made it dramatically worse, by
   the same amount as (1). This rules out "which block comes first in
   source" as an independently tunable axis; both attempts to make the
   physical code layout match retail's actual shared-label structure
   instead broke the register allocation that the ORIGINAL, worse-looking
   layout was getting right.

**Left as-is: the original if-block-then-else form remains the best C
reached (45/79, 81 real words vs retail's 79), despite the confirmed
2-word drift.** Do not spend further attempts trying to force the literal
shared-label CFG shape by hand; both direct translations of it regress. A
permuter run seeded on this exact body (`tools/setup-permuter.sh
DreamSys__AdvanceMoveCycle <this body>`) is the next reasonable step, and should now
also close the tail-merge residue in addition to the two previously known
ones -- it was not tried this round due to time budget, and the seed used
for a PRIOR round's queued-but-not-yet-run permuter mention (`func_8002B3F4`,
`func_80064E34`) never actually included this residue since it wasn't known
to exist yet.

### Proposed learning

**A `funcdiff` "differs outside this range" warning is not always a false
positive, and is not always attributable to the residue(s) already
documented for a function.** This report carried a "clean (drift-free)
build" claim for two rounds; direct re-verification (rebuilding the exact
preserved C, not just re-reading the report) found a genuine 2-word
overshoot at a THIRD site neither prior residue mentions. **Always rebuild
a "verify this stall" task's exact preserved body and re-run the real
oracle before trusting a report's drift claim** -- reading the asm alone
(as an earlier round-19 pass on this same function did, "no corrections
found") is not sufficient to catch a drift claim that is simply wrong,
because the asm read confirms the RESIDUES described, not the absence of
OTHER residues the report doesn't mention.

Separately: a source-level "obviously more literal" translation of
retail's actual shared-branch-target CFG (via `goto` or via reordered
`if`/`else`) is not guaranteed to move a function closer to retail --
here it moved it sharply further away, on TWO independent attempts at the
same idea. Matching retail's CFG structurally and matching retail's
register allocation are different axes, and this function is a clean
negative for assuming the first helps the second.

### Proposed learning (standing check, promotion-ready -- head-requested writeup, round 19)

**A preserved "near-miss" body's own drift claim is a claim, not a fact,
and it does not stay true just because nobody edited the function
since.** Two independent instances this round (`DreamSys__AdvanceMoveCycle` above;
`func_80063144`'s divergence #2, see that report) each carried a
"clean"/"pure reordering, same word count" claim that was wrong, in the
SAME direction (both undercounted the real word length by exactly the
same mechanism), across TWO SEPARATE prior rounds each. That is not a
DreamSys anecdote -- any report anywhere in this project that states a
word count or a "no drift" claim for a body that was not rebuilt in the
SAME session as the claim is unverified until it is.

**The check, runnable by anyone, no prior context needed:**

```sh
# 1. Paste the report's exact preserved body into src/<unit>.c in place
#    of the function's INCLUDE_ASM (verbatim -- do not "improve" it first).
./build-and-verify.sh > /tmp/<name>_b.log 2>&1; echo "build exit=$?"
grep -nE 'error:|parse error|undefined reference' /tmp/<name>_b.log | head -8
.venv/bin/python3 tools/funcdiff.py <func>

# 2. Cross-check the word count directly against the compiled object --
#    do not trust funcdiff's own window arithmetic alone.
tools/binutils/bin/mipsel-linux-gnu-objdump -d build/src/<unit>.c.o \
  | awk '/<'<func>'>:/{f=1} f{print} f && /^$/{exit}'
# Count instructions start-to-last-nop and compare to the retail word
# count in the .s header: `nonmatching <func>, 0xNN` -- 0xNN/4 words.

# 3. Restore INCLUDE_ASM before doing anything else with the result.
```

**What a trustworthy answer looks like:** the compiled word count from
step 2 equals retail's `0xNN/4` EXACTLY, and `funcdiff.py` prints no
"differs outside this range" warning. Either one on its own is not
enough -- funcdiff's warning is what caught both instances this round,
but the objdump count is what CONFIRMS it is a real overshoot and not a
window artifact (a real, separate class of false positive: see
`CalcDreamColor`'s own round-19 section this same round, where an
outside-range warning was checked with `cmp -l` and turned out to be
entirely WITHIN the function's own byte range -- not drift at all). Do
not skip the objdump step because the warning already "sounds bad"; the
warning and the real defect are two different observations and only
one is decisive.

**The specific trap that generalises past both instances found this
round: a permuter `--debug` bucket label describes the scorer's internal
edit-distance operation, not a guarantee about word count.**
`func_80063144`'s divergence #2 was bucketed `Reorderings: 2` by
`permuter.py --debug`, which reads as "same instructions, different
positions, same total count" -- and was carried that way in a report for
two rounds. Rebuilding the exact body showed the real compiled function
is 1 word LONGER than retail. The permuter's own internal scoring
algorithm chose to describe the diff it found as a "reordering"; that is
a fact about the SCORER's edit-distance heuristic, not a fact about
whether the two candidate objects are the same length. Treat `--debug`'s
bucket breakdown (`Register Differences`, `Reorderings`, `Insertions`,
`Deletions`, `Stack Differences`) as a lead about WHAT KIND of change
might close the gap, never as a substitute for the objdump word count
above.

**When this check is worth spending the two minutes:** before staffing a
runner onto a function based on a report's claimed residue size or
"reorder only" classification (a head ranking a queue by parsed residues
is exactly this case), and before writing a NEW report section that
inherits a prior claim ("re-tested axis X against the existing body" is
only meaningful if that existing body was itself rebuilt first, not
assumed current).

## ROUND 20 (runner echo): the drift genuinely narrowed, 2 words -> 1 word; residue #2 isolated, not yet closed

Per the coordinator's directive ("the drift is the part worth attacking"),
spent this round on the drift specifically rather than re-attempting a
full match. Re-read the raw disassembly around the entry test fresh
(not just this report's prose): retail's `beqz $v1,.L80059CD8` (the
`unk_0xA0==0` early-out) has `addu $s3,$zero,$zero` in ITS OWN delay slot
(`ret = 0`, set unconditionally, right at the top), and the immediately
following fallthrough instruction is `addu $s3,$v1,$zero` (`ret =
unk_0xA0`, overwriting the delay-slot value on the taken path only) --
i.e. retail's real source sets `ret` to a default at the very top, then
conditionally overwrites it inside the `if`, rather than writing a
separate `else { ret = 0; }` block far down at the tail. This is the
SAME "unconditional default, then conditional overwrite" idiom this
report's own `delta = -50; if (...) delta = 50;` finding already
confirmed elsewhere in this same function -- untried at the TOP-level
`ret`/`if` structure until this round (the two previously-tried fixes,
`goto`-based early return and swapped if/else order, are different
restructurings of the SAME shape, not this one).

**Applied it:**

```c
s32 doCallback = 0;
s32 ret = 0;
...
if (this->unk_0xA0 != 0) {
    ret = this->unk_0xA0;
    ... (unchanged BIG block, no `else` clause at all) ...
}
if (!doCallback)
    this->vt->DreamSys__StopVoice(this);
return ret;
```

**Result, confirmed via `asm-differ` word-for-word against the compiled
object: the top of the function (the entry test, both `ret` assignments,
and the whole `count`/`unk_0xB4` store) now matches retail BYTE-EXACT** --
the previously-documented tail-merge defect (an extra `j` plus a
duplicated `move $s3,zero`) is GONE. `objdump` confirms the compiled
function is now **80 words, only 1 word longer than retail's 79** -- down
from the previously-confirmed 81-word/2-word overshoot. This is real,
measured narrowing of the drift, not a reshuffle: the specific defect this
report's own round-19 section identified (the tail-merge duplicate) no
longer exists in the compiled output.

**What's left is entirely residue #2, this report's own already-documented
`count`/`bit` register-and-placement issue, now isolated and slightly
better characterised.** `objdump` shows the ONE remaining extra
instruction is `bit`'s computation (`count & 1`) landing one instruction
EARLY (right after storing `count` back to `this->unk_0xB4`, BEFORE
`this->unk_0xAC` is even loaded), where retail computes it as the
`bne $unk_0xAC,4,...`'s own delay-slot filler -- unconditionally, but
scheduled LATE, adjacent to that branch, not adjacent to `count`'s store.
Tried removing the separate `bit` local entirely (inlining `(count & 1)
== 0` directly into the `doCallback = ...` assignment, no named
intermediate) -- **narrowed further to 19/79 in-range** (still 80 words,
1-word drift persists, same instruction misplaced) but did not close it.
Both variants confirmed via direct `objdump` word-count, not funcdiff's
raw window alone.

**Not closed; restored to `INCLUDE_ASM`.** This is a genuine partial
result: the drift is reduced from 2 words to 1, the previously-undocumented
tail-merge defect is fully fixed and can be dropped from this function's
open-questions list, and the sole remaining defect is now precisely the
`count`/`bit` scheduling residue this report already named (not a new
mechanism) -- narrowed, not solved.

**Commutative-add operand-order screen (per the coordinator's standing
request to apply it everywhere): NOT this class.** Neither residue here
is a commutative `addu`'s operand/destination-register choice -- one is a
tail-merge/jump-elimination optimization (now fixed) and the other is a
delay-slot-fill placement choice for an independent bitwise computation
(same FAMILY as `func_8004ABD0`'s delay-slot hoist in `class_3ac78`, not
the `addu rd,rs,rt` vs `addu rd,rt,rs` shape at all).

### Proposed learning

**The "unconditional default value, then conditional overwrite" idiom
(already known for a SINGLE scalar assigned inside one `if`/`else` pair)
generalises to a value whose default and override are set at the very
TOP of a function via a branch's own delay slot, even when the function's
C-level `if` that does the overriding is large and spans most of the
function body.** The tell that this idiom applies, read directly off the
disassembly rather than inferred from the C: the function's ENTRY branch
(not a later one) has the "default" value materialised in its OWN delay
slot, and the very next instruction (the branch's fallthrough) overwrites
it unconditionally on the taken-into-the-big-block path. Where this idiom
was NOT yet tried (a whole-function-scope `if`/`else` around a large
block, rather than a small scalar pick), applying it fixed a real,
previously load-bearing 2-word drift completely, narrowing the function's
remaining defect to a single, already-isolated, unrelated residue.

## ROUND 25 (runner echo): two new levers tried against the isolated 1-word residue, neither moved it

Reproduced round 20's exact 80/79-word, 19/79-in-range state from scratch
(confirmed identical diff, not just trusting the prior write-up) before
trying anything new, per this round's own "rebuild before you act on an
inherited figure" standing instruction.

**Lever 1: a `goto`-based early return for the OUTER entry guard**, the
exact reshape that closed the branch-order gap in this unit's
`DreamSys__StepLookOffset`/`DreamSys__StepLookYaw` this round (see those reports):

```c
doCallback = 0;
if (this->unk_0xA0 == 0) {
	ret = 0;
	goto dispatch;
}
ret = this->unk_0xA0;
... (unchanged big block) ...
dispatch:
if (!doCallback)
	this->vt->DreamSys__StopVoice(this);
return ret;
```

**Result: WORSE, not better.** GCC did not translate the `if
(unk_0xA0==0) { ...; goto dispatch; }` into retail's direct `beqz
v1,dispatch` with the main body as pure fallthrough. Instead it INVERTED
the guard (`bnez v1,MAIN`, jumping over the small "then" block) and added a
whole extra `j` plus a duplicated `move s3,zero` -- reintroducing exactly
the tail-merge defect round 20 had already eliminated. **This is a
negative data point for the block-order lever's generality**: it closed
two sibling functions' branch-order gaps outright, but here the function
ALREADY had its entry guard in the "big-block if, `ret` defaulted at top"
shape round 20 found (not the "small early-return arm" shape the lever
targets) -- converting a working "default value + big-block if" back into
an explicit small early-return arm is the WRONG direction for this
specific residue, not a shape it was ever missing.

**Lever 2: a bare `__asm__("")` scheduling barrier**, placed immediately
inside `if (count < 4) { ... }` before the `this->unk_0xAC == 4` check, on
the theory that the residue (`(count & 1)` computed one instruction EARLY
relative to where retail schedules it, adjacent to the `this->unk_0xAC`
branch instead of adjacent to `count`'s own store) is a pure
INSTRUCTION-ORDER question the barrier is licensed to fix per CLAUDE.md's
own test ("if removing it changes ORDER only, it is allowed").

**Result: WORSE.** The barrier did suppress the early hoist (a `nop` now
sits in the delay slot instead), but GCC did not then schedule the ORIGINAL
one-instruction `andi $v0,$a0,0x1` into the freed slot the way retail does.
Instead it recompiled the whole `(count & 1) == 0` expression as a
DIFFERENT two-instruction sequence (`xori $s1,$a0,0x1` / `andi
$s1,$s1,0x1`) split across the delay slot and the following instruction —
same result value, one instruction longer overall. So the barrier changed
more than order here: it triggered a different EXPRESSION LOWERING for the
same boolean, not a pure reordering of the same instructions. Per the
banned-lever test this is disqualifying (removing/adding the barrier changes
which INSTRUCTIONS exist, not just their order), so this is not a legitimate
lever for this residue either, despite superficially matching the
"instruction order only" criterion at the point it was inserted.

**Both reshapes reverted; `INCLUDE_ASM` restored with round 20's own
80/79-word body preserved as the best-reached (see `#if 0` block in
`src/DreamSys.c`, positioned in ROM order).** No further attempts made
this round; two independent, previously-unexplored axes (the block-order
lever, and a scheduling barrier at the exact diverging instruction) both
failed to close the last word, which is a stronger signal that this
specific residue needs a genuinely different idea (or a permuter run) than
another hand attempt would supply.

### Proposed learning (round 25)

The block-order lever (explicit `goto` for an early-return arm) is not a
strictly-dominant rewrite over the "default value at top, conditionally
overwritten in a big-block `if`" idiom this same function already
demonstrates (round 20) — applying it where the LATTER shape is already
correct undoes working ground rather than improving it. Before trying the
`goto` lever, check whether the function's entry guard already has its
default value living in the guard's OWN delay slot (visible directly in
the `.s`): if so, the big-block-`if`/default-at-top shape is what's needed,
not an explicit early-return arm.
