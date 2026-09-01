# DreamSys__InitMoodContibutors -- MATCHED (44/44 words)

Unit: `DreamSys`. Round 2026-09-01-e (runner echo).

## Final C

```c
void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special)
{
	this->vt->ClearMoodGraph(this, &this->areaMoods);
	this->vt->ClearMoodGraph(this, &this->entityMoods);
	if (special != NULL) {
		this->vt->LogMood(this, &this->areaMoods, special);
		this->vt->LogMood(this, &this->entityMoods, special);
	}
}
```

## Derivation

Straightforward once the two vtable slots the body dispatches through were
resolved: `+0x204` is `ClearMoodGraph` (already matched,
`DreamSys__ClearMoodGraph`) and `+0x208` is `LogMood` (still `INCLUDE_ASM`,
but its prototype was already in `include/DreamSys.h` from an earlier
round). Both dispatches are through THIS function's own `this->vt`, not a
different class's table -- straight passthrough calls on `areaMoods` /
`entityMoods`, guarded once by `special != NULL` for the two `LogMood`
calls. Matched on the first attempt; no residue.

## Proposed learning

None beyond what is already documented; this one was a clean vtable-slot
resolution with no residue.
