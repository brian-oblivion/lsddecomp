# GetTeleportTimeBonus — MATCHED 6/6

> Renamed from `func_8005BFC4` on 2026-09-26 (tools/rename.py). Address 0x8005bfc4.

**Unit:** DreamSys · **Size:** 6 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (sole reference `gLinkSrcStage`
via `%gp_rel`). Round 42 RESOLVED that blocker with
`--gp-symbols`/`--no-nop-mflo-mfhi`. Rebuilt fresh this round and matched on
the first attempt.

## What it does

`return (sLinkSrcStage == 0) ? 0xA : 0;` -- the asm computes this via
`sltiu`/`negu`/`andi` rather than a branch (an unsigned "is zero" test turned
into an all-ones mask, then masked to `0xA`), which is exactly what GCC 2.6.3
emits for a ternary on a simple equality-to-zero test; no special shape was
needed to reproduce it.

Called by `DreamSys__TryInstantTeleportLink` (still `INCLUDE_ASM`, see that function's own
report) as `saved = GetTeleportTimeBonus();` with no arguments -- consistent with
the existing header prototype `extern s32 GetTeleportTimeBonus(void);`.

## Final body

```c
s32 GetTeleportTimeBonus(void)
{
	return (sLinkSrcStage == 0) ? 0xA : 0;
}
```

`sLinkSrcStage` was already declared `extern s32 sLinkSrcStage;` in `include/dream_sys.h`.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetTeleportTimeBonus` -> `6/6 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Naming

`GetTeleportTimeBonus` -- tier B (round 92, runner delta, FINISHING-PLAN track 7).
Renamed from `func_8005BFC4` (tier C since round 66).

Round 66 kept the placeholder because "nothing in any carved unit calls it"; that
stopped being true when `DreamSys__TryInstantTeleportLink` was matched. That
caller, after `TestForInstantTeleporters` has found a link (which leaves the
trigger's stage in `sLinkSrcStage`), calls this with no arguments and, when the
result is non-zero and the dream is not a flashback session, sets the dream time
limit to `getDreamTimerScaled() + result` -- i.e. the result is extra time, in
the same units as `DreamSys__GetSetDreamTimeLimit`, granted by the teleport. The
body gives 10 of those units when the teleporter was on stage 0 and none
otherwise. Tier B: the mechanics (a time bonus that depends on the source
stage) are established; why stage 0 alone earns it is not.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Called by DreamSys__TryInstantTeleportLink with NO arguments, same shape as GetStageLinkAngle
   above; return value is forwarded straight into ExecuteLink's stage-type
   argument, hence s32 (round 2026-09-02). MATCHED, defined later in this
   unit's own ROM order -- forward declaration only (gp-relative blocker
   resolved; see docs/match-reports/GetTeleportTimeBonus.md). */
```

## History: track 12 (round 106, charlie), comments moved out of the source

The API documentation pass moved these comments here, verbatim (commit
`2aa001d93`); the source keeps the API doc and, where a C spelling needs
it, a one-line `MATCHING:` note.

From `include/dream_sys.h`:

```c
/* Called by DreamSys__TryInstantTeleportLink with no arguments, like
   GetStageLinkAngle; its return value goes straight into ExecuteLink's
   stage-type argument. Defined after its caller. */
extern s32 GetTeleportTimeBonus(void);
```
