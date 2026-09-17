# DreamSys__LoadNextFlashback

**Unit:** DreamSys · **Size:** 49 words · **Status:** MATCHED (49/49)

## What it does

Already forward-declared (`bool DreamSys__LoadNextFlashback(DreamSys *this,
bool unknown);`), and its own vtable slot (`+0x1CC`) was already identified
in this round's `DreamSys__StartDay.md`. Loads the next stored flashback
entry into `this`'s current state, if one is available.

## The C

```c
bool DreamSys__LoadNextFlashback(DreamSys *this, bool unknown)
{
	s32 idx;
	FlashbackEntry *entry;

	idx = this->currentFlashbackIndex;
	if (idx >= this->amountFlashbacksAvailable) {
		goto fail;
	}
	this->unknwon_int_0x44 = 0xE;
	entry = &this->storedFlasbacks[idx];
	if (!unknown) {
		this->vt->slot30(this, 0xE);
	}
	this->currentDay = entry->day;
	this->currentStage = entry->stageID;
	this->linkCoordinates = entry->position;
	return true;
fail:
	return false;
}
```

## Evidence

- `this->currentFlashbackIndex` / `this->amountFlashbacksAvailable`: both
  already-named fields; the raw offset `+0x46C` for the latter matches
  exactly where hand-summing the struct places it (right after
  `moodPreviousDays[365]`'s 730 bytes plus 2 bytes of padding).
- `&this->storedFlasbacks[idx]`: the index arithmetic (`idx*8 + idx`, then
  `<<2`, i.e. `idx*36`) confirms `sizeof(FlashbackEntry) == 0x24` (36) --
  matches summing that struct's own fields (`4+10+4+4+4+2+4+4 = 36`).
- `entry->day` → `this->currentDay` (`+0x180`, already established from
  `DreamSys__StartDay` this round); `entry->stageID` → `this->currentStage`
  (`+0x164`, likewise already established).
- `this->linkCoordinates = entry->position;`: a whole-struct assignment of
  the 10-byte `PlayerSpawnPoint`. Retail copies it as one unaligned word
  (`lwl`/`lwr`, `swl`/`swr`) plus one halfword (`lh`/`sh`) rather than
  byte-by-byte -- the already-documented "whole-struct assignment
  reproduces retail's block-move codegen" idiom
  (`DECOMPILATION_LEARNINGS.md`), here at a non-power-of-2 size instead of
  the previously-seen 4-word-aligned case.
- `this->vt->slot30`: reuses the `BasicClass__NotifyParents` slot established
  by `ExecuteLink` earlier this round -- same table, same signature
  (`this`, literal `0xE`).

## Residue and fix

First attempt wrote the early exit as a plain `return false;` inside the
guard `if`. That compiled and linked, but scored 10/49 with a **136929-byte
"differs outside range"** warning -- the earliest and largest such count
this round, correctly read as "this function's own SIZE is wrong," not as
"nothing here matches." `asm-differ` showed exactly the already-documented
residue class from `DECOMPILATION_LEARNINGS.md`
("An early exit returning a DIFFERENT value from the main path"): a plain
`return false;` makes GCC place the exit block after the main path with an
explicit `j` back to the shared epilogue (one extra word), and the
condition sense came out inverted (`bnez` where retail has `beqz`) as a
side effect of the block reordering. Switching to
`if (cond) goto fail; ... fail: return false;` retargeted the branch
directly at the epilogue and stole the return value into its delay slot,
matching retail exactly and closing all 39 residue words at once (including
a spurious extra `move $a0, $s0` reload before the `slot30` call, which
disappeared as a side effect of the corrected register allocation rather
than needing its own fix).

## Third-learning check (per head's request)

**Not needed.** `entry` (`&this->storedFlasbacks[idx]`) is computed once
and used several times afterward, but never across an intervening `jalr` in
the FINAL structure -- the one call that does happen (`vt->slot30`) occurs
BEFORE any of `entry`'s fields are read, so there is no reload-after-call
hazard to guard against. (In the version with the goto lever mis-set, the
extra reload was of `this`/`a0`, not of `entry`, and it resolved itself once
the goto fix corrected scheduling -- not a case of this lever applying.)

## Proposed learning

**Confirms `DECOMPILATION_LEARNINGS.md`'s existing "early exit returning a
different value" lever generalizes to a `goto`-vs-`return` choice discovered
from a huge "differs outside range" byte count, not just a small
same-function residue.** The byte count (136929) was the tell that this was
a whole-function-shape problem before even reading `asm-differ`'s output --
worth checking that number BEFORE spending time reading a diff line by
line, since a large one narrows the search to "wrong control-flow shape,"
not "wrong register choice."
