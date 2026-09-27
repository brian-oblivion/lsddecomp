# DreamSys__ApplyMoveCommand

> Renamed from `func_80059E98` on 2026-09-22 (tools/rename.py). Address 0x80059e98.

**Unit:** DreamSys · **Size:** 110 words · **Status:** MATCHED, 110/110.

## What it does

Called by `DreamSys__TickMoveFree`/`DreamSys__TickMoveForced` as
`this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1))` -- `arg1`
is a mood/day-type selector. When `arg1 == 0` the function does nothing at
all (matches retail's own early `beqz` straight to the epilogue -- no `$v0`
write on that path, consistent with the function's return value being
meaningless there and never inspected by its only two callers on that
branch). Otherwise:

```c
s32 DreamSys__ApplyMoveCommand(DreamSys *this, s32 arg1)
{
	s32 delta;
	PlayerSpawnPoint *pos;

	if (arg1 != 0) {
		delta = MOVE_COMMAND_SIGNS[arg1] * MOVE_MODE_SPEEDS[this->unk_0xAC];
		this->vt->DreamSys__NoOpSlot12C(this);
		pos = this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0);
		if (!this->vt->DreamSys__TryStaircaseLink(this, pos)
		 && !this->vt->DreamSys__TryInstantTeleportLink(this, pos)
		 && !this->vt->DreamSys__TryTunnelLink(this, pos)) {
			this->vt->DreamSys__SaveLinkSnapshot(this);
			MOVE_COMMAND_DISPATCH[arg1](this, delta, (void *)(this->unk_0x90C < 1));
			if (this->currentStage == 0
			 && this->unk_0x14->unk_0x1C < -0x7D0
			 && this->unk_0x14->unk_0x18 >= -0x1F3) {
				this->vt->LinkWall(this, this, 4);
			}
		}
		this->unk_0x14->unk_0x0 = 0;
	}
}
```

`delta` is a signed value (`MOVE_COMMAND_SIGNS[arg1]`, a small `{0,1,-1}` sign/step
table, times `MOVE_MODE_SPEEDS[this->unk_0xAC]`, a `{0,0x18,0x40,0x80,0x180}`
magnitude table indexed by the OTHER "current mood class" field), forwarded
into `MOVE_COMMAND_DISPATCH[arg1]` -- a genuine dispatch table of function pointers
(`MOVE_COMMAND_DISPATCH[0]` is null; unreachable here since `arg1 == 0` already
returned). `pos` comes from `this->unk_0x4C->methods->slot0x10C`, already
typed `PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2)` --
matches the three "link test" calls that follow, which try
`DreamSys__TryStaircaseLink`/`DreamSys__TryInstantTeleportLink`/`DreamSys__TryTunnelLink` in that (reverse-address)
order, short-circuiting on the first one that returns nonzero. Only if all
three fail does it snapshot `*this->unk_0x14` (`DreamSys__SaveLinkSnapshot`, already
matched, vtable +0x220) and run the dispatch call, then conditionally
`LinkWall`.

## New struct/vtable knowledge committed alongside this round

All in `include/DreamSys.h`:

- **`vtable_DreamSys` gains three real slots that were placeholder
  `u32 unknown_functions_0x1d0[1]` / `u32 unknown_functions_0x1d8[2]`**:
  `DreamSys__TryTunnelLink` (+0x1D0, confirmed already-matched signature `bool
  (DreamSys *this, PlayerSpawnPoint *currentPos)` -- its OWN definition in
  `src/DreamSys.c` gave the exact type), `DreamSys__TryInstantTeleportLink` (+0x1D8) and
  `DreamSys__TryStaircaseLink` (+0x1DC), the latter two still `INCLUDE_ASM` but typed
  identically to `DreamSys__TryTunnelLink`/`DreamSys__TryStageTimerLink` on the strength of this
  call site alone (same argument shape, same short-circuit pattern used
  three times in a row). No layout change -- 1 `u32`/2 `u32` become
  1/2 function pointers, same total bytes.
- **`DreamSysUnk14`'s opaque `unknown_values_0x4[0x34/4]` blob split** to
  name `unk_0x18` and `unk_0x1C` (both `s32`), the two fields this function
  reads and bounds-checks. Total size unchanged (13 words before and after);
  `DreamSys__SaveLinkSnapshot`/`DreamSys__RestoreLinkSnapshot`'s existing whole-struct block-copy of this
  same type was re-verified green after the split (the whole-image SHA1
  passed with both functions still matched).
- **`DreamSys::unk_0x90C` retyped `s32` -> `u32`.** Retail compares it with
  `sltiu` (unsigned), not `slti`; both existing writers (round 2026-08-30)
  only ever set it to the literal 0, so the retype is safe.
- **`extern s32 MOVE_MODE_SPEEDS[5]`, `extern s8 MOVE_COMMAND_SIGNS[8]`, and
  `extern void (*MOVE_COMMAND_DISPATCH[5])(DreamSys *this, s32 val, void *extra)`**
  (the last declared after the real `DreamSys` typedef, matching
  `Actor__MoveLocalZOrFindLink`/`Actor__MoveLocalXOrFindLink`'s element signature -- those are two of
  MOVE_COMMAND_DISPATCH's five entries).
- **`this->unk_0x164` turned out to already be a named field**: offset
  arithmetic through `unknown_values_0x138[12]` + two `MoodGraphContributor`
  members (`areaMoods`, `entityMoods`, each `sizeof` 0x10) lands exactly on
  `currentStage` at +0x164 -- no header edit needed there, just using the
  existing name instead of inventing an `unk_0x164`.

## Notes

- **The early-return path has no explicit `return` statement** (the function
  falls off the end for `arg1 == 0`, matching retail's own lack of any `$v0`
  write on that path) -- GCC 2.6.3 warns `control reaches end of non-void
  function` but compiles it identically to retail. Both callers only use
  the return value when they got it via the `DreamSys__AdvanceMoveCycle(this, 1)`
  argument path, which per the vtable's own existing comments never passes
  a literal 0 for `arg1`, so the undefined-value path is never actually
  observed by real callers -- consistent with retail not bothering to set
  it up.
- **One real residue during matching**: `this->unk_0x90C < 1` initially
  compiled to `slti` (signed) against retail's `sltiu` (unsigned) because
  the field was declared `s32`. Retyping to `u32` (justified above) closed
  it in one step -- this is the "a wrong TYPE produces register/instruction-
  shaped symptoms" pattern from `DECOMPILATION_LEARNINGS.md`, just showing
  up as a compare-instruction choice rather than a register choice.
- **`vt->LinkWall(this, this, 4)`**: the second argument is genuinely `this`
  itself (not a distinct `PlayerSpawnPoint`/pos value) -- confirmed byte-for-
  byte, an `addu $a1,$s0,$zero` right before the call, `$s0` being `this`
  throughout this function. `LinkWall`'s own signature (`void *arg1`)
  already accommodates an arbitrary pointer here.

### Proposed learning

Confirms `DECOMPILATION_LEARNINGS.md`'s existing "a wrong type produces
register/instruction-shaped symptoms" entry with a new concrete instance:
an unsigned-vs-signed COMPARE INSTRUCTION CHOICE (`sltiu` vs `slti`), not
just register allocation, can hinge on a shared struct field's declared
signedness -- worth checking any `< 1`/`>= 0`-shaped residue against the
field's sign before assuming it is a control-flow or register problem.

## Naming

`DreamSys__ApplyMoveCommand` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059E98`.

Given the move command `DreamSys__AdvanceMoveCycle` returned,
scales it into a delta (`MOVE_COMMAND_SIGNS[cmd] * MOVE_MODE_SPEEDS[moveMode]`),
fetches the current spawn point from `linkMgr`, and tries the three link tests
(staircase, instant teleporter, tunnel) BEFORE moving. Only if none of them fires
does it save the link snapshot and call the mover
`MOVE_COMMAND_DISPATCH[cmd](this, delta, ...)`, followed by a stage-0-only wall-link
check on two bounds of `unk_0x14`. "ApplyMoveCommand" names the whole of that: the
command is what it takes, and applying it may mean linking instead of moving.

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
