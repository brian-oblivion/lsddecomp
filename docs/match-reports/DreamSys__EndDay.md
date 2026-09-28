# DreamSys__EndDay

**Unit:** DreamSys · **Size:** 51 words · **Status:** MATCHED (51/51)

## What it does

Already forward-declared (`s32 DreamSys__EndDay(DreamSys *this, s32
arg1);`). Commits `storedDay` into `currentDay`; on a normal end-of-day
(`!isFlashbackSession && arg1 == 0`) runs the day-close sequence (unlock
score, chart update, day advance); on `arg1 == 2` (regardless of flashback
state) instead does a "new game" reset. Returns `isFlashbackSession`.

## The C

```c
s32 DreamSys__EndDay(DreamSys *this, s32 arg1)
{
	this->currentDay = this->storedDay;
	if (!this->isFlashbackSession && arg1 == 0) {
		this->vt->CalcUnlockScore(this);
		this->vt->UpdateDreamChart(this, &this->moodPreviousDays[this->currentDay]);
		this->vt->AdvanceDay(this);
	} else if (arg1 == 2) {
		this->vt->InitNewGame(this);
		this->unk_0x878 = 1;
	}
	return this->isFlashbackSession;
}
```

## Evidence

The raw disassembly's control flow is a chain of gotos that reuses a
register holding the literal `2` as a comparison value from TWO different
entry points (the `isFlashbackSession != 0` path and the
`isFlashbackSession == 0 && arg1 != 0` path both fall into the same `arg1
== 2` test with `$v0` pre-loaded to `2`). Tracing every path shows the
`InitNewGame`/`unk_0x878=1` block is reachable if and only if `arg1 == 2`
(independent of `isFlashbackSession`, since the ONLY path that consumes
`arg1 == 0` specifically is the day-close block, which exits before ever
reaching the `arg1 == 2` test) -- collapsing the whole thing to a plain
`if`/`else if` on that observation matched on the first attempt, no
reshaping needed.

- `this->vt->CalcUnlockScore` (`+0x210`), `->UpdateDreamChart` (`+0x1FC`,
  this round's own `DreamSys__UpdateDreamChart`), `->AdvanceDay` (`+0x1A4`,
  already matched), `->InitNewGame` (`+0x198`) -- all resolved via
  `tools/classtable.py gDreamSysMethods` before writing any C, all already
  declared with matching signatures in `include/dream_sys.h`.
- `&this->moodPreviousDays[this->currentDay]`: the index arithmetic
  (`currentDay << 1`, i.e. `*2`, then `+0x190`) confirms `moodPreviousDays`
  (a `MoodGraphPoint[365]`, 2 bytes per element) starts at `+0x190` and is
  indexed by `currentDay` -- matches the already-declared field exactly.
- `this->unk_0x878`: already-named field (used elsewhere by
  `DreamSys__ClearNewGameFlag`/`DreamSys__GetNewGameFlag` per existing header comments).

No new fields, no new vtable slots.

## Third-learning check (per head's request)

**Not needed.** No field survives an intervening `jalr` and gets read
again -- `this->currentDay` is written once up front and then only used
(not re-read after a call) as an ARGUMENT to `UpdateDreamChart`, computed
fresh from `this->currentDay` at that one call site with nothing between
its computation and use.

## Proposed learning

**A chain of `goto`s that funnels multiple entry conditions through one
shared comparison (here, `$v0` pre-loaded to a literal from two different
predecessors) is often provably equivalent to a much flatter `if`/`else
if` once every path to the shared block is traced to what it actually
requires** -- worth doing the full path trace before assuming the retail
control flow needs to be reproduced with literal `goto`s. Here the
"shared `v0==2`" trick was pure delay-slot/branch-reuse compiler
scheduling, not evidence of an actual three-way branch in the source.
