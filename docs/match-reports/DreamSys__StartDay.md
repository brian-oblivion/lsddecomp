# DreamSys__StartDay

**Unit:** DreamSys · **Size:** 45 words · **Status:** MATCHED (45/45)

## What it does

Already forward-declared (`s32 DreamSys__StartDay(DreamSys *this);`).
Resets per-day bookkeeping, then branches on whether this is a flashback
session: if so, loads the next flashback; otherwise checks whether the
upcoming day is special and sets up mood contributors accordingly, bailing
out early (`-1`) on a special day before spawn placement runs. Returns the
current stage on the normal path.

## The C

```c
s32 DreamSys__StartDay(DreamSys *this)
{
	s32 oldDay;
	MoodGraphPoint *special;

	oldDay = this->currentDay;
	this->currentFlashbackIndex = 0;
	this->dreamTimer = 0;
	this->storedDay = oldDay;
	if (this->isFlashbackSession) {
		this->vt->LoadNextFlashback(this, 1);
	} else {
		special = IsDaySpecial(&this->nextCinematic, this->currentDay + 1);
		this->vt->InitMoodContibutors(this, special);
		if (special != NULL) {
			return -1;
		}
		this->vt->InitSpawnLoc(this);
	}
	return this->currentStage;
}
```

## No new struct/vtable knowledge needed

Every raw offset this function touches turned out to already be named:

- `+0x87C` → `currentFlashbackIndex` (already established).
- `+0x88C` → `storedDay` (already established; matches the disassembly's
  unconditional store of the OLD `currentDay` value here, in the delay slot
  of the `isFlashbackSession` test -- it runs on both paths).
- `+0x1CC` (vtable) → `LoadNextFlashback`. Derived by counting vtable slots
  forward from `StaticWallLink` (`+0x1C8`, already known from its own
  report) through `LoadNextFlashback`, landing exactly on `+0x1CC` --
  confirms the queued `DreamSys__LoadNextFlashback`'s signature
  (`bool (*)(DreamSys*, bool)`) matches this call site's `(this, 1)`.
- `+0x1C0` (vtable) → `InitSpawnLoc`, by the same backward count from
  `DynamicLink` (`+0x1C4`, known from its report).
- `+0x1F0` (vtable) → `InitMoodContibutors` -- this unit's OWN
  just-matched function, called back through the vtable with the
  `IsDaySpecial` result as its `special` argument. Counted forward from
  `DreamSys__TryStageTimerLink` (`+0x1D4`, confirmed via `classtable.py` in that
  function's own report) through the five slots between it and
  `InitMoodContibutors`.

No header edits at all this function -- purely a slot-counting exercise
against already-established neighbors, following the guide's advice to
read neighboring reports before attempting a function.

## Third-learning check (per head's request)

**Not needed.** `oldDay` (`this->currentDay`, read once at the top) is used
again after the `LoadNextFlashback`/`InitMoodContibutors` calls only via
`this->storedDay = oldDay`, which happens BEFORE either call (in program
order, matched by the delay-slot scheduling) -- so it never crosses a
`jalr`. `special` (the `IsDaySpecial` return) is naturally a local already
since its value is used in two later statements, but nothing here required
reasoning about aliasing across a call boundary the way `TaskCoreObj`/
`TaskCoreObjMethods` did in the previous unit.

## Proposed learning

**Vtable slot arithmetic from a KNOWN neighbor is cheap and precise in a
dense, heavily-worked unit like DreamSys.** Three slots this function needed
were resolved by counting struct-declaration lines between two
already-established anchors (no `classtable.py` run required, though it
remains the authoritative check) -- worth doing before writing any header
code, since it can turn what looks like "three new slots to model" into
"zero, all already named."
