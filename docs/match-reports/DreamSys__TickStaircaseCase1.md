# DreamSys__TickStaircaseCase1 — MATCHED

> Renamed from `func_8005AC24` on 2026-09-22 (tools/rename.py). Address 0x8005ac24.

**Unit:** DreamSys · **Size:** 81 words (99 instructions incl. delay
slots/nops, `0x4B424`-`0x4B568`) · **Status:** MATCHED (81/81 words),
whole-image SHA1 verified green by `build-and-verify.sh`. Round 2026-09-02,
runner BRAVO. Matched on the first attempt — no residue.

## What it does

A "retry/attempt band" gate, the same family as the already-matched
`DreamSys__TickStaircaseCase0`/`DreamSys__TickStaircaseCase2`/`DreamSys__TickStaircaseCase3` a few functions earlier in
this unit (all four share the identical skeleton: on the first call,
initialize `this->unk_0x91C` via `DreamSys__ApplyRelativeOffset` with a per-function
`struct RelativePos` constant; branch on `this->unk_0xAC != 4`; in each
arm, bail with `return 1;` past a threshold, otherwise OR together 2-3
half-open `[lo, lo+len)` band tests on `this->unk_0x914` to conditionally
set a flag/fire a call; finish by conditionally setting `this->unk_0x88`,
unconditionally setting `this->unk_0xA0 = 1;`, bumping `this->unk_0x914`,
and returning 0). This one has THREE bands per arm (its siblings have two),
and reuses `sRotationYawMinus45` (already named, by `DreamSys__TickStaircaseCase3`) for its
`SceneNode__UpdateRotation` call in the `unk_0xAC == 4` arm.

Both blocker screens are clean: no `gp_rel` hit and no
`addiu $at, $at, %lo` hit anywhere in `DreamSys__TickStaircaseCase1.s`.

## New knowledge

- `sStaircaseOffset1` (`struct RelativePos`) — this function's own per-instance
  constant passed to `DreamSys__ApplyRelativeOffset`, sibling to the already-named
  `sStaircaseOffset0`/`sStaircaseOffset2`/`sStaircaseOffset3`.

No struct fields needed new names or types — every field this function
touches (`unk_0x914`, `unk_0xAC`, `unk_0xA4`, `unk_0x88`, `unk_0xA0`,
`unk_0x91C`) and every helper it calls (`DreamSys__ApplyRelativeOffset`, `SceneNode__UpdateRotation`)
were already established by the sibling functions.

## Source

```c
s32 DreamSys__TickStaircaseCase1(DreamSys *this)
{
	s32 flag;

	if (this->unk_0x914 == 0) {
		DreamSys__ApplyRelativeOffset(this, &sStaircaseOffset1, &this->unk_0x91C);
	}
	if (this->unk_0xAC != 4) {
		if (this->unk_0x914 >= 0x95)
			return 1;
		if ((u32)(this->unk_0x914 - 0x16) < 0xF || (u32)(this->unk_0x914 - 0x39) < 0x10 || (u32)(this->unk_0x914 - 0x6E) < 0xF) {
			this->unk_0xA4 = 1;
		}
		flag = (u32)(this->unk_0x914 - 0x39) < 0x35;
	} else {
		if (this->unk_0x914 >= 0x19)
			return 1;
		if ((u32)(this->unk_0x914 - 6) < 2 || (u32)(this->unk_0x914 - 0xB) < 2 || (u32)(this->unk_0x914 - 0x14) < 2) {
			this->vt->SceneNode__UpdateRotation(this, 0, &sRotationYawMinus45);
		}
		flag = (u32)(this->unk_0x914 - 3) < 0xE;
	}
	if (flag) {
		this->unk_0x88 = 2;
	}
	this->unk_0xA0 = 1;
	this->unk_0x914++;
	return 0;
}
```

## Derivation

Read the disassembly cold first (before checking for siblings) and derived
the band boundaries directly from the `addiu`/`sltiu` immediates. Then
found `DreamSys__TickStaircaseCase0`/`DreamSys__TickStaircaseCase3` already in this same file just above
the `INCLUDE_ASM` line and confirmed the template — critically, that
`DreamSys__TickStaircaseCase3` already answers the one real design question this shape
poses: whether to cache `this->unk_0x914` in a local across the multiple
band tests, or re-mention `this->unk_0x914` directly each time. The
sibling's answer is "re-mention it, every time, including within the same
OR-chain" — no local at all for the field, only a `flag` local for the
FINAL band's result (since that one feeds a separate, later `if`). Copied
that idiom verbatim (adjusted for three bands instead of two) and it
matched immediately, with zero register-identity or reload residue —
confirming the sibling's derivation rather than needing to re-derive it.

## Proposed learning

**When a function's disassembly closely resembles an ALREADY-MATCHED
function elsewhere in the SAME file, check for it and copy its exact idiom
(local-variable-vs-repeated-field-access choices especially) before
deriving from first principles.** `DreamSys__TickStaircaseCase3`'s C, sitting a few dozen
lines above this function's own `INCLUDE_ASM` line, answered in advance
the exact "cache in a local or re-read the field" question that has cost
other functions in this unit multiple attempts (see
`docs/match-reports/DreamSys__AdvanceMoveCycle.md`, same round, same unit, unresolved
for a different field). This is cheaper than the permuter and cheaper than
manual derivation, and it is very easy to miss when working strictly
top-to-bottom through a queue instead of first scanning nearby already-
matched code for a template.

## Naming

- **Tier B.** Table index 1 of the same sStaircaseTickFns family as DreamSys__TickStaircaseCase0, against sStaircaseOffset1; same evidence and same caveat.
