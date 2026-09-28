# DreamSys__InitMoodContributors

> Renamed from `DreamSys__InitMoodContibutors` on 2026-09-28 (tools/rename.py). Address 0x8005b2f4.

**Unit:** DreamSys · **Size:** 44 words · **Status:** MATCHED (44/44)

## What it does

Already forward-declared (`void DreamSys__InitMoodContributors(DreamSys
*this, MoodGraphPoint *special);`). Clears both mood contributors, then --
only if a `special` mood point is supplied -- logs it into both.

## The C

```c
void DreamSys__InitMoodContributors(DreamSys *this, MoodGraphPoint *special)
{
	this->vt->ClearMoodGraph(this, &this->areaMoods);
	this->vt->ClearMoodGraph(this, &this->entityMoods);
	if (special != NULL) {
		this->vt->LogMood(this, &this->areaMoods, special);
		this->vt->LogMood(this, &this->entityMoods, special);
	}
}
```

## Evidence

- `&this->areaMoods` / `&this->entityMoods`: `addiu $s2, $s0, 0x144` /
  `addiu $s3, $s0, 0x154` -- exactly the offsets already established for
  these two fields (`MoodGraphContributor`, 0x10 bytes each, so
  0x144+0x10=0x154 checks out).
- `this->vt->ClearMoodGraph`: vtable slot `+0x204`, already named and typed
  by `DreamSys__ClearMoodGraph.md` (matched an earlier round).
- `this->vt->LogMood`: vtable slot `+0x208`, immediately after
  `ClearMoodGraph` in both the disassembly (`0x204` then `0x208`) and the
  already-declared `vtable_DreamSys` struct order in
  `include/dream_sys.h` -- no new typing needed, just confirms the existing
  declaration's slot position is right.

No new fields, no new slots. Matched on the first attempt.

## Third-learning check (per head's request)

**Not applicable.** No field is read, survives a `jalr`, and is read again
-- `this` itself is the only thing that crosses the four calls, and it's a
parameter (never reloaded from memory, no aliasing concern). No locals
needed beyond the parameters themselves.

## Proposed learning

None -- straightforward confirmation that `ClearMoodGraph`/`LogMood` are
adjacent vtable slots, consistent with their already-recorded offsets.
