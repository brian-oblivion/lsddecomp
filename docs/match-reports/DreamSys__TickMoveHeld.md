# DreamSys__TickMoveHeld — MATCHED

> Renamed from `func_80059BD4` on 2026-09-22 (tools/rename.py). Address 0x80059bd4.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 3/3 words, full match on
the first attempt.

## Source

```c
s32 DreamSys__TickMoveHeld(DreamSys *this)
{
	return this->unk_0xA0 = 1;
}
```

Vtable slot `0x160`. The smallest function in this runner's range. Per
CLAUDE.md's "one-line wrapper" note, the byte pattern alone (`ori $v0,$zero,1
/ jr $ra / sw $v0,0xA0($a0)`) doesn't distinguish `void` from a return —
here the POSITIVE evidence for `s32` is that `$v0` is loaded with `1` BEFORE
the store and is left completely untouched through to `jr $ra`, i.e. the
retail code is deliberately keeping the just-computed value alive as the
return register rather than merely using it as a store source. Also called
by `DreamSys__TickMove` (matched earlier this round) as the direct source of its
own return value in the `unk_0x6c == 2` case, which independently confirms
the `s32` return type is actually used by a caller, not just theoretically
available.

## Proposed learning

None — clean first-attempt match, no residue. Filed mainly to record the
positive-evidence check for the return type, per CLAUDE.md's warning about
one-line wrappers.

## Naming

`DreamSys__TickMoveHeld` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059BD4`.

The `moveOverride == 2` arm, and the whole body is
`return this->moveCommand = 1;`. It imposes the same command
`DreamSys__TickMoveForced` does but runs no cycle and no move, so the object holds
the command without acting on it. Tier B: the mechanics are a one-liner, the
name's claim is the relationship to its two siblings.
