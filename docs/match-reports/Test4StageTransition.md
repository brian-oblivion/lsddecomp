# Test4StageTransition — MATCHED 46/46

> Renamed from `func_8005BE90` on 2026-09-22 (tools/rename.py). Address 0x8005be90.

**Unit:** DreamSys · **Size:** 46 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (two references,
`STAGE5_TRIGGER_GRIDPOS` and `gLinkDstStage`). Round 42 RESOLVED that blocker. This was the
hardest function in the round's queue -- 5 attempts across a real control-flow
puzzle before reaching byte-exact.

## What it does

A "link type" dispatcher: for `stage` in `{1, 3, 9, 0xC}` (falling through a
shared body with an extra check when `stage == 9`), or `stage == 5` (its own
distinct check), validate `currentPos` against a couple of conditions and
then call `GetRandomSpawnFromStage(target, (timer & 1) ? -0xC : stage,
timer)`, stashing the result in `gLinkDstStage`. Any other `stage` value
returns `-1` immediately. Called by `DreamSys__TryStageTimerLink` (still `INCLUDE_ASM`) as
`Test4StageTransition(&this->linkCoordinates, this->currentStage, currentPos,
this->dreamTimer)`.

## Attempts 1-4: right VALUES, wrong control-flow SHAPE

**Attempt 1, `switch` statement (4/46, 349495 bytes drift).** GCC 2.6.3 -O2
lowers a `switch` over case values spanning `1`-`0xC` to a JUMP TABLE
(bounds check + indexed load + `jr`), verified in isolation with a minimal
reproducer through the pinned pipeline -- completely different machine code
shape from retail's plain sequential compares. Confirmed: retail does NOT
use a switch here.

**Attempt 2, `if (A||B||C||D) {...} else if (E) {...} else return -1;`
(4/46, then 10/46 after tightening the inner conditions, ~131500 bytes
drift).** This reproduces retail's VALUES and gets close on instruction
count but the compiled length is consistently 1 word off from retail's 46,
because the last operand of the `||` chain falls through into the merged
body directly, and GCC schedules a piece of that body's first instruction
into the fallthrough branch's delay slot AND re-emits it at the jump target
(needed by the other three paths) -- a genuine duplicate `li v0,5`. Retail's
own delay slot at that exact branch holds the `-1` return-value setup
instead, meaning retail's SOURCE was not one big compound boolean at all.

**Attempts 3-4, explicit `goto`-chain mirroring retail's branch-by-branch
dispatch, correct LENGTH (46 words) but wrong BLOCK ORDER (16/46 then
27/46).** Splitting the dispatcher into five separate `if (stage == X) goto
...;` statements (one per case value, in retail's own tested order 3, 1, 5,
9, 0xC) reproduced the dispatcher exactly and fixed the length. What was
still wrong: the `case 5`-specific code block was written to appear
TEXTUALLY AFTER the shared `case 9` check in the source, so the compiler
laid it out physically LAST in the function. Retail's actual layout is the
opposite: the `case 5` body (retail's `.L8005BED0`) sits immediately after
the "is it 5" fallthrough of the shared block, with the `case 9` check
(retail's `.L8005BF00`) placed AFTER it -- reached only via a real jump. A
branch's ENCODED target offset is baked into its bytes, so getting the VALUES
right at the wrong physical block order still fails byte comparison even
once total length matches.

## Final body: same goto structure, blocks reordered to retail's physical layout (46/46)

```c
extern s32 sStage5TriggerGridPos;

s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer)
{
	s32 result;

	if (stage == 3)
		goto shared;
	if (stage == 1)
		goto shared;
	if (stage == 5)
		goto case5;
	if (stage == 9)
		goto shared;
	if (stage != 0xC)
		return -1;

shared:
	if (stage != 5)
		goto case9check;
case5:
	if (currentPos->position.y < -0xFFF)
		goto merge;
	if (*(s32 *)currentPos == sStage5TriggerGridPos)
		goto merge;
	return -1;

case9check:
	if (stage != 9)
		goto merge;
	if (currentPos->position.y < 0x800)
		return -1;

merge:
	if (timer & 1)
		stage = -0xC;
	result = GetRandomSpawnFromStage(target, stage, timer);
	gLinkDstStage = result;
	return result;
}
```

Two levers closed the last two gaps:

1. **`case5`'s block had to be textually placed immediately after the
   `shared:` label's own test**, not after `case9check`, so the compiler's
   natural fallthrough-follows-source-order layout puts it where retail has
   it. The dispatcher's `if (stage == 5) goto case5;` then jumps into the
   SAME fallthrough address the `shared:` block reaches on `stage == 5`,
   exactly reproducing retail's direct top-level jump into the middle of
   what looks like "shared" code.
2. **The final call's stage argument had to be a conditional REASSIGNMENT of
   the existing `stage` parameter (`if (timer & 1) stage = -0xC;`), not a
   ternary assigned to a fresh result variable.** A ternary
   (`(timer&1)?-0xC:stage`) computes into a NEW register in both branches and
   then copies it into the argument register -- one extra `move`. Retail
   leaves the argument alone unless the condition is true, costing one
   fewer instruction; this is the same class of lesson as
   `GetStageLinkAngle`'s "two returns vs. default-then-override" fix earlier this
   round, but for a call ARGUMENT rather than a return value.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py Test4StageTransition` -> `46/46 words match`.

### Proposed learning

For a dispatcher where several case values share a body but one case has
unique code PHYSICALLY INTERLEAVED between two pieces of the shared body
(retail here: shared-test, unique-case-5-body, shared-case-9-check, in that
address order) -- a `goto`-based translation only reproduces the bytes when
the GOTO TARGETS are placed in the SAME physical source order as retail's
blocks, not in whatever order reads most naturally. Getting every VALUE and
even the overall instruction COUNT right is not sufficient once a branch's
target offset is baked into the encoding; the block order matters
independently and must be checked by placement, not just by total length.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Naming

- **Tier B.** Same family shape as the already-named TestForStaticLink/TestForTunnelLinks/Test4StaircaseNodes/Test4InstantTeleporters (a PlayerSpawnPoint-in, stage-out test used by DreamSys__TryStageTimerLink), but unlike its siblings it does not consult a trigger table -- it applies stage/position/timer-parity rules directly and always produces a spawn via GetRandomSpawnFromStage when they hold.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Called by DreamSys__TryStageTimerLink as (&this->linkCoordinates, this->currentStage,
   currentPos, this->tick); result compared with `bltz` exactly like
   TestForStaticLink's call site, so s32 (round 2026-09-02). MATCHED, defined
   later in this unit's own ROM order -- this is a forward declaration, not a
   cross-unit prototype (the gp-relative blocker this was once filed under is
   resolved; see docs/match-reports/Test4StageTransition.md). */
```
