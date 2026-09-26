# DreamSys__FlashbackSaving

**Unit:** DreamSys · **Size:** 56 words (0xE0 bytes) · **Status:** MATCHED
(56/56 words, whole-image `./build-and-verify.sh` green)

## What it does

`gDreamSysMethods` slot `+0x218` (confirmed via `tools/classtable.py
gDreamSysMethods`). Roughly 1-in-3 (`rand() % 3 == 0`) of the time, if
`this->unk_0x4C` exists, records the player's current position as a new
flashback entry:

```c
void DreamSys__FlashbackSaving(DreamSys *this, s32 arg1, s32 arg2)
{
	PlayerSpawnPoint *pos;
	s32 local[4];

	if (this->unk_0x4C != NULL && rand() % 3 == 0) {
		pos = this->unk_0x4C->methods->slot0x10C(this->unk_0x4C, 0, 0);
		SceneNode__GetRotationDegrees(this, local);
		this->vt->AddFlashback(this, this->currentStage, pos, local, arg1, arg2, this->currentDay);
	}
}
```

Matched first attempt — no reshaping needed.

## New knowledge

- **`DreamSysUnk4CMethods` (`this->unk_0x4C`'s vtable) has a slot at `+0x10C`**,
  same offset and signature as `DreamSysEntityMethods::slot0x10C` (the
  unrelated `entity` parameter type used in `DreamSys__ProcessChunkChange`):
  `PlayerSpawnPoint *(*slot0x10C)(void *self, s32 arg1, s32 arg2)`. Called here
  as `(this->unk_0x4C, 0, 0)`. Plausibly the same underlying class as the
  `entity` objects elsewhere in this unit, but kept as a separate local view
  per this unit's established "multiple independent local views of the same
  method table" convention (`DECOMPILATION_LEARNINGS.md`) rather than unifying
  the two headers.
- **`gDreamSysMethods+0x214` retyped from an untyped `void *
  GameManager__AddFlashback` placeholder to `AddFlashback`**, with
  `DreamSys__AddFlashback`'s own signature. Grepped `src/` and `include/` for
  the old field name first — no other caller referenced it, so this is a
  plain correction, not an out-of-scope edit.
- **`gDreamSysMethods+0x218` retyped similarly to `FlashbackSaving`** (this
  function's own slot, per `classtable.py`). Not called through the vtable by
  anything in this unit; retyped for documentation only.
- **The call passes 7 arguments: 4 in `$a0`-`$a3`, 3 more on the caller's
  stack past the reserved `$a0`-`$a3` shadow space** (`sp+0x10`, `sp+0x14`,
  `sp+0x18`) — an ordinary o32 ABI overflow-argument case once you recognise
  it; nothing exotic, just more parameters than fit in registers.
- Confirms the already-documented "delay slot after a `jalr` captures the
  PRECEDING call's return value" idiom again: the `jal SceneNode__GetRotationDegrees`
  instruction's delay slot (`addu $s0, $v0, $zero`) captures `pos`, the return
  of the *preceding* `slot0x10C` call, not an argument to `SceneNode__GetRotationDegrees`.
- The `rand() % 3 == 0` reproduces retail's `mult`/`mfhi`/`sra`-`subu`
  sign-correction magic-multiply exactly, per the already-confirmed "`x % N`
  for compile-time-constant `N`: just write `%`" idiom — another instance,
  no new lever needed.

### Proposed learning

None beyond the confirmations already logged in `DECOMPILATION_LEARNINGS.md`
(delay-slot-after-jalr, `x % N`, multiple-local-views-of-one-vtable). Filing
here rather than duplicating them there.
