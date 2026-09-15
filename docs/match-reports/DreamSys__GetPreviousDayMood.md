> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `nop_mflo_mfhi`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# DreamSys__GetPreviousDayMood

**Unit:** DreamSys · **Size:** 64 words (0x100 bytes) · **Status:** STALL —
**solved at the C level**; blocked only by the already-documented
`nop_mflo_mfhi` toolchain issue. Restored to `INCLUDE_ASM`, `build exit=0`,
whole-image SHA1 green.

## The body. It is correct — all 64 instructions.

```c
void DreamSys__GetPreviousDayMood(DreamSys *this, MoodGraphPoint *target, bool unknown)
{
	s32 upper = 0;
	s32 dynamic = 0;

	if (unknown) {
		if (this->currentYear != 0 || this->currentDay != 0) {
			s32 idx;

			idx = this->currentDay - 1;
			dynamic = this->moodPreviousDays[idx].axis.dynamic;
			upper = this->moodPreviousDays[idx].axis.upper;
		}
	} else {
		s32 count;
		count = 0x16D;
		if (this->currentYear == 0)
			count = this->currentDay;
		if (count != 0) {
			MoodGraphPoint *p;
			s32 i;

			p = this->moodPreviousDays;
			i = 0;
			if (upper < count) {
				do {
					i++;
					dynamic += p->axis.dynamic;
					upper += p->axis.upper;
					p++;
				} while (i < count);
			}
			dynamic /= count;
			upper /= count;
		}
	}
	target->axis.dynamic = dynamic;
	target->axis.upper = upper;
}
```

`asm-differ` against this body: **every single instruction matches retail,
same registers, same branch targets, same instruction order — except for two
extra `nop`s** maspsx inserts between the first division's `mflo $t1` and the
second division's `div $zero,$t0,$a2`. That is the whole residue; it is
literally two `nop` words, nothing else. Everything up to and after that pair
of words is byte-identical, including the shared-tail store pattern, the
loop's exact instruction order, the range-check magic-multiply divisions'
overflow guards, and (once found — see below) the exact register identity of
both accumulators.

## Reading order (what makes this function worth documenting)

`DreamSys::currentDay`/`currentYear` at offsets `0x17C`/`0x180` and
`moodPreviousDays` at `0x190` (all already-established fields) determine one
of two computations selected by the `unknown` parameter:

- `unknown` true: read back ONE day's stored mood (the day before
  `currentDay`, or the special case where `currentYear` has already ticked
  over and `currentDay` is still 0).
- `unknown` false: average every day so far this year (`currentDay` days, or
  the full `365` if the year has already advanced) into `dynamic`/`upper`.

Both paths converge on a SHARED two-byte store at the very end
(`target->axis.dynamic = dynamic; target->axis.upper = upper;`) — the
single-day path does not do its own separate store-and-return, it just leaves
`dynamic`/`upper` set and falls through to the same final write the average
path uses. Writing it with an early `return` after a direct store (the first,
wrong attempt) breaks this sharing and costs 2 words, changing which merge
point a stray `j` lands on. This is the same species of residue documented
for `DreamSys__TimerTick` this round, closed instead of stalled here because
the "shared tail" was reachable by removing the early return rather than by
choosing between duplicate physical copies.

## Register-identity residue (t0 vs t1) — closed by declaration order alone

An intermediate attempt reproduced every branch and instruction CORRECTLY but
with `dynamic`'s accumulator in `$t1`/retail's `$t0` role swapped (and vice
versa) — a textbook "same instructions, different registers" residue.
**Per CLAUDE.md rule 6, this was NOT fixed with a register constraint.**
Swapping the two locals' DECLARATION ORDER (`s32 upper = 0; s32 dynamic = 0;`
instead of `dynamic` first) closed it for free, with no other code changed.
Declaring `upper` first before this fix, in isolation, without the other
control-flow fixes below already in place, made the residue WORSE (44 -> 18
words) — the declaration-order lever only produced the right effect once the
surrounding control flow already matched retail's, which is worth remembering
before re-testing a declaration-order swap against a still-wrong body.

## The loop guard — reusing an already-zero register instead of testing 0

The loop's pre-test (whether to enter the accumulate loop at all) is NOT
`if (0 < count)` written literally — retail computes `slt $v0, $t0, $a2`,
reusing the `upper` accumulator register (already 0 at that point, untouched
since function entry) as the left-hand side, rather than materializing a
fresh zero. Writing `if (upper < count)` instead of `if (0 < count)`
reproduced this exactly. This is the same "reuse a register that already
holds zero instead of loading a fresh immediate" idiom noted for
`DreamSys__TimerTick`'s `unk_0x44`-adjacent zero test earlier this round, now
confirmed a second time with a completely different pair of values.

Getting the surrounding statement PLACEMENT right mattered too: `p =
this->moodPreviousDays;` and `i = 0;` both had to be hoisted OUTSIDE the
`if (upper < count)` guard (unconditional, matching retail's own
unconditional pointer-setup-then-optional-loop shape) rather than left inside
it — moving only the pointer setup out first (leaving `i = 0` inside) reduced
but did not close the residue; moving both out closed the branch structure
completely.

## The residue: `nop_mflo_mfhi`, confirmed identical to `IsDaySpecial`'s

```
retail:  ...  div $zero,$t1,$a2  ...  mflo $t1  |  div $zero,$t0,$a2  ...
built:   ...  div $zero,$t1,$a2  ...  mflo $t1  | nop | nop | div $zero,$t0,$a2  ...
```

Two `nop`s inserted between the first division's `mflo` result read and the
second division's `div`, and nothing else differs anywhere in the function.
This is the exact residue class already fully diagnosed and escalated in
`docs/research/addiu-at-blocker.md` (addendum 2026-09-02b, discovered via
`IsDaySpecial`, matched this same round): Psy-Q cc1 emits *commented-out*
`#nop`/`#nop` hazard hints around back-to-back `mult`/`mflo`-`mfhi` pairs, and
maspsx's `nop_mflo_mfhi` flag (`True` at the project's pinned
`--aspsx-version=2.34`) turns those hints into real instructions retail does
not have. **Not a scheduling residue and there is no source shape that closes
it** — per the existing research, the flag cannot be toggled independently of
three other nop-insertion/addressing rules the pin also needs elsewhere. Not
re-derived from scratch here (no isolated reproducer re-run for this specific
function) since the byte pattern is unambiguous and identical to the
already-confirmed instance.

## New knowledge

- **`DreamSys::currentDay`/`currentYear`/`moodPreviousDays` all confirmed**
  (already-named fields, offsets validated by this function's exact byte
  match up to the blocker).
- Reinforces (does not newly discover) two idioms already logged this round
  for `DreamSys__TimerTick`/`func_8005AB2C`: reusing an already-zero register
  for a comparison instead of a fresh immediate, and needing statements
  hoisted OUTSIDE a guard to match an unconditional-setup-then-optional-work
  shape.
- **Second independent instance of the `nop_mflo_mfhi` blocker**, in a
  different unit function than `IsDaySpecial` but the identical byte
  signature (two `nop`s between a `mflo` and the next `div`). Strengthens the
  case that this is a general "any function with two back-to-back integer
  divisions" hazard under the pinned toolchain, not a one-off.

### Proposed learning

`nop_mflo_mfhi` is not a one-instance quirk: two independent functions in two
different rounds (`IsDaySpecial`, this function) hit the identical two-`nop`
residue between consecutive `mflo`/`div` sequences. Worth a corpus census (how
many queued functions perform two or more integer divisions in sequence) the
next time someone revisits `docs/research/addiu-at-blocker.md` — this class
may be as broad as the `addiu_at` census itself.
