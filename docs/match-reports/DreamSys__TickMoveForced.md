# DreamSys__TickMoveForced — MATCHED

> Renamed from `func_80059B50` on 2026-09-22 (tools/rename.py). Address 0x80059b50.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 33/33 words, full match on
the first attempt.

## Source

```c
s32 DreamSys__TickMoveForced(DreamSys *this)
{
	this->unk_0xA0 = 1;
	if (this->unk_0x70 != 0)
		return this->vt->DreamSys__AdvanceMoveCycle(this, 0);
	return this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1));
}
```

Vtable slot `0x15C`. Forces `unk_0xA0` to `1` unconditionally, then either
returns `DreamSys__AdvanceMoveCycle(this, 0)`'s result directly, or chains
`DreamSys__AdvanceMoveCycle(this, 1)` into `DreamSys__ApplyMoveCommand(this, ...)` and returns that.
Both callees are still `INCLUDE_ASM` (slot `0x164`, and outside this
runner's range respectively).

## Derivation

Applied `DreamSys__TickMoveFree`'s freshly-learned lesson directly: despite this
ALSO being an early-exit-with-different-value shape (the `unk_0x70 != 0`
case returns a different call chain than the fallthrough case), wrote the
PLAIN early-return form with no `goto` and the two virtual calls nested
directly into the final `return` expression, matching `DreamSys__TickMoveFree`'s
working shape exactly. Matched immediately — no residue, no reshaping
needed this time.

## Proposed learning

Reinforces `DreamSys__TickMoveFree`'s finding: for this pair of sibling functions
(`DreamSys__TickMoveFree`/`DreamSys__TickMoveForced`, both dispatched off `DreamSys__TickMove` and
sharing the same `DreamSys__AdvanceMoveCycle`→`DreamSys__ApplyMoveCommand` call chain), the plain,
unadorned early-return form is correct, and the `goto`-based lever from
broadcast #1 would have been the wrong reflex to reach for here too.

## Naming

`DreamSys__TickMoveForced` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059B50`.

The `moveOverride == 1` arm. Forces `moveCommand` to 1 before
anything else, then -- unlike `DreamSys__TickMoveFree` -- still advances the cycle
when `movementBlocked` is set, just with `DreamSys__AdvanceMoveCycle`'s `arg1` 0,
which suppresses the +-50 view bob and skips
`DreamSys__ApplyMoveCommand` entirely. "Forced" is the contrast with Free: the move
command is imposed rather than read.
