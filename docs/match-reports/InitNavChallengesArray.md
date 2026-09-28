# InitNavChallengesArray — MATCHED 11/11

**Unit:** DreamSys · **Size:** 11 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (`gpNavChallengesComplete`,
via `%gp_rel`). Round 42 RESOLVED that blocker. Rebuilt fresh this round and
matched on the first attempt.

## What it does

Zeroes the 30-byte navigation-challenges array (backwards, index 29 down to
0 -- a `bgez`-terminated countdown loop, matching a `for (i = 29; i >= 0;
i--)` written in that exact direction rather than a forward loop or a
`memset` call), then publishes the array pointer and a link-penalty counter
into two global pointers, zeroing the counter first.

## Final body

```c
void InitNavChallengesArray(s8 (*arrayMem)[30], s32 *linkCounter)
{
	s32 i;

	for (i = 29; i >= 0; i--)
		(*arrayMem)[i] = 0;
	gpNavChallengesComplete = arrayMem;
	*linkCounter = 0;
	gpDinamicLinkPenalty = linkCounter;
}
```

All types (`arrayMem`, `linkCounter`, `gpNavChallengesComplete`,
`gpDinamicLinkPenalty`) were already declared in `include/dream_sys.h` from
earlier rounds' call-site analysis; nothing new to declare.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py InitNavChallengesArray` -> `11/11 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.
