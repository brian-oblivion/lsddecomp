# DreamSys__InitNewGame — MATCHED 30/30

**Unit:** DreamSys · **Size:** 30 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (`SAVE_MAGIC`). Round 42
RESOLVED that blocker. Rebuilt fresh this round and matched on the first
attempt.

## What it does

Resets a "new game" batch of `DreamSys` fields to zero, seeds
`unknown_sdata_0x178` from a `.sdata` constant, sets `screenShakeOn = 1`,
calls `InitNavChallengesArray` (matched earlier this round) on
`&this->navChallengesArray`/`&this->amountDynamicLinksDone`, and `memset`s a
500-byte tail region (`unknown_values_0x684`) to zero.

Field offsets were verified against the struct layout with a host-side
`offsetof()` probe compiled `-m32` (matching the target's 4-byte pointers;
compiling without `-m32` gives 8-byte-pointer offsets that do NOT match the
struct's own offset-encoding names) -- every field touched here
(`currentYear`, `currentDay`, `totalFlasbackUnlockScore`,
`navigationFlasbackUnlockScore`, `instanceFlasbackUnlockScore`,
`amountFlashbacksAvailable`, `unknown_values_0x5d8`, `screenShakeOn`,
`unknown_word_0x67c`, `unknown_word_0x680`, `unknown_values_0x684`) landed
exactly at the byte offset its name already encodes, confirming the
project's `_0xNNN` naming convention is reliable for this struct.

## Final body

```c
extern void *memset(unsigned char *dst, unsigned char c, int n);
extern s32 sSaveMagic;

void DreamSys__InitNewGame(DreamSys *this)
{
	this->unknown_sdata_0x178 = sSaveMagic;
	this->currentYear = 0;
	this->currentDay = 0;
	this->totalFlasbackUnlockScore = 0;
	this->navigationFlasbackUnlockScore = 0;
	this->instanceFlasbackUnlockScore = 0;
	this->amountFlashbacksAvailable = 0;
	this->unknown_values_0x5d8[7] = 0;
	this->unknown_values_0x5d8[0] = 0;
	this->screenShakeOn = 1;
	this->unknown_word_0x67c = 0;
	this->unknown_word_0x680 = 0;
	InitNavChallengesArray(&this->navChallengesArray, &this->amountDynamicLinksDone);
	memset((unsigned char *)&this->unknown_values_0x684, 0, 0x1F4);
}
```

`memset`'s extern declaration copies `ScreenWidgets.c`'s own local view
verbatim (`extern void *memset(unsigned char *dst, unsigned char c, int
n);`) -- it is a Psy-Q/libc function, uncarved (`psyq_memset.s`), so per
project convention this is a unit-local prototype, not something added to a
shared header.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py DreamSys__InitNewGame` -> `30/30 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Comment moved from src/world/dream_sys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* A constant read out of .sdata and copied whole into
   this->saveMagic -- naming convention matches (the field's own
   name already flags it as sdata-sourced). Not dereferenced by this
   function or any other in this unit's queue. */
```
