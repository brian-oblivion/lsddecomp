# DreamSys__TryTunnelLink

> Renamed from `func_8005A700` on 2026-09-22 (tools/rename.py). Address 0x8005a700.

**Unit:** DreamSys · **Size:** 40 words · **Status:** MATCHED (40/40 words, full build verified byte-exact)
**Vtable slot:** `gDreamSysMethods +0x1D0` (first of a run of four previously-unnamed slots)

## Context

Same overall shape as the already-matched `DreamSys__StaticWallLink`
(guard flag, a `Test4*`-family static-link test against
`this->linkCoordinates`/`currentPos`/`this->currentStage`, `ExecuteLink` on
success) but with two extra steps folded in between the link test and
`ExecuteLink`: a call into an out-of-unit function that fills a small stack
buffer, then a call into `DreamSys__CheckTunnelHeading` (in-unit, still `INCLUDE_ASM`,
blocked) that consumes that buffer and gates the rest of the function.

`Test4TunnelLinks` is already a matched, real C function in this unit
(`s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint
*currentPos, s32 stage)`) -- this function calls it with exactly that
signature.

`SceneNode__GetRotationDegrees` is NOT in this unit at all; its body disassembles into
`asm/SceneNode.s`, an uncarved segment untouched by this round. Per
`DECOMPILATION_LEARNINGS.md` ("calling into a function that is still
`INCLUDE_ASM` elsewhere is fine"), it gets a local `extern` prototype typed
purely from this call site's register setup (`this`, `&local`), with the
usual caveat that a discarded return value proves nothing about `void`.

## The C

```c
bool DreamSys__TryTunnelLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = Test4TunnelLinks(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	SceneNode__GetRotationDegrees(this, local);
	if (!DreamSys__CheckTunnelHeading(&this->unk_0x888, &this->unk_0x884, local))
		return false;
	if (this->unk_0xA8 == 0)
		return false;
	ExecuteLink(this, result, 0xF, 0);
	return true;
}
```

Supporting header additions (`include/DreamSys.h`):

```c
extern void SceneNode__GetRotationDegrees(DreamSys *this, void *arg1);
extern s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2);
```

Two DreamSys fields retyped from raw byte arrays to `s32`, since this
function writes/reads them as whole words (previously
`s8 unknown_values_0x880[4]` / `s8 unknown_values_0x888[4]`):

```c
s32 unk_0x880;   /* written by DreamSys__TryStageTimerLink (round 2026-09-02, another
                    function in this same round -- see that report) */
s32 unk_0x888;   /* address taken here (&this->unk_0x888) and forwarded
                    to DreamSys__CheckTunnelHeading -- read-through by that (still
                    INCLUDE_ASM) callee, not by this function */
```

Also named the first of a run of four consecutive, previously-anonymous
vtable slots (`+0x1D0..+0x1DC`, all four resolve via `tools/classtable.py`
to `DreamSys__TryTunnelLink`/`DreamSys__TryStageTimerLink`/`DreamSys__TryInstantTeleportLink`/`DreamSys__TryStaircaseLink`); only
this round's own slot (`+0x1D0`) was given a real signature, the other
three stay as `u32` filler pending their own rounds:

```c
u32 unknown_functions_0x1d0[1];
bool (*DreamSys__TryTunnelLink)(DreamSys *this, PlayerSpawnPoint *currentPos);
u32 unknown_functions_0x1d8[2];
```

## Derivation notes

- The stack buffer at `sp+0x10` (`local[4]`) is sized purely from the
  frame layout: `addiu $sp,$sp,-0x30`, saved regs occupy `sp+0x20..sp+0x30`
  (`s0`,`s1`,`ra`), leaving exactly `0x10` bytes for locals/outgoing-arg
  spill at `sp+0x10..sp+0x20`. A 4-word array reproduces that frame size
  exactly; nothing in this function reads its fields individually, so its
  internal layout is unconfirmed and irrelevant to this match.
- `a1` (`currentPos`) is never reloaded before the `Test4TunnelLinks` call
  -- it is the function's own second parameter, passed straight through,
  exactly like `DreamSys__StaticWallLink`'s call to `TestForStaticLink`.

## Attempt log

Matched on the first attempt, following the `DreamSys__StaticWallLink`
template directly (same field offsets `0x164`/`0x16C`/`0x24`/`0x44`,
confirmed independently via a host-side `offsetof` walk of the whole
`DreamSys` struct treating every pointer field as 4 bytes).

## Provenance

round 2026-09-02, runner ALPHA, unit DreamSys.

## Naming

- **Tier B.** `bool(DreamSys*, PlayerSpawnPoint*)`; wraps Test4TunnelLinks and calls ExecuteLink (type 0xD) on success. One of a 4-member family (with TryStageTimerLink/TryInstantTeleportLink/TryStaircaseLink) the header's own pre-existing comment already called "three link tests", called in sequence from DreamSys__ApplyMoveCommand; also called directly from DreamSys__WallLink's own static-link path.
