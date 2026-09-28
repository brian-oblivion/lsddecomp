# DreamSys__TickDrift

> Renamed from `func_8005A0B0` on 2026-09-22 (tools/rename.py). Address 0x8005a0b0.

**Unit:** DreamSys · **Size:** 33 instructions · **Status:** MATCHED (33/33 words)

## What it does

Two independent guarded actions: if `unk_0xC4 != 0`, calls
`this->vt->Actor__AddTranslation(this, &sDriftStep)` (the same slot `DreamSys__ApplyRelativeOffset`
calls, this time with a static global vector instead of a computed diff)
and decrements `this->unk_0x5C->unk_0x24` by `0x258` (600); if `unk_0xC8 !=
0`, calls `ServiceSoundCueSet(this->unk_0x58, this->unk_0xCC)`.

## The C

```c
void DreamSys__TickDrift(DreamSys *this)
{
	if (this->unk_0xC4 != 0) {
		this->vt->Actor__AddTranslation(this, &sDriftStep);
		this->unk_0x5C->unk_0x24 -= 0x258;
	}
	if (this->unk_0xC8 != 0)
		ServiceSoundCueSet(this->unk_0x58, this->unk_0xCC);
}
```

`ServiceSoundCueSet` is already declared in `Entity.h` for a different struct's
fields (`extern void ServiceSoundCueSet(s32 arg0, void *arg1);`); added the same
declaration locally rather than cross-including `Entity.h`, same as
`FlushSoundCueSet` in an earlier round. Matched first try -- the `DreamSysVec3`
type and `Actor__AddTranslation` slot were already established by `DreamSys__ApplyRelativeOffset`
earlier in this round.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).

## Naming

`DreamSys__TickDrift` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005A0B0`.

The function `DreamSys__SelectCallback98(this, 2)` installs in
`callback_0x98`. Each tick, while `driftActive`, it adds the constant vector
`sDriftStep` -- (0, 512, 0), i.e. purely vertical -- to the object's position and
lowers `heightCurve->endValue` by 600; while `cueServiceActive`, it services the
sound cue set (`ServiceSoundCueSet(soundObj, soundCueSet)`).
Deliberately NOT called `TickFall` or `TickRise`: this unit never establishes which
way +Y points, so the name says "drift" and the comment says "+512 on the Y
axis".

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* libsnd_vmanager's SoundCueSet service pair, as this unit calls them
   (DreamSys__TickDrift / DreamSys__StopDrift: (soundObj, soundCueSet)).
   Moved here from DreamSys.h in track 4 (round 88): Entity.h declares the
   same functions with `void *` parameters, and a unit including both
   headers would see conflicting types. */
```
