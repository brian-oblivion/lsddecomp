# DreamSys__ResetLinkState — MATCHED

> Renamed from `func_800591B4` on 2026-09-22 (tools/rename.py). Address 0x800591b4.

**Unit:** DreamSys · **Size:** 87 words (94 instructions incl. delay
slots/nops, `0x499B4`-`0x49B10`) · **Status:** MATCHED (87/87 words),
whole-image SHA1 verified green by `build-and-verify.sh`. Round 2026-09-02,
runner BRAVO. Matched on the first attempt — no residue.

## What it does

Vtable slot `+0x0F8`. A straight-line "start dream" initializer, no
branches at all: six vtable calls (`LogChunkMood`, `DreamSys__SelectCallback80`,
`DreamSys__SelectCallback98`, `func_8005A168`, `func_8005A1B0`, `func_8005A1EC`), a large
block of per-dream state zeroed in between/after, and a closing
`Class6B5CC__GetRotationDegrees`/`func_8001CEB4` pair over a small local buffer — the same
`func_8001CEB4(this, 1, &local)` shape already established by
`DreamSys__GetSetDreamTimeLimit` a few functions earlier in this unit.

Every vtable slot and struct field this function touches was ALREADY named
by earlier rounds; the only new things this round contributes are the
function's own slot and a 12-byte local buffer's two known `s16` sub-fields.

Both blocker screens are clean: no `gp_rel` hit and no
`addiu $at, $at, %lo` hit anywhere in `DreamSys__ResetLinkState.s`.

## New vtable knowledge

- `vtable_DreamSys::DreamSys__ResetLinkState` (was inside `unknown_functions_0xec[4]`)
  — this function's own slot, `+0x0F8`, split out of that array (now
  `unknown_functions_0xec[3]`, covering `0xEC`/`0xF0`/`0xF4`).

No struct fields needed new names — this function happened to touch only
already-carved territory (`linkCoordinates`, `nextCinematic.entry`,
`unk_0x70`/`0x74`/`0x78`/`0x88`/`0x8C`/`0x90`/`0x94`/`0xA0`/`0xA4`/`0xB4`/
`0xB8`/`0xBC`/`0x908`/`0x90C`/`0x910`, and `unknwon_int_0x44`), which is
strong independent corroboration for all of those.

## Source

```c
void DreamSys__ResetLinkState(DreamSys *this, s32 arg1, s32 arg2)
{
	struct {
		s8 unknown_values_0x0[8];
		s16 field_0x8;
		s16 field_0xA;
	} local;

	this->vt->LogChunkMood(this, &this->linkCoordinates);
	this->vt->DreamSys__SelectCallback80(this, 1);
	this->vt->DreamSys__SelectCallback98(this, 1);
	this->vt->func_8005A168(this, arg1);

	this->unk_0xBC = -1;
	this->unk_0xB4 = 0;
	this->unk_0xB8 = 0;
	this->unk_0xA0 = 0;
	this->unk_0xA4 = 0;
	this->unk_0x88 = 0;
	this->unk_0x90 = 0;
	this->unk_0x8C = 0;
	this->unk_0x94 = 0;
	this->vt->func_8005A1B0(this, 0, 1, 1, 1);

	this->vt->func_8005A1EC(this, arg2);

	this->nextCinematic.entry = -1;
	this->unk_0x70 = 0;
	this->unknwon_int_0x44 = 0;
	this->unk_0x74 = 0;
	this->unk_0x908 = 0;
	this->unk_0x90C = 0;
	this->unk_0x910 = 0;
	this->unk_0x78 = 0;
	Class6B5CC__GetRotationDegrees(this, &local);

	local.field_0x8 = 0;
	local.field_0xA = 1;
	this->vt->func_8001CEB4(this, 1, &local);
}
```

## Derivation

Read directly off the disassembly with no branches to reason about — the
only care needed was PRESERVING THE EXACT STORE ORDER (this function has no
control flow to hide behind, so any reordering of the zero-stores or the
vtable calls would show up immediately as a byte diff) and correctly
identifying two things that looked like they might need new struct/local
types but turned out to already exist or need only a minimal local:

- `&this->unk_0x16C` (the first call's `a1`) is exactly `&this->
  linkCoordinates` — `currentStage` (`+0x164`, 4 bytes) + `nextCinematic`
  (`+0x168`, 4 bytes) lands `linkCoordinates` at `+0x16C` exactly, so no new
  field was needed, just the existing name.
- `this->unk_0x16A` (a `sh` store of `-1`) is `this->nextCinematic.entry`
  — the second half of the already-typed `CinematicCall` struct
  (`+0x168` = `bank`, `+0x16A` = `entry`).
- The stack buffer at `sp+0x18`, passed to both `Class6B5CC__GetRotationDegrees` (as an
  output buffer) and the closing `func_8001CEB4` (as `arg2`), only needed
  two of its bytes named (`+0x8` and `+0xA`, both `s16`, values `0` and `1`)
  — everything else in it is written by `Class6B5CC__GetRotationDegrees` itself and never
  read back by this function, so it stays `unknown_values_0x0[8]`. The 5th
  argument to `func_8005A1B0` (a literal `1`, spilled to `sp+0x10` by the
  O32 ABI) needed no explicit local at all — writing the call with five
  arguments directly let the compiler place it on the stack itself.

## Proposed learning

None beyond what's already documented — a clean first-attempt match on a
branch-free function is mostly a test of whether the CALLING unit's
existing field/vtable-slot names are right, and here every single one of
them (six vtable slots, thirteen struct fields) checked out against a
function none of them had been confirmed against before.
