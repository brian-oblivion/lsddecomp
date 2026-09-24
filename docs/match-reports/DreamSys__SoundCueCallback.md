# DreamSys__SoundCueCallback — MATCH (25/25 words)

> Renamed from `func_8005A1F4` on 2026-09-22 (tools/rename.py). Address 0x8005a1f4.

**Unit:** DreamSys · **Size:** 25 instructions · **Status: byte-exact, whole-image SHA1 confirmed.**

Previously filed as a STALL at 13/25 across three rounds. Round 19 (runner
delta) found the earlier reports' own semantic read of the function was
**wrong**, fixed it, and then found the exact source idiom that closes the
remaining register-identity residue. History below in arrival order; read
this section first.

## What it actually does (corrected — earlier rounds misread one store)

Vtable `+0x194`, never called through the vtable within this unit --
referenced only by RAW ADDRESS as `InitSoundCueSet`'s 5th argument (still
INCLUDE_ASM, uncarved `code_179d8`; see `DreamSys__StopDrift`'s report). If
`arg1->mode == 1`: tests whether `arg1->value` is evenly divisible by 20.
If it is, writes `9` and `-1` into `field_0x1C`/`field_0x20`. **If it is
NOT, writes the SAME literal `9` (not the quotient) and `-1` into
`field_0x30`/`field_0x34`.**

## The misread, and how it was found

Three prior rounds' reports (and the struct/doc prose) claimed the
not-divisible branch stores **the quotient** (`arg1->value / 20`) into
`field_0x30`. Reading `asm/nonmatchings/DreamSys/DreamSys__SoundCueCallback.s` directly,
instruction by instruction, shows this is wrong:

```
...
subu  $v0, $v0, $v1      ; v0 = quotient (value/20) -- last real write to v0
sll   $v1, $v0, 2
addu  $v1, $v1, $v0
sll   $v1, $v1, 2        ; v1 = quotient*20
bne   $a0, $v1, .L8005A244   ; branch (taken == NOT divisible)
 ori  $v0, $zero, 0x9    ; delay slot -- ALWAYS executes, overwrites v0 with 9
sw    $v0, 0x1C($a1)     ; [fallthrough / divisible] field_0x1C = 9
...
.L8005A244:
sw    $v0, 0x30($a1)     ; [branch taken / not divisible] field_0x30 = v0 = 9, NOT the quotient
...
```

A MIPS delay slot always executes, regardless of whether the branch is
taken. `ori $v0, $zero, 0x9` sits in the `bne`'s delay slot, so it runs on
**both** paths, unconditionally overwriting `$v0` (which held the quotient)
with the literal `9` before either path can read it. The quotient is
computed **only** to test divisibility (via the multiply-back
`quotient*20` and comparison) and is then thrown away — no path ever
stores it anywhere. This is the same "materialize a constant once, reuse it
across both arms with different destinations" idiom already documented for
`DreamSys__AdvanceMoveCycle`'s `delta = -50; if (...) delta = 50;` shape, just with the
constant reused identically (9 == 9) rather than overwritten (-50 -> 50).

**Every previous report's C had this backwards** (`arg1->field_0x30 =
arg1->value / 20;`), which is exactly the kind of "wrong CAUSE" CLAUDE.md
warns costs the most: it is arithmetically plausible, it does not fail to
compile, and it silently capped every subsequent round's best score at
13/25 because the store's VALUE was wrong, not just its register.

## The fix, and the idiom that closed the whole function

Fixing only the misread (store `9`, not the quotient) immediately moved the
score from 13/25 to **17/25** — every store and the branch target now
matched exactly; the only remaining diff was the well-known register-swap
in the division/multiply-back sequence itself (`$v0`/`$v1` swapped
throughout, same values, same instruction count).

A short `-j 6 --stop-on-zero` permuter run (17 iterations, seeded with the
corrected body) found the closing idiom: **materialize the comparison's
boolean RESULT into a named `int` local before branching on it**, rather
than testing the expression directly in the `if`:

```c
void DreamSys__SoundCueCallback(void *arg0, Func8005A1F4Arg *arg1)
{
	s32 isDivisible;

	if (arg1->mode == 1) {
		isDivisible = (arg1->value % 20) == 0;
		if (isDivisible) {
			arg1->field_0x1C = 9;
			arg1->field_0x20 = -1;
		} else {
			arg1->field_0x30 = 9;
			arg1->field_0x34 = -1;
		}
	}
}
```

This is byte-exact (25/25), confirmed with the real oracle
(`./build-and-verify.sh` exits 0, whole-image SHA1 matches). Landed in
`src/DreamSys.c` in place of the `INCLUDE_ASM`.

### Proposed learning

**Two independent, separable defects were stacked in this one 13/25
residue, and fixing only one of them would still have looked like a clean
register-identity stall.** The value bug (storing the wrong constant) and
the register-identity bug (which named local held the boolean) do not
interact in the diff output the way you'd expect — a wrong VALUE at a
store looks exactly like "12 more register-identity mismatches", because
every downstream instruction that touches the wrong value also picks
different registers than retail. **Always re-derive a "verify this stall"
task from the raw disassembly, not from the existing report's C** — three
rounds of "re-tested the type axis, confirmed unchanged" all re-tested
against the WRONG source and could never have found this, because the type
axis was never the actual defect.

The closing idiom itself is also worth generalising: **`s32 result = (cond);
if (result) { ... }` is not always equivalent to `if (cond) { ... }` to
this compiler** — materializing a boolean comparison's result into its own
named local before the branch changed register allocation in the
surrounding division sequence, even though the local is used exactly once,
immediately, with no other change to control flow. Try this split
specifically when a register-identity residue surrounds a comparison
whose truth value feeds a single subsequent `if`.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass) —
original (mis-)filing at 13/25.
Round 2026-09-04ish, runner delta (round 18) — re-tested type/width axis
and four structural reshapes against the WRONG (misread) body; all
correctly reported "unchanged" because the axis was never the real defect.
**Round 19, runner delta — found the misread, fixed it, then closed the
function with the boolean-materialization idiom above. MATCHED, 25/25.**

---

## History (pre-round-19, preserved for context; superseded above)

The sections below are the original STALL filing and round-18 re-test, kept
verbatim for provenance. They describe a body that is now known to have
been semantically wrong at `field_0x30`; do not use the C shown here.

### Original best-reached body (WRONG — do not use; superseded above)

```c
#if 0
typedef struct Func8005A1F4Arg {
	s32 mode;
	s32 value;
	s8 unknown_values_0x8[0x14];
	s32 field_0x1C;
	s32 field_0x20;
	s8 unknown_values_0x24[0xC];
	s32 field_0x30;
	s32 field_0x34;
} Func8005A1F4Arg;

void DreamSys__SoundCueCallback(void *arg0, Func8005A1F4Arg *arg1)
{
	if (arg1->mode == 1) {
		if (arg1->value % 20 == 0) {
			arg1->field_0x1C = 9;
			arg1->field_0x20 = -1;
		} else {
			arg1->field_0x30 = arg1->value / 20;   /* WRONG: retail stores literal 9 here, not the quotient */
			arg1->field_0x34 = -1;
		}
	}
}
#endif
```

### Residue as originally (mis-)diagnosed: "register identity in a straight-line division-and-reconstruction, no calls at all"

Retail computes the quotient (`a0/20`) into `$v0`, then reconstructs
`quotient*20` for the remainder check via `sll`/`addu`/`sll` all still
targeting `$v0`-derived registers in a specific pattern; this body's
natural codegen computes the SAME quotient into `$v1` instead, with the
reconstruction chain's registers correspondingly swapped throughout --
plus one extra `li $v0,-1` that appears to be spuriously hoisted into the
"then" branch before being overwritten by `li $v0,9`. 13/25 words match;
the mismatched 12 are entirely this register-naming cascade, not a logic
error (every VALUE computed is correct, only the register choice differs).

**Argument-register test, run per this round's standing instruction:**
this function makes NO calls at all (straight-line arithmetic and stores
only) -- there is no `jal`/`jalr` anywhere in its body, so neither `$v0`
nor `$v1` is ever live into a subsequent call. This comes out firmly on
the "true register-identity stall" side, not the "missing parameter"
side that `DreamSys__StopVoice` turned out to be in round 2026-08-30-c.

Reshapes tried, all producing this identical 13/25 residue (against the
WRONG body -- these results do not carry over to the corrected function):

1. Named `quotient` temp, computed once, reused for both the comparison
   and the else-branch store (shown structurally above, with a temp).
2. Same, but with an explicit `value` temp for `arg1->value` too (in case
   repeated field dereferences were confusing register allocation).
3. Fully inlined, no temps: `arg1->value == (arg1->value/20)*20` and a
   second, independent `arg1->value/20` in the else branch.
4. `%` instead of the multiply-back-and-compare form -- byte-identical to
   attempt 3, confirming GCC 2.6.3 treats them as the same expression.
5. A bare `__asm__("")` scheduling barrier right after the `mode == 1`
   check -- made it WORSE (a fresh `nop` and further-shifted residue),
   confirming this isn't an ordering issue reachable by a barrier.

### Round: coordinator-directed re-test (runner delta, round 18) -- corrects an earlier hypothesis, tests the type/width axis directly, both "resolved" against the wrong body

**Search concluded.** 48078 iterations, `-j 6 --stop-on-zero`, bounded by
`timeout 600`. Best score 50 (from base 255), plateaued, never reached
zero. Exit code **124** this time read correctly from the harness's own
captured output (the earlier `Entity__MoodCue81` run's logging defect --
echo appended to the SAME redirected log file -- did not recur here
because this run's echo was left unredirected, landing in the harness's
own task-output file instead; consistent with that defect being specific
to the redirect target, not the search itself).

**My own stated hypothesis from mid-round ("retail stores
`field_0x34 = -1` once, common to both branches") was WRONG -- tested
directly and falsified, not just re-confirmed.** Built the 50-point
candidate (`output-50-1/source.c`), `objdump`'d it, and diffed
instruction-for-instruction against `target.o`:

```
candidate: 48: j 58   (jumps to right before the f34 store -- BOTH branches execute it)
target:    48: j 5c   (jumps straight to jr ra -- the THEN branch SKIPS the f34 store entirely)
```

Retail's `field_0x34 = -1` is ELSE-branch-only, exactly as the ORIGINAL
report's C already had it. My commonization idea was a misreading of
what mutation gave the permuter a favorable score, not a genuine
structural insight -- the 50-point candidate is not "50 words closer to
correct," it trades the branch-target bug for a coincidentally similar
mechanical diff count. **Flagging this explicitly: a lower permuter score
reached via a structurally different control-flow change is not
automatically progress** -- it must be read back against the disassembly
before being trusted as a lead, same discipline as a `--debug` base score.
(Note, round 19: this hunt for a "common field_0x34" was on the right
TRACK -- retail really does hoist a common constant across both arms --
but pointed at the wrong field/value; it is `field_0x1C`/`field_0x30`
both getting `9`, not `field_0x34` alone.)

**Directly tested the coordinator's type/width lever against this
function's own disassembly, rather than reasoning about it.** Every
load and store in `DreamSys__SoundCueCallback.s` is confirmed word-width:

```
lw  $v1, 0x0($a1)     # mode  -- lw, so s32/u32, not narrower
lw  $a0, 0x4($a1)     # value -- lw, so s32/u32, not narrower
sw  ..., 0x1c/0x20/0x30/0x34($a1)   # all four output fields -- sw, so s32
```

No narrower load/store (`lb`/`lbu`/`lh`/`lhu`/`sb`/`sh`) appears anywhere
in this function, which rules out a width mismatch on ANY field
end-to-end -- a differently-sized declared type would change the
opcode itself, not just the register choice, and every opcode here
already matches what `include/DreamSys.h` declares. Signedness is
similarly constrained on `value`: retail's division sequence
(`sra $v1,$a0,0x1f` / `mfhi` / `sra` / `subu` -- the sign-correction
idiom for signed division-by-constant) is present, which is exactly
GCC's SIGNED-division codegen; an unsigned `value` would compile to a
plain multiply-high-and-shift with no sign correction at all. Retail
has the correction, so `value` is confirmed signed, matching the current
declaration.

**Four additional isolated `cc1` probes (extracted per
`CLAUDE.md`'s "escalate, do not experiment" reproducer recipe, each
under a second), all producing byte-identical (still register-swapped)
output**, ruling out four more candidate axes:
1. Named `s32 q = value / 20;` temp, used in both the comparison and the
   else-branch store (byte-identical to the plain form).
2. Reversed multiply operand order (`(value/20) * 20` vs `20 * (value/20)`)
   -- GCC's constant-propagation normalizes both to the same form.
3. Reordering the top-level structure (`if (mode != 1) return;` early-out
   followed by a separate `value` load, instead of the nested
   `if (mode == 1) { ... }`) -- identical.
4. (Combined with #1 in the same probe.)

**Conclusion (round 18, now superseded): this is now tested, not just
re-derived, and the verdict is unchanged -- genuine register-identity
stall.** (Round 19 note: every one of these axes was tested against the
body with the wrong `field_0x30` value, so none of them could have found
the real defect; this conclusion held for the wrong reason.)

No source changes at the time; `INCLUDE_ASM` untouched.

## Naming

`DreamSys__SoundCueCallback` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005A1F4`.

Named for where its ADDRESS goes, which is the only thing that
identifies it: `DreamSys__SelectCallback98`'s mode-2 case passes
`this->vt->DreamSys__SoundCueCallback` as `InitSoundCueSet`'s fifth argument, and
that function (src/code_179d8_e.c, matched) stores it in `SoundCueSet::callback`.
Nothing in any carved unit calls it, so its own parameter struct stays local and
opaque: the body only picks one of two field pairs to write 9 and -1 into,
depending on whether `arg1->value` is a multiple of 20.
