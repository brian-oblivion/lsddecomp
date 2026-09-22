# DreamSys__TryStageTimerLink

> Renamed from `func_8005A7A0` on 2026-09-22 (tools/rename.py). Address 0x8005a7a0.

**Unit:** DreamSys · **Size:** 35 words · **Status:** MATCHED (35/35 words, full build verified byte-exact)
**Vtable slot:** `DREAMSYS_METHODS +0x1D4` (second of the four-slot run; see DreamSys__TryTunnelLink.md)

## Context

Sibling of `DreamSys__TryTunnelLink` (same round, immediately preceding this function
in ROM order) and `DreamSys__StaticWallLink`, but using yet another link
test: `Test4StageTransition` (in-unit, still `INCLUDE_ASM`, gp-relative-blocked),
which takes FOUR arguments -- `linkCoordinates`, `currentStage`,
`currentPos`, AND `dreamTimer` -- one more than `TestForStaticLink`'s three.
On success it also calls `GetStageLinkAngle()` (niladic, still `INCLUDE_ASM`,
also gp-relative-blocked) and stores its return alongside clearing two
adjacent fields, before `ExecuteLink`ing with literal type `0x10`.

## The C

```c
bool DreamSys__TryStageTimerLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = Test4StageTransition(&this->linkCoordinates, this->currentStage, currentPos, this->dreamTimer);
	if (result < 0)
		return false;
	this->unk_0x880 = GetStageLinkAngle();
	this->unk_0x884 = 0;
	this->unk_0x888 = 0;
	ExecuteLink(this, result, 0x10, 0);
	return true;
}
```

Header additions (`include/DreamSys.h`):

```c
extern s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer);
extern s32 GetStageLinkAngle(void);
```

(The `unk_0x880`/`unk_0x884`/`unk_0x888` retyping is shared with
`DreamSys__TryTunnelLink`'s report -- both functions in this round touch the same two
fields, `unk_0x880` is written here for the first time.)

## Derivation notes

- `Test4StageTransition`'s return is stored in a callee-saved register across the
  call to `GetStageLinkAngle()`, since `result` (the link-test outcome) is
  still needed afterward as `ExecuteLink`'s second argument. Writing this as
  ordinary sequential C (`result = ...; if (...) return false; this->unk_0x880
  = GetStageLinkAngle(); ...; ExecuteLink(this, result, ...)`) reproduces this for
  free -- GCC keeps `result` live across the intervening call because it is
  read again afterward; no explicit save/restore needed in source.
- Matched on the first attempt; no residue.

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.

## Naming

- **Tier B.** Wraps Test4StageTransition and calls ExecuteLink (type 0x10) on success; same family as DreamSys__TryTunnelLink.
