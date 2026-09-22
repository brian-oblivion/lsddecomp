# DreamSys__CheckTunnelHeading — MATCHED 59/59

> Renamed from `func_8005BD3C` on 2026-09-22 (tools/rename.py). Address 0x8005bd3c.

**Unit:** DreamSys · **Size:** 59 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (4 references, the
first to `D_8008ACBC`) AND `addiu_at` (2 runtime-indexed global loads
through `D_800889B8`/`D_80088858`). Round 21 resolved `addiu_at`; round 42
resolved `gp_rel`. Attempted fresh this round (a stretch pick beyond the
assigned queue).

## What it does

Looks up a "heading" byte from `D_800889B8[D_8008ACBC][D_8008ACC0]`,
validates it against `currentPos`'s stored heading via the already-matched
`IsHeadingAligned` (a cardinal-direction proximity test), and on success
writes one or two computed pointers (into a `DirectionTableEntry`-strided
table, `D_80088758`) through its two optional output parameters. Called by
`DreamSys__TryTunnelLink` (still `INCLUDE_ASM`) as `DreamSys__CheckTunnelHeading(&this->unk_0x888,
&this->unk_0x884, local)`.

## New declarations needed

`D_800889B8` and `D_80088858` are per-stage tables of pointers to byte
arrays (4-byte stride, indexed by `D_8008ACBC`/`D_8008ACC4` respectively,
each further indexed by `D_8008ACC0`/`D_8008ACC8` to read a single `u8`):

```c
extern u8 *D_800889B8[];
extern u8 *D_80088858[];
```

`D_80088758` is more subtle: it is a `DirectionTableEntry`-strided (12-byte)
table whose first element sits exactly 4 bytes before the SEPARATELY
referenced `D_8008875C` (the angle table `IsHeadingAligned` already indexes,
matched earlier this round's queue). splat drew a symbol boundary there
because `D_8008875C` is independently referenced elsewhere, not because the
underlying retail data is genuinely two different tables. This function
only ever ADDRESS-TAKES an element (`&D_80088758[i]`, storing the pointer
into an output parameter) and never dereferences one, so the element type
only needs to fix the STRIDE -- reusing the already-declared
`DirectionTableEntry` (12 bytes) is exact and avoids inventing a third
local type for one call site:

```c
extern DirectionTableEntry D_80088758[];
```

Also moved the `DirectionCheckArg` typedef, the `DirectionTableEntry`
typedef, `extern DirectionTableEntry D_8008875C[];`, and a new forward
declaration `extern s32 IsHeadingAligned(DirectionCheckArg *a0, u8 a1);` to
BEFORE `DreamSys__CheckTunnelHeading` (they previously sat between it and
`IsHeadingAligned`'s own definition) -- `DreamSys__CheckTunnelHeading` needs `DirectionCheckArg`
to cast its own `arg2` before forwarding it, and needs to call
`IsHeadingAligned`, which is defined later in ROM order. This is a pure
textual reordering of type declarations and one forward declaration; no
function DEFINITION moved, so ROM address order is unaffected.

## Attempt 1: right values, wrong exit-block layout (36/59, 84364 bytes drift)

```c
s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2)
{
	u8 heading;
	s32 idx;

	heading = D_800889B8[D_8008ACBC][D_8008ACC0];
	if (!IsHeadingAligned((DirectionCheckArg *)arg2, heading))
		return 0;

	if (arg1 != NULL)
		*arg1 = (s32)&D_80088758[heading];

	if (arg0 != NULL) {
		idx = D_80088858[D_8008ACC4][D_8008ACC8];
		*arg0 = (s32)&D_80088758[idx];
	}
	return 1;
}
```

An early `if (!cond) return 0;` guard, with the "success" path falling
straight through to a final `return 1;`, compiled `return 0`'s value
directly into the SAME branch that jumps to the shared epilogue (`v0 = 0`
in the branch's own delay slot, landing exactly on the epilogue's first
instruction) -- one word SHORTER than retail. Retail keeps the `v0 = 0`
case as its OWN separate instruction physically ADJACENT to the epilogue
but not merged into it, which forces the "success" path (`v0 = 1`) to jump
OVER that separate `v0 = 0` block to reach the same epilogue -- one
instruction retail has that this shape does not.

## Fix: single shared `result` variable, if/else instead of early return (59/59)

```c
s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2)
{
	u8 heading;
	s32 idx;
	s32 result;

	heading = D_800889B8[D_8008ACBC][D_8008ACC0];
	if (IsHeadingAligned((DirectionCheckArg *)arg2, heading)) {
		if (arg1 != NULL)
			*arg1 = (s32)&D_80088758[heading];

		if (arg0 != NULL) {
			idx = D_80088858[D_8008ACC4][D_8008ACC8];
			*arg0 = (s32)&D_80088758[idx];
		}
		result = 1;
	} else {
		result = 0;
	}
	return result;
}
```

Writing both outcomes as assignments to one `result` variable converging on
a single `return result;` (rather than an early `return 0;` inside the
guard) reproduced retail's exact block layout: the `else` branch (`result =
0`) ends up placed as its own block immediately before the epilogue, and
the `if` branch's own final assignment (`result = 1`) needs the jump-over
retail has. Byte-exact on this second attempt.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py DreamSys__CheckTunnelHeading` -> `59/59 words match`.

### Proposed learning

Same family as `Test4InstantTeleporters`'s and `Test4StageTransition`'s lessons
earlier this round, now confirmed a third time with a genuinely different
shape (an early-return GUARD at the top of a function, not an if/else
spanning the whole body): an early `if (!cond) return X;` at the top of a
function can compile with its return value merged directly into the shared
epilogue (no separate block, no jump) when retail actually keeps that
value's block SEPARATE and reaches the epilogue only via fallthrough from
it. Converting the guard into an if/else with a single shared `result`
variable and one `return` at the end is a cheap, mechanical lever to try
whenever an early-return guard's residue is "off by one word, values
otherwise correct."

## Provenance

round 43, runner ALPHA, unit DreamSys (stretch pick beyond the assigned
queue).
