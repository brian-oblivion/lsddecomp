# DreamSys__CheckStaircaseHeading — MATCHED 59/59

> Renamed from `func_8005C02C` on 2026-09-22 (tools/rename.py). Address 0x8005c02c.

**Unit:** DreamSys · **Size:** 59 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (4 references, the
first to `gLinkSrcStage`) AND `addiu_at` (2 runtime-indexed global loads
through `STAIRCASE_ENTER_HEADINGS`/`STAIRCASE_EXIT_HEADINGS`). Round 21 resolved `addiu_at`; round 42
resolved `gp_rel`. Attempted fresh this round (a stretch pick beyond the
assigned queue).

## What it does

Byte-for-byte the SAME shape as `DreamSys__CheckTunnelHeading` (matched earlier this
round) -- literally identical instruction sequence, just against two
DIFFERENT per-stage tables (`STAIRCASE_ENTER_HEADINGS`/`STAIRCASE_EXIT_HEADINGS` instead of
`TUNNEL_ENTER_HEADINGS`/`TUNNEL_EXIT_HEADINGS`), and used by a different caller. Called by
`DreamSys__TryStaircaseLink` (still `INCLUDE_ASM`) as `DreamSys__CheckStaircaseHeading(&this->unk_0x888,
&this->unk_0x884, local)` -- the header's own comment already flagged this
("identical call shape to DreamSys__CheckTunnelHeading above (same `local` buffer, same
two `this` fields)").

## Final body (59/59 on the first attempt)

```c
extern u8 *STAIRCASE_ENTER_HEADINGS[];
extern u8 *STAIRCASE_EXIT_HEADINGS[];

s32 DreamSys__CheckStaircaseHeading(s32 *arg0, s32 *arg1, void *arg2)
{
	u8 heading;
	s32 idx;
	s32 result;

	heading = STAIRCASE_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
	if (IsHeadingAligned((DirectionCheckArg *)arg2, heading)) {
		if (arg1 != NULL)
			*arg1 = (s32)&CARDINAL_ROTATIONS[heading];

		if (arg0 != NULL) {
			idx = STAIRCASE_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
			*arg0 = (s32)&CARDINAL_ROTATIONS[idx];
		}
		result = 1;
	} else {
		result = 0;
	}
	return result;
}
```

Copied `DreamSys__CheckTunnelHeading`'s FINAL (already-matched) body verbatim, substituting
the two table names -- including the `if/else`-with-shared-`result`
shape that closed `DreamSys__CheckTunnelHeading`'s own one-word "exit block layout" residue.
Matched on the very first attempt, confirming that residue was a genuine
property of the CONTROL-FLOW SHAPE (early-return guard vs. if/else), not
something specific to `DreamSys__CheckTunnelHeading`'s own tables or caller.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py DreamSys__CheckStaircaseHeading` -> `59/59 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys (stretch pick beyond the assigned
queue).

## Naming

- **Tier B.** Near-identical body to DreamSys__CheckTunnelHeading, differing only in which per-stage heading table it indexes (STAIRCASE_ENTER_HEADINGS/STAIRCASE_EXIT_HEADINGS here); called from DreamSys__TryStaircaseLink.
