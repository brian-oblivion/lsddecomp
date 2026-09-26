# GetTeleportTimeBonus — MATCHED 6/6

> Renamed from `func_8005BFC4` on 2026-09-26 (tools/rename.py). Address 0x8005bfc4.

**Unit:** DreamSys · **Size:** 6 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (sole reference `gLinkSrcStage`
via `%gp_rel`). Round 42 RESOLVED that blocker with
`--gp-symbols`/`--no-nop-mflo-mfhi`. Rebuilt fresh this round and matched on
the first attempt.

## What it does

`return (gLinkSrcStage == 0) ? 0xA : 0;` -- the asm computes this via
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
	return (gLinkSrcStage == 0) ? 0xA : 0;
}
```

`gLinkSrcStage` was already declared `extern s32 gLinkSrcStage;` in `include/DreamSys.h`.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetTeleportTimeBonus` -> `6/6 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Naming

`GetTeleportTimeBonus` -- tier C (round 66, runner alpha, FINISHING-PLAN track 3).

NOT RENAMED, and this records why. It is a free function whose
whole body is `return (gLinkSrcStage == 0) ? 0xA : 0;` -- gLinkSrcStage being the
source stage `GetStaticSpawn` records for the trigger it matched. Nothing in any
carved unit calls it and it is not a gDreamSysMethods slot, so there is no call site
to type the return value, and 0xA on its own names nothing. The tier-C
`Class__func_xxxxx` form does not apply either: it is not a method, so it keeps the
bare placeholder.
