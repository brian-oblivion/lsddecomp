# DreamSys__ApplyPendingTurn

> Renamed from `func_8005A050` on 2026-09-22 (tools/rename.py). Address 0x8005a050.

**Unit:** DreamSys · **Size:** 24 instructions · **Status:** MATCHED (24/24 words)

## What it does

Vtable slot `+0x174`. If `this->unk_0xA4` (a new index field) is nonzero,
calls `this->vt->SceneNode__UpdateRotation(this, 0, &sTurnRotations[idx])` and resets
`unk_0xA4` to 0. `sTurnRotations` is address-of only here -- never loaded
through -- so this is safe against the `addiu_at` runtime-indexed-load
blocker (`docs/research/addiu-at-blocker.md`): "Address-only table
arithmetic is safe", per `DECOMPILATION_LEARNINGS.md`.

## The C

```c
void DreamSys__ApplyPendingTurn(DreamSys *this)
{
	s32 idx;

	idx = this->unk_0xA4;
	if (idx != 0) {
		this->vt->SceneNode__UpdateRotation(this, 0, &sTurnRotations[idx]);
		this->unk_0xA4 = 0;
	}
}
```

```c
typedef struct D_80087E80Entry {
	s32 unk0;
	s32 unk4;
	s32 unk8;
} D_80087E80Entry;
extern D_80087E80Entry sTurnRotations[];
```

## Note: `sTurnRotations` is splat's auto-generated name for TWO seemingly
different things

An earlier round's comment on `TURN_ROTATION_YAW` says `&TURN_ROTATION_YAW[-1] (==
&sTurnRotations, a distinct label immediately before it)`, describing a
4-byte-stride array (`DreamSys__StepLookYaw`, `D_80087E84Entry` = `{s16, s16}`).
This function's own `lui`/`addiu %hi/%lo(sTurnRotations)` uses a 12-byte stride
(`idx*12`, from `sll,1` + `addu` + `sll,2`) -- incompatible with a 4-byte
element. Both call sites reference the SAME linker symbol name (splat picked
the same label because both point at the same byte address), but the
strides don't reconcile into one struct. Most likely explanation: two
unrelated globals that happen to sit at adjacent addresses, and the earlier
round's `[-1]` observation was really about `DreamSys__StepLookYaw` using a constant
(not runtime-indexed) address that happens to land exactly on this second,
unrelated symbol. Left both declarations in place with a comment
cross-referencing this; not resolved further this round.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

`DreamSys__ApplyPendingTurn` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005A050`.

Consumes `turnCommand`: `SceneNode__UpdateRotation(this, 0,
&sTurnRotations[idx])` and reset to 0. That slot is the rotation setter (matched,
src/SceneNode.c) and flag 0 means RELATIVE, so this turns the object; the table
entries for the only two values `DreamSys__OnPadEvent` ever writes (1 and 2)
decode to yaw -6 and +6 degrees. Tier B: a 6-degree step per tick is a gradual
turn, which is the mechanics; nothing establishes what raises the command.
