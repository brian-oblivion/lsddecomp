# DreamSys__GetSetFlashbackSession

> Renamed from `func_800590E8` on 2026-09-22 (tools/rename.py). Address 0x800590e8.

**Unit:** DreamSys · **Size:** 24 instructions · **Status:** MATCHED (24/24 words)

## What it does

Vtable slot `+0x0F0`. A get/set on `isFlashbackSession` (offset `0x68`,
already named) with a color-recompute fallback: if `value < 0`, computes
`CalcDreamColor(&this->moodPreviousDays[this->currentDay])` and writes it
through `out`; otherwise stores `value` into `isFlashbackSession`. Always
returns the OLD value of `isFlashbackSession`.

Resolving `this->0x180` (a raw offset in the disassembly, `lw $a0,
0x180($v0)`) to `this->currentDay` -- and the following `sll $a0,$a0,1;
addiu $a0,$a0,0x190` to `&this->moodPreviousDays[currentDay]` -- came from a
host `-m32` `offsetof` build of the struct as currently documented in the
header: `currentDay` lands at `0x180` and `moodPreviousDays` at `0x190`
exactly, and `MoodGraphPoint` (from `common.h`) is a 2-byte union, matching
the `sll ...,1` (×2) stride. `CalcDreamColor(MoodGraphPoint *mood)` was
already declared in the header from an earlier round, which confirmed the
argument type independently.

## The C

```c
s32 DreamSys__GetSetFlashbackSession(DreamSys *this, DreamColors *out, s32 value)
{
	s32 old;

	old = this->isFlashbackSession;
	if (value < 0) {
		*out = CalcDreamColor(&this->moodPreviousDays[this->currentDay]);
	} else {
		this->isFlashbackSession = value;
	}
	return old;
}
```

## Residue note: the `if`/`else` ORDER mattered, not just the condition

Writing this the "obvious" way first -- `if (value >= 0) { set } else {
compute }` -- built to a COMPLETELY different, much larger control-flow
shape (funcdiff showed dozens of words differing from word 6 onward, not a
localized residue). Retail's actual layout has the `value >= 0` case reached
via a taken branch (`bgez`) and the `value < 0` / color-compute case as the
fallthrough. Swapping to `if (value < 0) { compute } else { set }` --
literally matching retail's branch/fallthrough assignment -- fixed it to a
byte-exact match on the first try. Read the branch TARGETS before guessing
which arm is "the if"; this is the same category of trap
`DECOMPILATION_LEARNINGS.md` already documents for cross-jump/tail-merge
cases, just for plain if/else this time.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

`DreamSys__GetSetFlashbackSession` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_800590E8`.

The unit's established get/set shape (`DreamSys__GetSetDreamTimeLimit`,
`DreamSys__GetSetMoveMode`, `DreamSys__GetSetScreenShake`): a negative `value` means
query only. It always returns the OLD `isFlashbackSession`, and on the query path it
additionally writes today's dream colour
(`CalcDreamColor(&this->moodPreviousDays[this->currentDay])`) through `out`.
Tier B, and the name is deliberately incomplete: it says nothing about the colour
readout, because nothing establishes why a caller asking "am I in a flashback"
should also be told today's colour. That side effect is documented rather than
named.
