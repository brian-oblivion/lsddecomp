# DreamSys__CheckTunnelHeading — MATCHED 59/59

> Renamed from `func_8005BD3C` on 2026-09-22 (tools/rename.py). Address 0x8005bd3c.

**Unit:** DreamSys · **Size:** 59 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (4 references, the
first to `gLinkSrcStage`) AND `addiu_at` (2 runtime-indexed global loads
through `TUNNEL_ENTER_HEADINGS`/`TUNNEL_EXIT_HEADINGS`). Round 21 resolved `addiu_at`; round 42
resolved `gp_rel`. Attempted fresh this round (a stretch pick beyond the
assigned queue).

## What it does

Looks up a "heading" byte from `TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex]`,
validates it against `currentPos`'s stored heading via the already-matched
`IsHeadingAligned` (a cardinal-direction proximity test), and on success
writes one or two computed pointers (into a `DirectionTableEntry`-strided
table, `CARDINAL_ROTATIONS`) through its two optional output parameters. Called by
`DreamSys__TryTunnelLink` (still `INCLUDE_ASM`) as `DreamSys__CheckTunnelHeading(&this->unk_0x888,
&this->unk_0x884, local)`.

## New declarations needed

`TUNNEL_ENTER_HEADINGS` and `TUNNEL_EXIT_HEADINGS` are per-stage tables of pointers to byte
arrays (4-byte stride, indexed by `gLinkSrcStage`/`gLinkDstStage` respectively,
each further indexed by `gLinkTriggerIndex`/`gLinkSpawnIndex` to read a single `u8`):

```c
extern u8 *TUNNEL_ENTER_HEADINGS[];
extern u8 *TUNNEL_EXIT_HEADINGS[];
```

`CARDINAL_ROTATIONS` is more subtle: it is a `DirectionTableEntry`-strided (12-byte)
table whose first element sits exactly 4 bytes before the SEPARATELY
referenced `CARDINAL_ANGLES` (the angle table `IsHeadingAligned` already indexes,
matched earlier this round's queue). splat drew a symbol boundary there
because `CARDINAL_ANGLES` is independently referenced elsewhere, not because the
underlying retail data is genuinely two different tables. This function
only ever ADDRESS-TAKES an element (`&CARDINAL_ROTATIONS[i]`, storing the pointer
into an output parameter) and never dereferences one, so the element type
only needs to fix the STRIDE -- reusing the already-declared
`DirectionTableEntry` (12 bytes) is exact and avoids inventing a third
local type for one call site:

```c
extern DirectionTableEntry CARDINAL_ROTATIONS[];
```

Also moved the `DirectionCheckArg` typedef, the `DirectionTableEntry`
typedef, `extern DirectionTableEntry CARDINAL_ANGLES[];`, and a new forward
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

	heading = TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
	if (!IsHeadingAligned((DirectionCheckArg *)arg2, heading))
		return 0;

	if (arg1 != NULL)
		*arg1 = (s32)&CARDINAL_ROTATIONS[heading];

	if (arg0 != NULL) {
		idx = TUNNEL_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
		*arg0 = (s32)&CARDINAL_ROTATIONS[idx];
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

	heading = TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
	if (IsHeadingAligned((DirectionCheckArg *)arg2, heading)) {
		if (arg1 != NULL)
			*arg1 = (s32)&CARDINAL_ROTATIONS[heading];

		if (arg0 != NULL) {
			idx = TUNNEL_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
			*arg0 = (s32)&CARDINAL_ROTATIONS[idx];
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

## Naming

- **Tier B.** Near-identical body to DreamSys__CheckStaircaseHeading, differing only in which per-stage heading table it indexes (TUNNEL_ENTER_HEADINGS/TUNNEL_EXIT_HEADINGS here); called from DreamSys__TryTunnelLink.

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* TUNNEL_ENTER_HEADINGS: a per-stage table of pointers to byte arrays (4-byte stride,
   indexed by gLinkSrcStage), each further indexed by gLinkTriggerIndex to read the
   "heading" byte passed to IsHeadingAligned. TUNNEL_EXIT_HEADINGS is the analogous
   table for gLinkDstStage/gLinkSpawnIndex. Neither array's own element type is
   dereferenced beyond a single `u8` here. */
```

```c
/* The 12-byte-stride table whose first element sits 4 bytes before the
   separately-referenced `CARDINAL_ANGLES` -- splat drew the boundary there
   because `CARDINAL_ANGLES` is independently referenced, not because the
   underlying data is two different tables. Round 66 types it
   `RotationRatios` (include/DreamSys.h) rather than as a stride-only
   placeholder: every entry is three {numerator, denominator} degree ratios
   in exactly the form SceneNode__UpdateRotation consumes, and the four entries' yaw
   numerators are 0, 0x5A, 0xB4, 0x10E -- 0, 90, 180 and 270 degrees. That is
   also what the two functions below do with an element: they store its
   ADDRESS into DreamSys::enterRotation / ::exitRotation, and the only things
   those two fields are ever used for are SceneNode__UpdateRotation(this, 1, ptr) calls
   in DreamSys__SetMoveOverride, DreamSys__SpawnAtLink and
   DreamSys__TryStaircaseLink. */
```

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Called by DreamSys__TryTunnelLink as (&this->exitRotation, &this->enterRotation, &local) --
   same `local` buffer SceneNode__GetRotationDegrees fills above; result used as a truth
   value (`beqz`), so s32 (round 2026-09-02). MATCHED, defined later in
   this unit's own ROM order -- forward declaration only (the gp-relative
   and addiu_at blockers this was once filed under are both resolved; see
   docs/match-reports/DreamSys__CheckTunnelHeading.md). */
```
