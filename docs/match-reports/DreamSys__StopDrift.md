# DreamSys__StopDrift

> Renamed from `func_8005A134` on 2026-09-22 (tools/rename.py). Address 0x8005a134.

**Unit:** DreamSys · **Size:** 13 instructions · **Status:** MATCHED (13/13 words)

## What it does

Vtable slot `+0x17C`. Zeroes `unk_0xC4`, unconditionally stores `arg1` into
`unk_0xC8`, and, only if `arg1 != 0`, calls `FlushSoundCueSet(this->unk_0x58,
this->unk_0xCC)`. Called by `DreamSys__SelectMoveCallback`'s entry guard (per an earlier
round's vtable comment) as `(this, 0)` when `this->unk_0x9C == 2`.

## The C

```c
void DreamSys__StopDrift(DreamSys *this, s32 arg1)
{
	this->unk_0xC4 = 0;
	this->unk_0xC8 = arg1;
	if (arg1 != 0)
		FlushSoundCueSet(this->unk_0x58, this->unk_0xCC);
}
```

## Note: `unk_0xC8 = arg1` is unconditional in the source too, not just in the asm

Retail stores `arg1` into `unk_0xC8` in the delay slot of the `beqz $a1,
...` that guards the `FlushSoundCueSet` call -- i.e. it happens on BOTH paths.
Writing the store as a plain unconditional statement before the `if`
reproduces this directly; no need for a delay-slot-mimicking trick.

`FlushSoundCueSet` was already declared in `Entity.h` for a different struct's
fields (`extern void FlushSoundCueSet(s32 arg0, void *arg1);`); added the same
declaration locally to `DreamSys.h` rather than cross-including `Entity.h`.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

`DreamSys__StopDrift` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005A134`.

Always clears `driftActive`, which is what stops
`DreamSys__TickDrift`'s vertical step; sets `cueServiceActive` to its argument and
flushes the sound cue set when that argument is nonzero. Its only carved call site
(`DreamSys__SelectMoveCallback`'s entry guard, when the mode being left was 2) passes
0, so the flush branch is never observed running. Tier B, and the argument is the
reason: the name describes the unconditional half.
