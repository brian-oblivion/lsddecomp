# DreamSys__TickMove — MATCHED

> Renamed from `func_80059A58` on 2026-09-22 (tools/rename.py). Address 0x80059a58.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 37/37 words, full match.

## Source

```c
s32 DreamSys__TickMove(DreamSys *this)
{
	if (this->unk_0x6c == 0) {
		this->vt->DreamSys__ApplyPendingTurn(this);
		return this->vt->DreamSys__TickMoveFree(this);
	} else if (this->unk_0x6c != 2) {
		return this->vt->DreamSys__TickMoveForced(this);
	} else {
		return this->vt->DreamSys__TickMoveHeld(this);
	}
}
```

Vtable slot `0x154`. A dispatcher on `unk_0x6c`: `0` calls `DreamSys__ApplyPendingTurn`
(still `INCLUDE_ASM`, result discarded) then tail-returns `DreamSys__TickMoveFree`'s
result; `2` tail-returns `DreamSys__TickMoveHeld`'s result; anything else tail-returns
`DreamSys__TickMoveForced`'s result. Return type `s32` per CLAUDE.md's "one-line
wrapper" note — this is a passthrough of whatever the selected callee
returns, and two of the three callees (`DreamSys__TickMoveFree`, `DreamSys__TickMoveForced`,
matched later this round) are confirmed `s32`-returning, so a `void` wrapper
would be actively wrong here, not just unproven.

## Residue fixed

One attempt short of a match (34/37): wrote the `unk_0x6c == 2` case as
`if (unk_0x6c == 2) { BD4 } else { B50 }`, matching how I read the
comparison off the disassembly. Retail's actual branch is `bne
v1,v0,ELSE_TARGET` (branch away on NOT-equal) with the `!=2` case's body
placed at the far target and the `==2` case inline — the opposite pairing
from a naive `if (==2) {...} else {...}` reading. Swapping to `if (unk_0x6c
!= 2) { B50 } else { BD4 }` (matching retail's actual `bne`/inline pairing)
fixed it immediately. Same family of residue as `DreamSys__FlipMoveCommand` this round:
reading a comparison's DIRECTION off the disassembly and transcribing the
"obvious" `if (==) {...} else {...}` shape doesn't reliably reproduce which
block ends up inline vs at the branch target.

## Answering HEAD BROADCAST #1/#2/#3

Neither the goto-vs-return lever (broadcast #1) nor the loop-invariant lever
(broadcast #2) applies — no early exit differs by value (this function's
three paths are symmetric tail-returns, not an early-exit-vs-main-path
shape) and there's no loop. Broadcast #3's branch-target check is exactly
what caught this residue: comparing `bne` vs `beq` and which target each
pointed at, not assuming the comparison read one way implies the source was
written that way.

## Naming

`DreamSys__TickMove` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059A58`.

The function `DreamSys__SelectCallback98(this, 1)` installs in
`callback_0x98`, so it runs every tick in mode 1. Its whole body is a dispatch on
`moveOverride`: 0 runs `DreamSys__ApplyPendingTurn` then `DreamSys__TickMoveFree`,
2 runs `DreamSys__TickMoveHeld`, anything else runs `DreamSys__TickMoveForced`, and
it returns whichever result. Tier B.
