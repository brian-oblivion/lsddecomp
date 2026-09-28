# DreamSys__ResetSessionState

> Renamed from `DreamSys__func_588ec` on 2026-09-22 (tools/rename.py). Address 0x800588ec.

**Unit:** DreamSys · **Size:** 31 instructions · **Status:** MATCHED (31/31 words)

## What it does

Vtable slot `+0x040`. Resets a batch of session/journal state: calls two
helper methods (`this->vt->SceneNode__SetDisplay(this, 0)` then
`this->vt->SceneNode__UpdateRotation(this, 1, sRotationYaw180)`), then clears eight fields to
zero: `callback_0x80`, `callback_0x98`, the first word of the still-opaque
`unk_0xCC` block, three fields in the previously-undiscovered struct tail
(`unk_0x908`, `unk_0x90C`, `unk_0x910`), `unk_0x78`, and `unk_0x924`.

## The C

```c
void DreamSys__ResetSessionState(DreamSys *this)
{
	this->vt->SceneNode__SetDisplay(this, 0);
	this->vt->SceneNode__UpdateRotation(this, 1, sRotationYaw180);
	this->callback_0x80 = NULL;
	this->callback_0x98 = NULL;
	*(s32 *)this->unk_0xCC = 0;
	this->unk_0x908 = 0;
	this->unk_0x90C = 0;
	this->unk_0x910 = 0;
	this->unk_0x78 = 0;
	this->unk_0x924 = 0;
}
```

## Key finding: `sizeof(DreamSys)` was wrong -- the struct was 0x98 bytes short

Two of this function's stores (`unk_0x908`, `unk_0x90C`, `unk_0x910`,
`unk_0x924`) land past the struct's previously-documented end at `storedDay`
(offset `0x88C`, struct size `0x890`). Cross-checked against `New_DreamSys`
(still `INCLUDE_ASM`, but its own disassembly is retail bytes regardless):
it allocates via a literal `ori $a0, $zero, 0x928` before calling the
constructor. `sizeof(DreamSys)` is `0x928`, not `0x890` -- the header was
missing the tail 0x98 bytes entirely. Extended it with
`unknown_values_0x890[0x78]`, the four now-named words, and a trailing
`unknown_values_0x914[0x10]` gap. Verified against a host `-m32` build with
`offsetof` before committing (all pre-existing named fields' offsets
reproduce exactly with `-m32`; they do NOT with the host's native 64-bit
pointers, which was a five-minute detour before realising `void *` fields
need `-m32` to model this 32-bit target correctly).

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

`DreamSys__ResetSessionState` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `DreamSys__func_588ec`.

The constructor's last step (`DreamSys__DreamSys` returns
`this->vt->DreamSys__ResetSessionState(this)`), and the only thing it does is put the
object into its idle state: both tick callback slots NULLed directly (not via
`DreamSys__SelectCallback80/98`), `*(s32 *)this->soundCueSet = 0` -- that word is
`SoundCueSet::tag`, and `InitSoundCueSet` (src/PlacementGridVabSound.c, matched) refuses to
run unless it is 0, so this frees the cue set -- the three staircase-walk words
`staircaseActive`/`staircaseMoveGate`/`staircaseTickFn` cleared, `unk_0x78` and
`unk_0x924` cleared, and `SceneNode__UpdateRotation(this, 1, &sRotationYaw180)`, an ABSOLUTE
rotation to yaw 180.
"Session" is the soft half: nothing establishes that the state it clears is
per-dream rather than per-object, so tier B, not A.
